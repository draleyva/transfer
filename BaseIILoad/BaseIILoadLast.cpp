// tc05_group_parser_dates.cpp
// Build: g++ -std=c++17 -O2 -Wall -Wextra tc05_group_parser_dates.cpp -o tc05_group_parser_dates
//
// Usage:
//   ./tc05_group_parser_dates <input_file> [ctf|itf] [--anchor-year=YYYY] [--max-fx-gap-days=N]
//
// Examples:
//   ./tc05_group_parser_dates tc05.dat ctf
//   ./tc05_group_parser_dates tc05.dat itf --anchor-year=2026 --max-fx-gap-days=3
//
// Key behavior:
// - Each TC05/TCR0 begins a NEW group.
// - Groups are stored under groups_by_arn[ARN] (ARNs can repeat => list of groups per ARN).
// - Dates sub-json includes both raw and resolved ISO forms when possible.
// - Inconsistencies include date-format issues + FX date gap checks (conversion_date vs central_processing_date).

#include <algorithm>
#include <cctype>
#include <chrono>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <map>
#include <optional>
#include <set>
#include <sstream>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

enum class FileFormat { CTF, ITF };

// ---------------- helpers ----------------
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
        oss << "\\u" << std::hex << std::setw(4) << std::setfill('0') << (int)c << std::dec;
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

// ---------------- date utilities ----------------
struct Ymd {
  int y = 0, m = 0, d = 0;
  bool valid = false;
};

static bool is_leap(int y) {
  if (y % 400 == 0) return true;
  if (y % 100 == 0) return false;
  return (y % 4 == 0);
}

static int doy_to_month_day(int year, int doy, int& out_month, int& out_day) {
  static const int mdays_norm[12] = { 31,28,31,30,31,30,31,31,30,31,30,31 };
  int mdays[12];
  for (int i = 0; i < 12; i++) mdays[i] = mdays_norm[i];
  if (is_leap(year)) mdays[1] = 29;

  if (doy < 1 || doy >(is_leap(year) ? 366 : 365)) return 0;

  int m = 0;
  while (m < 12 && doy > mdays[m]) {
    doy -= mdays[m];
    m++;
  }
  if (m >= 12) return 0;

  out_month = m + 1;
  out_day = doy;
  return 1;
}

static std::string ymd_to_iso(const Ymd& x) {
  if (!x.valid) return "";
  std::ostringstream oss;
  oss << std::setw(4) << std::setfill('0') << x.y << "-"
    << std::setw(2) << std::setfill('0') << x.m << "-"
    << std::setw(2) << std::setfill('0') << x.d;
  return oss.str();
}

// Parses YDDD (4 digits). Returns (year_last_digit, doy)
static std::optional<std::pair<int, int>> parse_yddd(const std::string& yddd) {
  std::string s = trim_copy(yddd);
  if (s.size() != 4 || !is_digits(s)) return std::nullopt;
  int y_last = s[0] - '0';
  int doy = std::stoi(s.substr(1, 3));
  return std::make_pair(y_last, doy);
}

// Resolve a YDDD into a full year using anchorYear.
// Rule: pick the year in [anchorYear-15, anchorYear+15] whose last digit matches and is closest to anchorYear.
static std::optional<int> resolve_year_from_last_digit(int y_last, int anchorYear) {
  int bestYear = -1;
  int bestDist = 1e9;
  for (int y = anchorYear - 15; y <= anchorYear + 15; ++y) {
    if ((y % 10) != y_last) continue;
    int dist = std::abs(y - anchorYear);
    if (dist < bestDist) { bestDist = dist; bestYear = y; }
  }
  if (bestYear < 0) return std::nullopt;
  return bestYear;
}

static Ymd resolve_yddd_to_ymd(const std::string& yddd, int anchorYear) {
  Ymd out;
  auto p = parse_yddd(yddd);
  if (!p) return out;
  auto yearOpt = resolve_year_from_last_digit(p->first, anchorYear);
  if (!yearOpt) return out;

  int month = 0, day = 0;
  if (!doy_to_month_day(*yearOpt, p->second, month, day)) return out;
  out.y = *yearOpt; out.m = month; out.d = day; out.valid = true;
  return out;
}

