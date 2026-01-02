// tc05_group_parser.cpp
// Build: g++ -std=c++17 -O2 -Wall -Wextra tc05_group_parser.cpp -o tc05_group_parser
//
// Usage:
//   ./tc05_group_parser <input_file> [ctf|itf]
//
// What it does:
// - Reads BASE II records (CTF 168 bytes, or ITF 170 bytes with 2-byte hash at positions 3-4).
// - Normalizes ITF->CTF positions so parsing uses the same 1-based field positions.
// - Parses TC=05, TCR 0..7.
// - Groups records: each TCR0 starts a new group. Group key is ARN (from TCR0).
// - Outputs ONE JSON document to stdout:
//     {
//       "groups_by_arn": {
//         "<ARN>": [
//           {
//             "group_id": 1,
//             "arn": "...",
//             "tcrs": { "0":[...], "1":[...], ... "7":[...] },
//             "dates": { ... },
//             "inconsistencies": [ ... ],
//             "stats": { ... }
//           },
//           ...
//         ],
//         ...
//       },
//       "global_inconsistencies": [ ... ]
//     }
//
// Inconsistency detection (examples):
// - Record length not 168/170
// - TC != 05 inside an active group record stream (ignored, logged)
// - TCR outside 0..7
// - Records before the first TCR0 (logged)
// - Out-of-order TCR sequence inside a group (e.g., 7 then 2), repeated unexpected jumps
// - Missing ARN in TCR0
// - Non-digit date fields where digits expected, wrong lengths (basic validation)
// - Non-digit numeric fields where digits expected (basic validation)

#include <algorithm>
#include <cctype>
#include <fstream>
#include <iostream>
#include <map>
#include <optional>
#include <set>
#include <sstream>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

enum class FileFormat { CTF, ITF };

// ---------- small helpers ----------
static inline std::string rtrim_copy(std::string s) {
  while (!s.empty() && (s.back() == ' ' || s.back() == '\0' || s.back() == '\r' || s.back() == '\n' || s.back() == '\t'))
    s.pop_back();
  return s;
}
static inline std::string trim_copy(std::string s) {
  size_t b = 0;
  while (b < s.size() && std::isspace(static_cast<unsigned char>(s[b]))) b++;
  size_t e = s.size();
  while (e > b && std::isspace(static_cast<unsigned char>(s[e - 1]))) e--;
  return s.substr(b, e - b);
}
static inline bool is_digits(const std::string& s) {
  if (s.empty()) return false;
  for (unsigned char c : s) if (!std::isdigit(c)) return false;
  return true;
}
// 1-based slice
static inline std::string field(const std::string& rec168, int pos1, int len) {
  if (pos1 <= 0 || len <= 0) return {};
  size_t start = static_cast<size_t>(pos1 - 1);
  if (start >= rec168.size()) return {};
  size_t n = std::min(static_cast<size_t>(len), rec168.size() - start);
  return rec168.substr(start, n);
}

static inline std::string json_escape(const std::string& s) {
  std::ostringstream oss;
  for (unsigned char c : s) {
    switch (c) {
    case '\\': oss << "\\\\"; break;
    case '"': oss << "\\\""; break;
    case '\b': oss << "\\b"; break;
    case '\f': oss << "\\f"; break;
    case '\n': oss << "\\n"; break;
    case '\r': oss << "\\r"; break;
    case '\t': oss << "\\t"; break;
    default:
      if (c < 0x20) {
        oss << "\\u" << std::hex;
        oss.width(4);
        oss.fill('0');
        oss << (int)c << std::dec;
      }
      else {
        oss << static_cast<char>(c);
      }
    }
  }
  return oss.str();
}
static inline std::string J(const std::string& s) { return "\"" + json_escape(s) + "\""; }

static std::optional<std::string> normalize_record(const std::string& rec, FileFormat fmt) {
  if (fmt == FileFormat::CTF) {
    if (rec.size() != 168) return std::nullopt;
    return rec;
  }
  // ITF 170 -> remove positions 3-4 (1-based) => bytes [2..3] (0-based)
  if (rec.size() != 170) return std::nullopt;
  std::string out;
  out.reserve(168);
  out.append(rec, 0, 2);     // pos 1-2
  out.append(rec, 4, 166);   // pos 5..170
  if (out.size() != 168) return std::nullopt;
  return out;
}