// Parse MMDD (4 digits) and resolve to a year near anchorYear, using a reference Ymd (e.g., central processing date).
static Ymd resolve_mmdd_to_ymd_near(const std::string& mmdd, const Ymd& ref) {
  Ymd out;
  std::string s = trim_copy(mmdd);
  if (s.size() != 4 || !is_digits(s) || !ref.valid) return out;
  int mm = std::stoi(s.substr(0, 2));
  int dd = std::stoi(s.substr(2, 2));
  if (mm < 1 || mm > 12 || dd < 1 || dd > 31) return out;

  // Candidate years: ref.y-1, ref.y, ref.y+1; choose the one that yields a real date and is closest to ref date.
  auto score = [&](int y)->std::optional<long long> {
    // validate day in month
    static const int mdays_norm[12] = { 31,28,31,30,31,30,31,31,30,31,30,31 };
    int mdays = mdays_norm[mm - 1];
    if (mm == 2 && is_leap(y)) mdays = 29;
    if (dd > mdays) return std::nullopt;

    // crude distance in days using chrono via tm/mktime
    std::tm t1{}; t1.tm_year = y - 1900; t1.tm_mon = mm - 1; t1.tm_mday = dd;
    std::tm t2{}; t2.tm_year = ref.y - 1900; t2.tm_mon = ref.m - 1; t2.tm_mday = ref.d;
    std::time_t tt1 = std::mktime(&t1);
    std::time_t tt2 = std::mktime(&t2);
    if (tt1 == (time_t)-1 || tt2 == (time_t)-1) return std::nullopt;
    long long diff = (long long)std::llabs((long long)tt1 - (long long)tt2) / 86400LL;
    return diff;
    };

  int bestY = 0;
  long long bestDiff = (1LL << 62);
  for (int y : {ref.y - 1, ref.y, ref.y + 1}) {
    auto d = score(y);
    if (!d) continue;
    if (*d < bestDiff) { bestDiff = *d; bestY = y; }
  }
  if (bestY == 0) return out;

  out.y = bestY; out.m = mm; out.d = dd; out.valid = true;
  return out;
}

static std::optional<long long> days_between(const Ymd& a, const Ymd& b) {
  if (!a.valid || !b.valid) return std::nullopt;
  std::tm ta{}; ta.tm_year = a.y - 1900; ta.tm_mon = a.m - 1; ta.tm_mday = a.d;
  std::tm tb{}; tb.tm_year = b.y - 1900; tb.tm_mon = b.m - 1; tb.tm_mday = b.d;
  std::time_t tta = std::mktime(&ta);
  std::time_t ttb = std::mktime(&tb);
  if (tta == (time_t)-1 || ttb == (time_t)-1) return std::nullopt;
  long long diff = ((long long)ttb - (long long)tta) / 86400LL;
  return diff;
}

// ---------------- parsed record model ----------------
struct ParsedRecord {
  char tcr = '?';
  long long record_index = 0;
  std::map<std::string, std::string> fields;
};

static ParsedRecord parse_tcr0(const std::string& r, long long idx) {
  ParsedRecord pr; pr.tcr = '0'; pr.record_index = idx;
  pr.fields["tc"] = trim_copy(field(r, 1, 2));
  pr.fields["tcq"] = trim_copy(field(r, 3, 1));
  pr.fields["tcr"] = trim_copy(field(r, 4, 1));
  pr.fields["account_number"] = trim_copy(field(r, 5, 16));
  pr.fields["arn"] = trim_copy(field(r, 27, 23));
  pr.fields["acquirer_business_id"] = trim_copy(field(r, 50, 8));
  pr.fields["purchase_date_mmdd"] = trim_copy(field(r, 58, 4));
  pr.fields["central_processing_date_yddd"] = trim_copy(field(r, 164, 4));
  return pr;
}

static ParsedRecord parse_tcr1(const std::string& r, long long idx) {
  ParsedRecord pr; pr.tcr = '1'; pr.record_index = idx;
  pr.fields["tc"] = trim_copy(field(r, 1, 2));
  pr.fields["tcq"] = trim_copy(field(r, 3, 1));
  pr.fields["tcr"] = trim_copy(field(r, 4, 1));
  // Conversion Date stored as provided; treated as YDDD by our validator.
  pr.fields["conversion_date_yddd"] = trim_copy(field(r, 118, 4)); // Conversion Date field. :contentReference[oaicite:2]{index=2}
  return pr;
}