// Reads either fixed-block or line-delimited.
static std::vector<std::string> read_records(const std::string& path, size_t recLen) {
  std::ifstream in(path, std::ios::binary);
  if (!in) throw std::runtime_error("Cannot open input file: " + path);

  std::string data((std::istreambuf_iterator<char>(in)), std::istreambuf_iterator<char>());

  // line-delimited heuristic
  if (data.find('\n') != std::string::npos) {
    std::vector<std::string> recs;
    std::istringstream iss(data);
    std::string line;
    while (std::getline(iss, line)) {
      if (!line.empty() && line.back() == '\r') line.pop_back();
      if (line.empty()) continue;
      if (line.size() < recLen) line.append(recLen - line.size(), ' ');
      if (line.size() >= recLen) recs.push_back(line.substr(0, recLen));
    }
    return recs;
  }

  // fixed-block
  if (data.size() % recLen != 0) {
    std::ostringstream oss;
    oss << "File size (" << data.size() << ") is not multiple of record length (" << recLen << ")";
    throw std::runtime_error(oss.str());
  }

  std::vector<std::string> recs;
  recs.reserve(data.size() / recLen);
  for (size_t off = 0; off < data.size(); off += recLen) {
    recs.push_back(data.substr(off, recLen));
  }
  return recs;
}

// ---------- Parsed structures (key fields + raw record) ----------

struct ParsedRecord {
  char tcr = '?';
  std::map<std::string, std::string> fields;  // key->value (trimmed)
  std::string raw_168;                        // normalized
  long long record_index = 0;                 // position in file (1-based)
};

static ParsedRecord parse_tcr0(const std::string& r, long long idx) {
  // TC05/TCR0 key fields (positions are in normalized 168)
  ParsedRecord pr;
  pr.tcr = '0';
  pr.record_index = idx;
  pr.raw_168 = r;

  pr.fields["tc"] = trim_copy(field(r, 1, 2));
  pr.fields["tcq"] = trim_copy(field(r, 3, 1));
  pr.fields["tcr"] = trim_copy(field(r, 4, 1));

  pr.fields["account_number"] = trim_copy(field(r, 5, 16));
  pr.fields["account_number_extension"] = trim_copy(field(r, 21, 3));
  pr.fields["arn"] = trim_copy(field(r, 27, 23));
  pr.fields["acquirer_business_id"] = trim_copy(field(r, 50, 8));
  pr.fields["purchase_date_mmdd"] = trim_copy(field(r, 58, 4));
  pr.fields["central_processing_date_yddd"] = trim_copy(field(r, 164, 4));

  pr.fields["destination_amount"] = trim_copy(field(r, 62, 12));
  pr.fields["destination_currency"] = trim_copy(field(r, 74, 3));
  pr.fields["source_amount"] = trim_copy(field(r, 77, 12));
  pr.fields["source_currency"] = trim_copy(field(r, 89, 3));

  pr.fields["merchant_name"] = rtrim_copy(field(r, 92, 25));
  pr.fields["merchant_city"] = rtrim_copy(field(r, 117, 13));
  pr.fields["merchant_country_code"] = trim_copy(field(r, 130, 3));
  pr.fields["mcc"] = trim_copy(field(r, 133, 4));

  return pr;
}

static ParsedRecord parse_tcr1(const std::string& r, long long idx) {
  ParsedRecord pr;
  pr.tcr = '1';
  pr.record_index = idx;
  pr.raw_168 = r;

  pr.fields["tc"] = trim_copy(field(r, 1, 2));
  pr.fields["tcq"] = trim_copy(field(r, 3, 1));
  pr.fields["tcr"] = trim_copy(field(r, 4, 1));

  // A few common fields + conversion date (often used)
  pr.fields["business_format_code"] = trim_copy(field(r, 5, 1));
  pr.fields["member_message_text"] = rtrim_copy(field(r, 24, 50));
  pr.fields["card_acceptor_id"] = trim_copy(field(r, 81, 15));
  pr.fields["terminal_id"] = trim_copy(field(r, 96, 8));
  pr.fields["conversion_date"] = trim_copy(field(r, 118, 4)); // yymm or yymm? kept as-is per file
  pr.fields["cashback"] = trim_copy(field(r, 158, 9));
  return pr;
}

static ParsedRecord parse_tcr2_generic(const std::string& r, long long idx) {
  ParsedRecord pr;
  pr.tcr = '2';
  pr.record_index = idx;
  pr.raw_168 = r;

  pr.fields["tc"] = trim_copy(field(r, 1, 2));
  pr.fields["tcq"] = trim_copy(field(r, 3, 1));
  pr.fields["tcr"] = trim_copy(field(r, 4, 1));

  // TCR2 is country/region-specific; capture the discriminator and keep raw.
  pr.fields["business_format_code"] = trim_copy(field(r, 5, 2));
  pr.fields["raw"] = r; // full record for downstream decoding
  return pr;
}

static ParsedRecord parse_tcr3_min(const std::string& r, long long idx) {
  ParsedRecord pr;
  pr.tcr = '3';
  pr.record_index = idx;
  pr.raw_168 = r;

  pr.fields["tc"] = trim_copy(field(r, 1, 2));
  pr.fields["tcq"] = trim_copy(field(r, 3, 1));
  pr.fields["tcr"] = trim_copy(field(r, 4, 1));
  pr.fields["business_format_code"] = trim_copy(field(r, 5, 2));
  pr.fields["industry_subformat"] = trim_copy(field(r, 7, 2));
  pr.fields["raw_payload"] = rtrim_copy(field(r, 9, 160));
  return pr;
}

static ParsedRecord parse_tcr4(const std::string& r, long long idx) {
  ParsedRecord pr;
  pr.tcr = '4';
  pr.record_index = idx;
  pr.raw_168 = r;

  pr.fields["tc"] = trim_copy(field(r, 1, 2));
  pr.fields["tcq"] = trim_copy(field(r, 3, 1));
  pr.fields["tcr"] = trim_copy(field(r, 4, 1));

  pr.fields["business_format_code"] = trim_copy(field(r, 15, 2));
  pr.fields["message_reason_code"] = trim_copy(field(r, 47, 4));
  pr.fields["surcharge_amount"] = trim_copy(field(r, 51, 8));
  pr.fields["payment_account_reference"] = trim_copy(field(r, 112, 29));
  pr.fields["token_requestor_id"] = trim_copy(field(r, 141, 11));
  pr.fields["total_discount_amount"] = trim_copy(field(r, 160, 8));
  return pr;
}

static ParsedRecord parse_tcr5(const std::string& r, long long idx) {
  ParsedRecord pr;
  pr.tcr = '5';
  pr.record_index = idx;
  pr.raw_168 = r;

  pr.fields["tc"] = trim_copy(field(r, 1, 2));
  pr.fields["tcq"] = trim_copy(field(r, 3, 1));
  pr.fields["tcr"] = trim_copy(field(r, 4, 1));

  pr.fields["transaction_identifier"] = trim_copy(field(r, 5, 15));
  pr.fields["authorized_amount"] = trim_copy(field(r, 20, 12));
  pr.fields["authorization_currency_code"] = trim_copy(field(r, 32, 3));
  pr.fields["authorization_response_code"] = trim_copy(field(r, 35, 2));
  pr.fields["pan_token"] = trim_copy(field(r, 150, 16));
  pr.fields["cvv2_result_code"] = trim_copy(field(r, 168, 1));
  return pr;
}

static ParsedRecord parse_tcr6(const std::string& r, long long idx) {
  ParsedRecord pr;
  pr.tcr = '6';
  pr.record_index = idx;
  pr.raw_168 = r;

  pr.fields["tc"] = trim_copy(field(r, 1, 2));
  pr.fields["tcq"] = trim_copy(field(r, 3, 1));
  pr.fields["tcr"] = trim_copy(field(r, 4, 1));

  pr.fields["business_format_code"] = trim_copy(field(r, 5, 2));
  pr.fields["limited_use_account_number"] = trim_copy(field(r, 7, 19));
  pr.fields["limited_use_effective_date"] = trim_copy(field(r, 29, 6));
  pr.fields["limited_use_expiration_date"] = trim_copy(field(r, 35, 6));
  return pr;
}