static ParsedRecord parse_tcr2_generic(const std::string& r, long long idx) {
  ParsedRecord pr; pr.tcr = '2'; pr.record_index = idx;
  pr.fields["tc"] = trim_copy(field(r, 1, 2));
  pr.fields["tcq"] = trim_copy(field(r, 3, 1));
  pr.fields["tcr"] = trim_copy(field(r, 4, 1));
  pr.fields["business_format_code"] = trim_copy(field(r, 5, 2));
  return pr;
}

static ParsedRecord parse_tcr3_min(const std::string& r, long long idx) {
  ParsedRecord pr; pr.tcr = '3'; pr.record_index = idx;
  pr.fields["tc"] = trim_copy(field(r, 1, 2));
  pr.fields["tcq"] = trim_copy(field(r, 3, 1));
  pr.fields["tcr"] = trim_copy(field(r, 4, 1));
  pr.fields["business_format_code"] = trim_copy(field(r, 5, 2));
  pr.fields["industry_subformat"] = trim_copy(field(r, 7, 2));
  return pr;
}

static ParsedRecord parse_tcr4(const std::string& r, long long idx) {
  ParsedRecord pr; pr.tcr = '4'; pr.record_index = idx;
  pr.fields["tc"] = trim_copy(field(r, 1, 2));
  pr.fields["tcq"] = trim_copy(field(r, 3, 1));
  pr.fields["tcr"] = trim_copy(field(r, 4, 1));
  pr.fields["business_format_code"] = trim_copy(field(r, 15, 2));
  return pr;
}

static ParsedRecord parse_tcr5(const std::string& r, long long idx) {
  ParsedRecord pr; pr.tcr = '5'; pr.record_index = idx;
  pr.fields["tc"] = trim_copy(field(r, 1, 2));
  pr.fields["tcq"] = trim_copy(field(r, 3, 1));
  pr.fields["tcr"] = trim_copy(field(r, 4, 1));
  pr.fields["transaction_identifier"] = trim_copy(field(r, 5, 15));
  return pr;
}

static ParsedRecord parse_tcr6(const std::string& r, long long idx) {
  ParsedRecord pr; pr.tcr = '6'; pr.record_index = idx;
  pr.fields["tc"] = trim_copy(field(r, 1, 2));
  pr.fields["tcq"] = trim_copy(field(r, 3, 1));
  pr.fields["tcr"] = trim_copy(field(r, 4, 1));
  pr.fields["limited_use_effective_date_yymmdd"] = trim_copy(field(r, 29, 6));
  pr.fields["limited_use_expiration_date_yymmdd"] = trim_copy(field(r, 35, 6));
  return pr;
}

static ParsedRecord parse_tcr7(const std::string& r, long long idx) {
  ParsedRecord pr; pr.tcr = '7'; pr.record_index = idx;
  pr.fields["tc"] = trim_copy(field(r, 1, 2));
  pr.fields["tcq"] = trim_copy(field(r, 3, 1));
  pr.fields["tcr"] = trim_copy(field(r, 4, 1));
  pr.fields["terminal_transaction_date_yymmdd"] = trim_copy(field(r, 10, 6)); // :contentReference[oaicite:3]{index=3}
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
    ParsedRecord pr; pr.tcr = tcr; pr.record_index = idx;
    pr.fields["tc"] = trim_copy(field(r, 1, 2));
    pr.fields["tcq"] = trim_copy(field(r, 3, 1));
    pr.fields["tcr"] = trim_copy(field(r, 4, 1));
    return pr;
  }
  }
}

// ---------------- grouping model ----------------
struct Group {
  long long group_id = 0;
  std::string arn;
  std::string group_tag;

  std::map<char, std::vector<ParsedRecord>> tcrs;

  // Raw dates (unique sets)
  std::set<std::string> purchase_mmdd;
  std::set<std::string> central_yddd;
  std::set<std::string> conversion_yddd;
  std::set<std::string> terminal_yymmdd;
  std::set<std::string> limited_use_eff_yymmdd;
  std::set<std::string> limited_use_exp_yymmdd;

  // Resolved ISO dates (when possible)
  std::set<std::string> purchase_iso;
  std::set<std::string> central_iso;
  std::set<std::string> conversion_iso;

  std::vector<std::string> inconsistencies;

  bool started = false;
  int last_tcr_seen = -1;
};

static void add_issue(Group& g, const std::string& msg) { g.inconsistencies.push_back(msg); }