static ParsedRecord parse_tcr7(const std::string& r, long long idx) {
  ParsedRecord pr;
  pr.tcr = '7';
  pr.record_index = idx;
  pr.raw_168 = r;

  pr.fields["tc"] = trim_copy(field(r, 1, 2));
  pr.fields["tcq"] = trim_copy(field(r, 3, 1));
  pr.fields["tcr"] = trim_copy(field(r, 4, 1));

  pr.fields["transaction_type"] = trim_copy(field(r, 5, 2));
  pr.fields["card_sequence_number"] = trim_copy(field(r, 7, 3));
  pr.fields["terminal_transaction_date"] = trim_copy(field(r, 10, 6)); // YYMMDD
  pr.fields["terminal_country_code"] = trim_copy(field(r, 22, 3));
  pr.fields["cryptogram_amount"] = trim_copy(field(r, 87, 12));
  pr.fields["issuer_script1_results"] = trim_copy(field(r, 159, 10));
  return pr;
}

static ParsedRecord parse_tc05_tcr0_7(const std::string& r, char tcr, long long idx) {
  switch (tcr) {
  case '0': return parse_tcr0(r, idx);
  case '1': return parse_tcr1(r, idx);
  case '2': return parse_tcr2_generic(r, idx);
  case '3': return parse_tcr3_min(r, idx);
  case '4': return parse_tcr4(r, idx);
  case '5': return parse_tcr5(r, idx);
  case '6': return parse_tcr6(r, idx);
  case '7': return parse_tcr7(r, idx);
  default: {
    ParsedRecord pr;
    pr.tcr = tcr;
    pr.record_index = idx;
    pr.raw_168 = r;
    pr.fields["tc"] = trim_copy(field(r, 1, 2));
    pr.fields["tcq"] = trim_copy(field(r, 3, 1));
    pr.fields["tcr"] = trim_copy(field(r, 4, 1));
    pr.fields["raw"] = r;
    return pr;
  }
  }
}

// ---------- Group model ----------
struct Group {
  long long group_id = 0;
  std::string arn;
  std::string group_tag;

  // tcr -> list of parsed records (some flows may repeat/duplicate TCRs)
  std::map<char, std::vector<ParsedRecord>> tcrs;

  // dates sub-json
  std::set<std::string> purchase_dates_mmdd;
  std::set<std::string> central_proc_dates_yddd;
  std::set<std::string> terminal_txn_dates_yymmdd;
  std::set<std::string> conversion_dates;        // from TCR1 (kept as-is)
  std::set<std::string> limited_use_effective;   // from TCR6
  std::set<std::string> limited_use_expiration;  // from TCR6

  std::vector<std::string> inconsistencies;

  // sequence tracking
  bool started = false;
  int last_tcr_seen = -1;
  long long first_record_index = 0;
  long long last_record_index = 0;
};

static std::string mask_pan_like(const std::string& v) {
  std::string s = trim_copy(v);
  if (s.size() <= 10) return s;
  // keep first 6 and last 4
  std::string out = s;
  for (size_t i = 6; i + 4 < out.size(); ++i) out[i] = '*';
  return out;
}

static std::string make_group_tag_from_tcr0(const ParsedRecord& tcr0) {
  std::string arn = "";
  auto it = tcr0.fields.find("arn");
  if (it != tcr0.fields.end()) arn = it->second;

  std::string acq = tcr0.fields.count("acquirer_business_id") ? tcr0.fields.at("acquirer_business_id") : "";
  std::string pd = tcr0.fields.count("purchase_date_mmdd") ? tcr0.fields.at("purchase_date_mmdd") : "";
  std::string pan = tcr0.fields.count("account_number") ? mask_pan_like(tcr0.fields.at("account_number")) : "";

  std::ostringstream oss;
  oss << "TC05|ARN=" << arn << "|ACQ=" << acq << "|PAN=" << pan << "|PD=" << pd;
  return oss.str();
}