static std::string make_group_tag_from_tcr0(const ParsedRecord& pr0) {
  auto get = [&](const std::string& k)->std::string {
    auto it = pr0.fields.find(k);
    return (it == pr0.fields.end()) ? "" : trim_copy(it->second);
    };
  std::string arn = get("arn");
  std::string acq = get("acquirer_business_id");
  std::string pd = get("purchase_date_mmdd");
  std::ostringstream oss;
  oss << "TC05|ARN=" << arn << "|ACQ=" << acq << "|PD=" << pd;
  return oss.str();
}

static void validate_sequence(Group& g, int tcr_num, long long record_index) {
  if (!g.started) { g.started = true; g.last_tcr_seen = tcr_num; return; }
  if (tcr_num < g.last_tcr_seen) {
    std::ostringstream oss;
    oss << "TCR out-of-order inside group at record_index=" << record_index
      << " (saw TCR" << tcr_num << " after TCR" << g.last_tcr_seen << ")";
    add_issue(g, oss.str());
  }
  g.last_tcr_seen = std::max(g.last_tcr_seen, tcr_num);
}

static void collect_dates(Group& g, const ParsedRecord& pr) {
  auto add = [](std::set<std::string>& s, const std::string& v) {
    std::string t = trim_copy(v);
    if (!t.empty()) s.insert(t);
    };

  if (pr.tcr == '0') {
    if (pr.fields.count("purchase_date_mmdd")) add(g.purchase_mmdd, pr.fields.at("purchase_date_mmdd"));
    if (pr.fields.count("central_processing_date_yddd")) add(g.central_yddd, pr.fields.at("central_processing_date_yddd"));
  }
  if (pr.tcr == '1') {
    if (pr.fields.count("conversion_date_yddd")) add(g.conversion_yddd, pr.fields.at("conversion_date_yddd"));
  }
  if (pr.tcr == '7') {
    if (pr.fields.count("terminal_transaction_date_yymmdd")) add(g.terminal_yymmdd, pr.fields.at("terminal_transaction_date_yymmdd"));
  }
  if (pr.tcr == '6') {
    if (pr.fields.count("limited_use_effective_date_yymmdd")) add(g.limited_use_eff_yymmdd, pr.fields.at("limited_use_effective_date_yymmdd"));
    if (pr.fields.count("limited_use_expiration_date_yymmdd")) add(g.limited_use_exp_yymmdd, pr.fields.at("limited_use_expiration_date_yymmdd"));
  }
}

static void validate_basic_formats(Group& g, const ParsedRecord& pr) {
  auto check_digits_len = [&](const std::string& key, size_t len, const std::string& label) {
    auto it = pr.fields.find(key);
    if (it == pr.fields.end()) return;
    std::string v = trim_copy(it->second);
    if (v.empty()) return;
    if (v.size() != len || !is_digits(v)) {
      std::ostringstream oss;
      oss << "Invalid " << label << " in TCR" << pr.tcr
        << " (record_index=" << pr.record_index << "): expected " << len << " digits, got '" << v << "'";
      add_issue(g, oss.str());
    }
    };

  if (pr.tcr == '0') {
    check_digits_len("purchase_date_mmdd", 4, "purchase_date_mmdd");
    check_digits_len("central_processing_date_yddd", 4, "central_processing_date_yddd");
    std::string arn = pr.fields.count("arn") ? trim_copy(pr.fields.at("arn")) : "";
    if (arn.empty()) add_issue(g, "Missing/blank ARN in TCR0 (group key not reliable).");
  }
  if (pr.tcr == '1') {
    check_digits_len("conversion_date_yddd", 4, "conversion_date_yddd(YDDD)");
  }
  if (pr.tcr == '7') {
    check_digits_len("terminal_transaction_date_yymmdd", 6, "terminal_transaction_date_yymmdd");
  }
  if (pr.tcr == '6') {
    check_digits_len("limited_use_effective_date_yymmdd", 6, "limited_use_effective_date_yymmdd");
    check_digits_len("limited_use_expiration_date_yymmdd", 6, "limited_use_expiration_date_yymmdd");
  }
}

// Resolve and cross-check conversion date vs central processing date (and optionally purchase date).
static void resolve_and_validate_dates(Group& g, int anchorYear, int maxFxGapDays) {
  // Use first central_processing_date_yddd as reference if available.
  std::optional<std::string> central_raw;
  if (!g.central_yddd.empty()) central_raw = *g.central_yddd.begin();

  Ymd central_ymd;
  if (central_raw) {
    central_ymd = resolve_yddd_to_ymd(*central_raw, anchorYear);
    if (central_ymd.valid) g.central_iso.insert(ymd_to_iso(central_ymd));
    else add_issue(g, "Unable to resolve central_processing_date_yddd=" + *central_raw + " to ISO date using anchorYear.");
  }

  // Purchase date MMDD can be resolved near central date (year inferred).
  if (central_ymd.valid) {
    for (const auto& mmdd : g.purchase_mmdd) {
      Ymd p = resolve_mmdd_to_ymd_near(mmdd, central_ymd);
      if (p.valid) g.purchase_iso.insert(ymd_to_iso(p));
      else add_issue(g, "Unable to resolve purchase_date_mmdd=" + mmdd + " near central processing date.");
    }
  }

  // Conversion date YDDD -> ISO and gap checks.
  for (const auto& c : g.conversion_yddd) {
    Ymd conv = resolve_yddd_to_ymd(c, anchorYear);
    if (!conv.valid) {
      add_issue(g, "Unable to resolve conversion_date_yddd=" + c + " to ISO date using anchorYear.");
      continue;
    }
    g.conversion_iso.insert(ymd_to_iso(conv));

    if (central_ymd.valid) {
      auto diffOpt = days_between(conv, central_ymd);
      if (diffOpt) {
        long long diff = *diffOpt;
        if (std::llabs(diff) > maxFxGapDays) {
          std::ostringstream oss;
          oss << "FX conversion_date seems far from central_processing_date: "
            << "conversion=" << ymd_to_iso(conv) << " (raw " << c << "), "
            << "central=" << ymd_to_iso(central_ymd) << " (raw " << *central_raw << "), "
            << "diff_days=" << diff << ", threshold=" << maxFxGapDays;
          add_issue(g, oss.str());
        }
      }
    }
  }
}

// ---------------- JSON output ----------------
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
  os << "}}";
}

static void dump_set(std::ostream& os, const std::set<std::string>& s) {
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
    << "\"raw\":{"
    << "\"purchase_date_mmdd\":"; dump_set(os, g.purchase_mmdd); os << ","
    << "\"central_processing_date_yddd\":"; dump_set(os, g.central_yddd); os << ","
    << "\"conversion_date_yddd\":"; dump_set(os, g.conversion_yddd); os << ","
    << "\"terminal_transaction_date_yymmdd\":"; dump_set(os, g.terminal_yymmdd); os << ","
    << "\"limited_use_effective_date_yymmdd\":"; dump_set(os, g.limited_use_eff_yymmdd); os << ","
    << "\"limited_use_expiration_date_yymmdd\":"; dump_set(os, g.limited_use_exp_yymmdd);
  os << "},"
    << "\"resolved_iso\":{"
    << "\"purchase_date\":"; dump_set(os, g.purchase_iso); os << ","
    << "\"central_processing_date\":"; dump_set(os, g.central_iso); os << ","
    << "\"conversion_date\":"; dump_set(os, g.conversion_iso);
  os << "}"
    << "},";

  os << "\"inconsistencies\":[";
  for (size_t i = 0; i < g.inconsistencies.size(); ++i) {
    if (i) os << ",";
    os << J(g.inconsistencies[i]);
  }
  os << "]"
    << "}";
}

// ---------------- main ----------------
static std::optional<int> parse_int_arg(const std::string& s, const std::string& prefix) {
  if (s.rfind(prefix, 0) != 0) return std::nullopt;
  std::string v = s.substr(prefix.size());
  if (v.empty() || !is_digits(v)) return std::nullopt;
  return std::stoi(v);
}