static void add_inconsistency(Group& g, const std::string& msg) {
  g.inconsistencies.push_back(msg);
}

static void collect_dates(Group& g, const ParsedRecord& pr) {
  auto add = [](std::set<std::string>& s, const std::string& v) {
    std::string t = trim_copy(v);
    if (!t.empty()) s.insert(t);
    };

  if (pr.tcr == '0') {
    if (pr.fields.count("purchase_date_mmdd")) add(g.purchase_dates_mmdd, pr.fields.at("purchase_date_mmdd"));
    if (pr.fields.count("central_processing_date_yddd")) add(g.central_proc_dates_yddd, pr.fields.at("central_processing_date_yddd"));
  }
  else if (pr.tcr == '7') {
    if (pr.fields.count("terminal_transaction_date")) add(g.terminal_txn_dates_yymmdd, pr.fields.at("terminal_transaction_date"));
  }
  else if (pr.tcr == '1') {
    if (pr.fields.count("conversion_date")) add(g.conversion_dates, pr.fields.at("conversion_date"));
  }
  else if (pr.tcr == '6') {
    if (pr.fields.count("limited_use_effective_date")) add(g.limited_use_effective, pr.fields.at("limited_use_effective_date"));
    if (pr.fields.count("limited_use_expiration_date")) add(g.limited_use_expiration, pr.fields.at("limited_use_expiration_date"));
  }
}

static void validate_record_basic(Group& g, const ParsedRecord& pr) {
  // Basic format checks (digits + lengths).
  auto check_digits_len = [&](const std::string& key, size_t len, const std::string& label) {
    auto it = pr.fields.find(key);
    if (it == pr.fields.end()) return;
    std::string v = trim_copy(it->second);
    if (v.empty()) return;
    if (v.size() != len || !is_digits(v)) {
      std::ostringstream oss;
      oss << "Invalid " << label << " in TCR" << pr.tcr << " (record_index=" << pr.record_index
        << "): expected " << len << " digits, got '" << v << "'";
      add_inconsistency(g, oss.str());
    }
    };

  auto check_digits_any = [&](const std::string& key, const std::string& label) {
    auto it = pr.fields.find(key);
    if (it == pr.fields.end()) return;
    std::string v = trim_copy(it->second);
    if (v.empty()) return;
    if (!is_digits(v)) {
      std::ostringstream oss;
      oss << "Invalid " << label << " in TCR" << pr.tcr << " (record_index=" << pr.record_index
        << "): expected digits, got '" << v << "'";
      add_inconsistency(g, oss.str());
    }
    };

  if (pr.tcr == '0') {
    check_digits_len("purchase_date_mmdd", 4, "purchase_date_mmdd");
    check_digits_len("central_processing_date_yddd", 4, "central_processing_date_yddd");
    check_digits_any("destination_amount", "destination_amount");
    check_digits_any("source_amount", "source_amount");

    // ARN should be present and not blank
    auto it = pr.fields.find("arn");
    if (it == pr.fields.end() || trim_copy(it->second).empty()) {
      add_inconsistency(g, "Missing/blank ARN in TCR0 (group key cannot be reliable).");
    }
  }

  if (pr.tcr == '7') {
    check_digits_len("terminal_transaction_date", 6, "terminal_transaction_date_yymmdd");
    check_digits_any("cryptogram_amount", "cryptogram_amount");
  }

  if (pr.tcr == '6') {
    check_digits_len("limited_use_effective_date", 6, "limited_use_effective_date_yymmdd");
    check_digits_len("limited_use_expiration_date", 6, "limited_use_expiration_date_yymmdd");
  }
}

static void validate_sequence(Group& g, int tcr_num, long long record_index) {
  // Expect generally non-decreasing as a sanity check within a group.
  // BASE II can omit some TCRs; repeats can happen; we only flag hard decreases (except a new TCR0 starts a new group).
  if (!g.started) {
    g.started = true;
    g.last_tcr_seen = tcr_num;
    g.first_record_index = record_index;
    g.last_record_index = record_index;
    return;
  }

  if (tcr_num < g.last_tcr_seen) {
    std::ostringstream oss;
    oss << "TCR out-of-order inside group (record_index=" << record_index
      << "): saw TCR" << tcr_num << " after TCR" << g.last_tcr_seen;
    add_inconsistency(g, oss.str());
  }
  g.last_tcr_seen = std::max(g.last_tcr_seen, tcr_num);
  g.last_record_index = record_index;
}

// ---------- JSON output ----------
static void dump_record_json(std::ostream& os, const ParsedRecord& pr) {
  os << "{"
    << "\"record_index\":" << pr.record_index << ","
    << "\"tcr\":" << J(std::string(1, pr.tcr)) << ","
    << "\"fields\":{";
  bool first = true;
  for (const auto& kv : pr.fields) {
    if (!first) os << ",";
    first = false;
    os << J(kv.first) << ":" << J(rtrim_copy(kv.second));
  }
  os << "}"
    << "}";
}

static void dump_set_json(std::ostream& os, const std::set<std::string>& s) {
  os << "[";
  bool first = true;
  for (const auto& v : s) {
    if (!first) os << ",";
    first = false;
    os << J(v);
  }
  os << "]";
}

static void dump_group_json(std::ostream& os, const Group& g) {
  os << "{"
    << "\"group_id\":" << g.group_id << ","
    << "\"arn\":" << J(g.arn) << ","
    << "\"group_tag\":" << J(g.group_tag) << ","
    << "\"tcrs\":{";

  bool firstT = true;
  for (const auto& kv : g.tcrs) {
    if (!firstT) os << ",";
    firstT = false;
    os << J(std::string(1, kv.first)) << ":[";
    for (size_t i = 0; i < kv.second.size(); ++i) {
      if (i) os << ",";
      dump_record_json(os, kv.second[i]);
    }
    os << "]";
  }
  os << "},";

  os << "\"dates\":{"
    << "\"purchase_date_mmdd\":"; dump_set_json(os, g.purchase_dates_mmdd); os << ","
    << "\"central_processing_date_yddd\":"; dump_set_json(os, g.central_proc_dates_yddd); os << ","
    << "\"terminal_transaction_date_yymmdd\":"; dump_set_json(os, g.terminal_txn_dates_yymmdd); os << ","
    << "\"conversion_date\":"; dump_set_json(os, g.conversion_dates); os << ","
    << "\"limited_use_effective_date\":"; dump_set_json(os, g.limited_use_effective); os << ","
    << "\"limited_use_expiration_date\":"; dump_set_json(os, g.limited_use_expiration);
  os << "},";

  os << "\"inconsistencies\":[";
  for (size_t i = 0; i < g.inconsistencies.size(); ++i) {
    if (i) os << ",";
    os << J(g.inconsistencies[i]);
  }
  os << "],";

  os << "\"stats\":{"
    << "\"first_record_index\":" << g.first_record_index << ","
    << "\"last_record_index\":" << g.last_record_index << ","
    << "\"distinct_tcrs_present\":" << g.tcrs.size()
    << "}"
    << "}";
}