int main(int argc, char** argv) {
  try {
    if (argc < 2) {
      std::cerr << "Usage: " << argv[0] << " <input_file> [ctf|itf] [--anchor-year=YYYY] [--max-fx-gap-days=N]\n";
      return 2;
    }

    std::string input = argv[1];
    FileFormat fmt = FileFormat::CTF;

    int anchorYear = 2026;     // default (you can override)
    int maxFxGapDays = 7;      // default threshold

    // Parse optional args
    for (int i = 2; i < argc; i++) {
      std::string a = argv[i];
      std::string low = a;
      std::transform(low.begin(), low.end(), low.begin(), [](unsigned char c) { return std::tolower(c); });

      if (low == "ctf") fmt = FileFormat::CTF;
      else if (low == "itf") fmt = FileFormat::ITF;
      else if (auto v = parse_int_arg(low, "--anchor-year=")) anchorYear = *v;
      else if (auto v = parse_int_arg(low, "--max-fx-gap-days=")) maxFxGapDays = *v;
      else {
        throw std::runtime_error("Unknown argument: " + a);
      }
    }

    const size_t rawLen = (fmt == FileFormat::CTF) ? 168 : 170;
    auto rawRecords = read_records(input, rawLen);

    std::map<std::string, std::vector<Group>> groups_by_arn;
    std::vector<std::string> global_inconsistencies;

    auto global_issue = [&](const std::string& msg) { global_inconsistencies.push_back(msg); };

    std::optional<Group> current;
    long long groupCounter = 0;

    auto flush_current = [&]() {
      if (!current) return;
      // Resolve dates + validations that require the full group
      resolve_and_validate_dates(*current, anchorYear, maxFxGapDays);

      std::string arnKey = current->arn.empty() ? "(missing_arn)" : current->arn;
      groups_by_arn[arnKey].push_back(std::move(*current));
      current.reset();
      };

    for (long long i = 0; i < (long long)rawRecords.size(); ++i) {
      long long record_index = i + 1;
      auto normOpt = normalize_record(rawRecords[(size_t)i], fmt);
      if (!normOpt) {
        std::ostringstream oss;
        oss << "Record length mismatch at record_index=" << record_index << " (expected " << rawLen << " raw bytes).";
        global_issue(oss.str());
        continue;
      }
      const std::string& r = *normOpt;

      std::string tc = trim_copy(field(r, 1, 2));
      std::string tcrS = trim_copy(field(r, 4, 1));
      char tcr = tcrS.empty() ? '?' : tcrS[0];

      if (tc != "05") {
        if (current) {
          std::ostringstream oss;
          oss << "Non-TC05 record encountered inside active group at record_index=" << record_index
            << " (tc='" << tc << "'). Ignored.";
          add_issue(*current, oss.str());
        }
        continue;
      }

      if (tcr < '0' || tcr > '7') {
        std::ostringstream oss;
        oss << "Unsupported TCR '" << tcr << "' for TC05 at record_index=" << record_index << ".";
        if (current) add_issue(*current, oss.str());
        else global_issue(oss.str());
        continue;
      }

      if (tcr == '0') {
        flush_current();
        Group g;
        g.group_id = ++groupCounter;

        ParsedRecord pr0 = parse_tc05_tcr0_7(r, '0', record_index);
        g.arn = pr0.fields.count("arn") ? trim_copy(pr0.fields.at("arn")) : "";
        g.group_tag = make_group_tag_from_tcr0(pr0);

        validate_sequence(g, 0, record_index);
        validate_basic_formats(g, pr0);
        collect_dates(g, pr0);
        g.tcrs['0'].push_back(std::move(pr0));

        current = std::move(g);
        continue;
      }

      if (!current) {
        std::ostringstream oss;
        oss << "TC05 TCR" << tcr << " encountered before any TCR0 at record_index=" << record_index << " (ignored).";
        global_issue(oss.str());
        continue;
      }

      ParsedRecord pr = parse_tc05_tcr0_7(r, tcr, record_index);
      validate_sequence(*current, (int)(tcr - '0'), record_index);
      validate_basic_formats(*current, pr);
      collect_dates(*current, pr);
      current->tcrs[tcr].push_back(std::move(pr));
    }

    flush_current();

    // Output
    std::cout << "{";
    std::cout << "\"config\":{"
      << "\"anchor_year\":" << anchorYear << ","
      << "\"max_fx_gap_days\":" << maxFxGapDays << ","
      << "\"format\":" << J(fmt == FileFormat::CTF ? "ctf" : "itf")
      << "},";

    std::cout << "\"groups_by_arn\":{";
    bool firstArn = true;
    for (const auto& kv : groups_by_arn) {
      if (!firstArn) std::cout << ",";
      firstArn = false;
      std::cout << J(kv.first) << ":[";
      for (size_t gi = 0; gi < kv.second.size(); ++gi) {
        if (gi) std::cout << ",";
        dump_group_json(std::cout, kv.second[gi]);
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