int main(int argc, char** argv) {
  try {
    if (argc < 2) {
      std::cerr << "Usage: " << argv[0] << " <input_file> [ctf|itf]\n";
      return 2;
    }

    std::string input = argv[1];
    FileFormat fmt = FileFormat::CTF;
    if (argc >= 3) {
      std::string f = argv[2];
      std::transform(f.begin(), f.end(), f.begin(), [](unsigned char c) { return std::tolower(c); });
      if (f == "itf") fmt = FileFormat::ITF;
      else if (f == "ctf") fmt = FileFormat::CTF;
      else throw std::runtime_error("Unknown format: " + f + " (expected ctf|itf)");
    }

    const size_t rawLen = (fmt == FileFormat::CTF) ? 168 : 170;
    auto rawRecords = read_records(input, rawLen);

    // groups_by_arn[arn] => list of groups (because ARN can repeat)
    std::map<std::string, std::vector<Group>> groups_by_arn;
    std::vector<std::string> global_inconsistencies;

    auto global_issue = [&](const std::string& msg) {
      global_inconsistencies.push_back(msg);
      };

    std::optional<Group> current;
    long long groupCounter = 0;

    auto flush_current = [&]() {
      if (!current.has_value()) return;
      Group& g = *current;
      std::string arn = g.arn.empty() ? "(missing_arn)" : g.arn;
      groups_by_arn[arn].push_back(std::move(g));
      current.reset();
      };

    for (long long i = 0; i < (long long)rawRecords.size(); ++i) {
      long long record_index = i + 1;
      const std::string& raw = rawRecords[(size_t)i];

      auto normOpt = normalize_record(raw, fmt);
      if (!normOpt) {
        std::ostringstream oss;
        oss << "Record length mismatch at record_index=" << record_index
          << " (expected " << rawLen << " raw bytes).";
        global_issue(oss.str());
        continue;
      }
      const std::string& r = *normOpt;

      std::string tc = trim_copy(field(r, 1, 2));
      std::string tcrS = trim_copy(field(r, 4, 1));
      char tcr = tcrS.empty() ? '?' : tcrS[0];

      if (tc != "05") {
        // Not TC05; ignore. If we are inside a group, note it (some feeds interleave; you may want to stop instead).
        if (current.has_value()) {
          std::ostringstream oss;
          oss << "Non-TC05 record encountered inside active group at record_index=" << record_index
            << " (tc='" << tc << "'). Record ignored.";
          add_inconsistency(*current, oss.str());
        }
        continue;
      }

      if (tcr < '0' || tcr > '7') {
        std::ostringstream oss;
        oss << "Unsupported TCR '" << tcr << "' for TC05 at record_index=" << record_index << ".";
        if (current.has_value()) add_inconsistency(*current, oss.str());
        else global_issue(oss.str());
        continue;
      }

      if (tcr == '0') {
        // Start a new group
        flush_current();

        Group g;
        g.group_id = ++groupCounter;

        ParsedRecord pr0 = parse_tc05_tcr0_7(r, '0', record_index);

        g.arn = pr0.fields.count("arn") ? trim_copy(pr0.fields["arn"]) : "";
        g.group_tag = make_group_tag_from_tcr0(pr0);

        // Validate + collect
        validate_sequence(g, 0, record_index);
        validate_record_basic(g, pr0);
        collect_dates(g, pr0);
        g.tcrs['0'].push_back(std::move(pr0));

        current = std::move(g);
        continue;
      }

      // If records appear before first TCR0, log and skip.
      if (!current.has_value()) {
        std::ostringstream oss;
        oss << "TC05 TCR" << tcr << " encountered before any TCR0 at record_index=" << record_index << " (ignored).";
        global_issue(oss.str());
        continue;
      }

      // Parse and attach to current group
      ParsedRecord pr = parse_tc05_tcr0_7(r, tcr, record_index);

      validate_sequence(*current, (int)(tcr - '0'), record_index);
      validate_record_basic(*current, pr);
      collect_dates(*current, pr);

      current->tcrs[tcr].push_back(std::move(pr));
    }

    flush_current();

    // Emit JSON
    std::cout << "{";
    std::cout << "\"groups_by_arn\":{";
    bool firstArn = true;
    for (const auto& kv : groups_by_arn) {
      if (!firstArn) std::cout << ",";
      firstArn = false;
      const std::string& arn = kv.first;
      const auto& groups = kv.second;

      std::cout << J(arn) << ":[";
      for (size_t gi = 0; gi < groups.size(); ++gi) {
        if (gi) std::cout << ",";
        dump_group_json(std::cout, groups[gi]);
      }
      std::cout << "]";
    }
    std::cout << "},";

    std::cout << "\"global_inconsistencies\":[";
    for (size_t k = 0; k < global_inconsistencies.size(); ++k) {
      if (k) std::cout << ",";
      std::cout << J(global_inconsistencies[k]);
    }
    std::cout << "]";
    std::cout << "}\n";

    return 0;

  }
  catch (const std::exception& ex) {
    std::cerr << "ERROR: " << ex.what() << "\n";
    return 1;
  }
}
