#include <algorithm>
#include <cctype>
#include <fstream>
#include <iostream>
#include <map>
#include <set>
#include <sstream>
#include <string>
#include <utility>
#include <vector>

// ----------------------- helpers: string -----------------------
static inline std::string rtrim(std::string s) {
  while (!s.empty() && (s.back() == ' ' || s.back() == '\t' || s.back() == '\r' || s.back() == '\n'))
    s.pop_back();
  return s;
}
static inline std::string ltrim(std::string s) {
  size_t i = 0;
  while (i < s.size() && (s[i] == ' ' || s[i] == '\t' || s[i] == '\r' || s[i] == '\n')) i++;
  return s.substr(i);
}
static inline std::string trim(std::string s) { return rtrim(ltrim(std::move(s))); }

static inline std::string slice_1based(const std::string& rec, int start_pos_1based, int len) {
  // Positions in spec are 1-based inclusive.
  int idx0 = start_pos_1based - 1;
  if (idx0 < 0) return "";
  if (idx0 >= (int)rec.size()) return "";
  int avail = (int)rec.size() - idx0;
  int take = std::min(len, avail);
  return rec.substr((size_t)idx0, (size_t)take);
}

static inline bool is_all_digits(const std::string& s) {
  for (unsigned char c : s) if (!std::isdigit(c)) return false;
  return !s.empty();
}

// ----------------------- helpers: JSON -----------------------
static std::string json_escape(const std::string& s) {
  std::ostringstream o;
  for (unsigned char c : s) {
    switch (c) {
    case '\"': o << "\\\""; break;
    case '\\': o << "\\\\"; break;
    case '\b': o << "\\b"; break;
    case '\f': o << "\\f"; break;
    case '\n': o << "\\n"; break;
    case '\r': o << "\\r"; break;
    case '\t': o << "\\t"; break;
    default:
      if (c < 0x20) {
        o << "\\u" << std::hex << std::uppercase
          << (int)0 << (int)0 << (int)0 << (int)c; // minimal
      }
      else {
        o << (char)c;
      }
    }
  }
  return o.str();
}

struct Json {
  // Very small JSON builder: object = map<string,string>, array = vector<Json>, string value only.
  enum class Type { OBJ, ARR, STR, NIL };
  Type type{ Type::NIL };

  std::map<std::string, Json> obj;
  std::vector<Json> arr;
  std::string str;

  static Json object() { Json j; j.type = Type::OBJ; return j; }
  static Json array() { Json j; j.type = Type::ARR; return j; }
  static Json string(std::string v) { Json j; j.type = Type::STR; j.str = std::move(v); return j; }
  static Json nil() { return Json{}; }

  void put(const std::string& k, Json v) { obj[k] = std::move(v); }
  void push(Json v) { arr.push_back(std::move(v)); }

  std::string dump(int indent = 0) const {
    std::ostringstream o;
    dump_impl(o, indent, 0);
    return o.str();
  }

  void dump_impl(std::ostringstream& o, int indent, int level) const {
    auto pad = [&](int lv) {
      for (int i = 0; i < lv * indent; i++) o << ' ';
      };

    switch (type) {
    case Type::NIL:
      o << "null";
      break;
    case Type::STR:
      o << "\"" << json_escape(str) << "\"";
      break;
    case Type::ARR: {
      o << "[";
      if (!arr.empty()) {
        if (indent) o << "\n";
        for (size_t i = 0; i < arr.size(); i++) {
          if (indent) pad(level + 1);
          arr[i].dump_impl(o, indent, level + 1);
          if (i + 1 < arr.size()) o << ",";
          if (indent) o << "\n";
        }
        if (indent) pad(level);
      }
      o << "]";
      break;
    }
    case Type::OBJ: {
      o << "{";
      if (!obj.empty()) {
        if (indent) o << "\n";
        size_t i = 0;
        for (const auto& kv : obj) {
          if (indent) pad(level + 1);
          o << "\"" << json_escape(kv.first) << "\": ";
          kv.second.dump_impl(o, indent, level + 1);
          if (++i < obj.size()) o << ",";
          if (indent) o << "\n";
        }
        if (indent) pad(level);
      }
      o << "}";
      break;
    }
    }
  }
};

// ----------------------- field parsing -----------------------
struct FieldDef {
  std::string name;
  int start; // 1-based
  int len;
};

static Json parse_fields(const std::string& rec, const std::vector<FieldDef>& defs) {
  Json o = Json::object();
  for (const auto& f : defs) {
    o.put(f.name, Json::string(trim(slice_1based(rec, f.start, f.len))));
  }
  return o;
}

// ----------------------- TC05 TCR layouts (partial but useful) -----------------------
// TCR0 key fields from spec layout (full record is 168) :contentReference[oaicite:10]{index=10}
static const std::vector<FieldDef> TCR0 = {
  {"transaction_code", 1, 2},
  {"qualifier", 3, 1},
  {"tcr", 4, 1},
  {"account_number", 5, 16},
  {"account_number_ext", 21, 3},
  {"acquirer_reference_number", 27, 23},
  {"acquirer_business_id", 50, 8},
  {"purchase_date_mmdd", 58, 4},            // MMDD :contentReference[oaicite:11]{index=11}
  {"destination_amount", 62, 12},
  {"destination_currency", 74, 3},
  {"source_amount", 77, 12},
  {"source_currency", 89, 3},
  {"merchant_name", 92, 25},
  {"merchant_city", 117, 13},
  {"merchant_country_code", 130, 3},
  {"merchant_category_code", 133, 4},
  {"central_processing_date_yddd", 164, 4},  // YDDD :contentReference[oaicite:12]{index=12}
  {"reimbursement_attribute", 168, 1}
};

// TCR1 Additional Data layout (selected, includes Conversion Date) :contentReference[oaicite:13]{index=13}
static const std::vector<FieldDef> TCR1 = {
  {"transaction_code", 1, 2},
  {"qualifier", 3, 1},
  {"tcr", 4, 1},
  {"business_format_code", 5, 1},
  {"token_assurance_method", 6, 2},
  {"rate_table_id", 8, 5},
  {"scheme_identifier", 13, 2},
  {"documentation_indicator", 23, 1},
  {"member_message_text", 24, 50},
  {"fee_program_indicator", 76, 3},
  {"card_acceptor_id", 81, 15},
  {"terminal_id", 96, 8},
  {"national_reimbursement_fee", 104, 12},
  {"conversion_date_yymm", 118, 4}, // "Conversion Date" field in spec :contentReference[oaicite:14]{index=14}
  {"cashback", 158, 9},
  {"pos_environment", 168, 1}
};

// TCR2 Argentina layout (selected) :contentReference[oaicite:15]{index=15}
static const std::vector<FieldDef> TCR2_AR = {
  {"transaction_code", 1, 2},
  {"qualifier", 3, 1},
  {"tcr", 4, 1},
  {"business_format_code", 5, 2},            // AR :contentReference[oaicite:16]{index=16}
  {"installment_plan", 7, 2},
  {"installment_count", 9, 2},
  {"installment_current", 11, 2},
  {"installment_grace_period", 13, 2},
  {"installment_type", 15, 2},
  {"vat_amount", 17, 8},
  {"vat_rate", 25, 4},
  {"other_tax_amount", 29, 8},
  {"other_tax_rate", 37, 4},
  {"deferred_cardholder_billing_date_yymmdd", 70, 6},
  {"deferred_settlement_date_yymmdd", 76, 6},
  {"deferred_settlement_date_original_yymmdd", 134, 6}
};

// TCR2 Uruguay layout (selected) :contentReference[oaicite:17]{index=17}
static const std::vector<FieldDef> TCR2_UY = {
  {"transaction_code", 1, 2},
  {"qualifier", 3, 1},
  {"tcr", 4, 1},
  {"business_format_code", 5, 2},          // UY :contentReference[oaicite:18]{index=18}
  {"deferred_settlement_date_yymmdd", 5, 6},
  {"deferred_settlement_date_original_yymmdd", 11, 6}
};

// TCR3 example minimal parse (layout is industry-specific; keep raw + header fields) :contentReference[oaicite:19]{index=19}
static const std::vector<FieldDef> TCR3_MIN = {
  {"transaction_code", 1, 2},
  {"qualifier", 3, 1},
  {"tcr", 4, 1},
  {"business_format_code", 5, 2},
  {"industry_subformat", 7, 2},
  {"raw_payload_9_168", 9, 160}
};

// TCR4 Supplemental Financial and Promotion Data (one of the TCR4 variants) :contentReference[oaicite:20]{index=20}
static const std::vector<FieldDef> TCR4 = {
  {"transaction_code", 1, 2},
  {"qualifier", 3, 1},
  {"tcr", 4, 1},
  {"enabler_verification_value", 5, 5},
  {"business_format_code", 15, 2},
  {"network_identification_code", 17, 4},
  {"contact_for_information", 21, 25},
  {"message_reason_code", 47, 4},
  {"surcharge_amount", 51, 8},
  {"promotion_type", 77, 2},
  {"promotion_code", 79, 25},
  {"payment_account_reference", 112, 29},
  {"token_requestor_id", 141, 11},
  {"total_discount_amount", 160, 8}
};

// TCR5 Payment Service Data :contentReference[oaicite:21]{index=21}
static const std::vector<FieldDef> TCR5 = {
  {"transaction_code", 1, 2},
  {"qualifier", 3, 1},
  {"tcr", 4, 1},
  {"transaction_identifier", 5, 15},
  {"authorized_amount", 20, 12},
  {"authorization_currency_code", 32, 3},
  {"authorization_response_code", 35, 2},
  {"merchant_telephone_number", 63, 14},
  {"interchange_fee_amount", 92, 15},
  {"source_to_base_fx", 108, 8},
  {"base_to_dest_fx", 116, 8},
  {"pan_token", 150, 16},
  {"cvv2_result_code", 168, 1}
};

// TCR6 Limited-Use Data (minimal) :contentReference[oaicite:22]{index=22}
static const std::vector<FieldDef> TCR6 = {
  {"transaction_code", 1, 2},
  {"qualifier", 3, 1},
  {"tcr", 4, 1},
  {"business_format_code", 5, 2},      // "LU" per spec :contentReference[oaicite:23]{index=23}
  {"limited_use_account_number", 7, 19},
  {"limited_use_id", 26, 3},
  {"limited_use_effective_date_yymmdd", 29, 6},
  {"limited_use_expiration_date_yymmdd", 35, 6},
  {"limited_use_data_raw_41_168", 41, 128}
};

// TCR7 Chip Card Transaction Data (selected, includes Terminal Transaction Date) :contentReference[oaicite:24]{index=24}
static const std::vector<FieldDef> TCR7 = {
  {"transaction_code", 1, 2},
  {"qualifier", 3, 1},
  {"tcr", 4, 1},
  {"transaction_type", 5, 2},
  {"card_sequence_number", 7, 3},
  {"terminal_transaction_date_yymmdd", 10, 6}, // :contentReference[oaicite:25]{index=25}
  {"terminal_country_code", 22, 3},
  {"cryptogram_amount", 87, 12},
  {"issuer_script_1_results", 159, 10}
};

// ----------------------- grouping -----------------------
struct GroupState {
  int group_id{ 0 };
  std::string group_tag;

  Json tcr0 = Json::nil();
  std::vector<Json> records;           // all TCRs (including TCR0 with tag)
  std::map<std::string, std::string> dates; // key->value (unique per key)
};

static std::string make_group_tag(const Json& tcr0obj) {
  // A readable identifier based on ARN + purchase date + merchant name.
  auto get = [&](const std::string& k)->std::string {
    auto it = tcr0obj.obj.find(k);
    if (it == tcr0obj.obj.end() || it->second.type != Json::Type::STR) return "";
    return it->second.str;
    };
  std::string arn = get("acquirer_reference_number");
  std::string mmdd = get("purchase_date_mmdd");
  std::string merch = get("merchant_name");
  std::string city = get("merchant_city");
  std::string ctry = get("merchant_country_code");

  std::ostringstream o;
  o << "TC05";
  if (!arn.empty()) o << " ARN=" << arn;
  if (!mmdd.empty()) o << " MMDD=" << mmdd;
  if (!merch.empty()) o << " MERCHANT=" << trim(merch);
  if (!city.empty()) o << " CITY=" << trim(city);
  if (!ctry.empty()) o << " COUNTRY=" << trim(ctry);
  return o.str();
}

static void add_date(GroupState& g, const std::string& key, const std::string& value) {
  std::string v = trim(value);
  if (v.empty()) return;
  // Keep first value for a given key (or replace if you prefer).
  if (g.dates.find(key) == g.dates.end()) g.dates[key] = v;
}

// ----------------------- main parsing logic -----------------------
static bool read_fixed_record(std::istream& in, std::string& out, int fixedLen = 168) {
  // Many BASE II dumps are one record per line; sometimes records are fixed bytes.
  // This reads a line and then normalizes to 168 (pads or truncates).
  std::string line;
  if (!std::getline(in, line)) return false;
  if (!line.empty() && line.back() == '\r') line.pop_back();

  if ((int)line.size() < fixedLen) line += std::string((size_t)(fixedLen - (int)line.size()), ' ');
  if ((int)line.size() > fixedLen) line.resize((size_t)fixedLen);
  out = std::move(line);
  return true;
}

static char tcr_of(const std::string& rec) {
  std::string v = slice_1based(rec, 4, 1);
  return v.empty() ? '\0' : v[0];
}

static std::string tc_of(const std::string& rec) {
  return slice_1based(rec, 1, 2);
}

static Json parse_tc05_record(const std::string& rec, GroupState& g) {
  char tcr = tcr_of(rec);

  if (tcr == '0') {
    Json o = parse_fields(rec, TCR0);
    // group tag will be injected by caller after computed
    return o;
  }
  if (tcr == '1') return parse_fields(rec, TCR1);

  if (tcr == '2') {
    // Choose by Business Format Code (positions 5-6)
    std::string bfc = trim(slice_1based(rec, 5, 2));
    if (bfc == "AR") return parse_fields(rec, TCR2_AR);
    if (bfc == "UY") return parse_fields(rec, TCR2_UY);

    // Unknown variant -> store basic header + raw.
    Json o = Json::object();
    o.put("transaction_code", Json::string(trim(slice_1based(rec, 1, 2))));
    o.put("qualifier", Json::string(trim(slice_1based(rec, 3, 1))));
    o.put("tcr", Json::string(trim(slice_1based(rec, 4, 1))));
    o.put("business_format_code", Json::string(bfc));
    o.put("raw_1_168", Json::string(rec));
    return o;
  }

  if (tcr == '3') return parse_fields(rec, TCR3_MIN);
  if (tcr == '4') return parse_fields(rec, TCR4);
  if (tcr == '5') return parse_fields(rec, TCR5);
  if (tcr == '6') return parse_fields(rec, TCR6);
  if (tcr == '7') return parse_fields(rec, TCR7);

  // Unknown TCR
  Json o = Json::object();
  o.put("transaction_code", Json::string(trim(slice_1based(rec, 1, 2))));
  o.put("qualifier", Json::string(trim(slice_1based(rec, 3, 1))));
  o.put("tcr", Json::string(std::string(1, tcr)));
  o.put("raw_1_168", Json::string(rec));
  return o;
}

static void extract_dates_for_group(GroupState& g, const Json& parsedRecord) {
  // Pull known date fields if present.
  auto pull = [&](const std::string& key, const std::string& outKey) {
    auto it = parsedRecord.obj.find(key);
    if (it != parsedRecord.obj.end() && it->second.type == Json::Type::STR) add_date(g, outKey, it->second.str);
    };

  pull("purchase_date_mmdd", "tcr0_purchase_date_mmdd");
  pull("central_processing_date_yddd", "tcr0_central_processing_date_yddd");
  pull("conversion_date_yymm", "tcr1_conversion_date");
  pull("deferred_cardholder_billing_date_yymmdd", "tcr2_deferred_cardholder_billing_date");
  pull("deferred_settlement_date_yymmdd", "tcr2_deferred_settlement_date");
  pull("deferred_settlement_date_original_yymmdd", "tcr2_deferred_settlement_date_original");
  pull("limited_use_effective_date_yymmdd", "tcr6_limited_use_effective_date");
  pull("limited_use_expiration_date_yymmdd", "tcr6_limited_use_expiration_date");
  pull("terminal_transaction_date_yymmdd", "tcr7_terminal_transaction_date");
}

static Json group_to_json(const GroupState& g) {
  Json o = Json::object();
  o.put("group_id", Json::string(std::to_string(g.group_id)));
  o.put("group_tag", Json::string(g.group_tag));
  o.put("tcr0", g.tcr0);
  Json arr = Json::array();
  for (const auto& r : g.records) arr.push(r);
  o.put("records", std::move(arr));
  return o;
}

static Json group_dates_to_json(const GroupState& g) {
  Json o = Json::object();
  o.put("group_id", Json::string(std::to_string(g.group_id)));
  o.put("group_tag", Json::string(g.group_tag));
  Json d = Json::object();
  for (const auto& kv : g.dates) d.put(kv.first, Json::string(kv.second));
  o.put("dates", std::move(d));
  return o;
}

int main(int argc, char** argv) {
  if (argc < 2) {
    std::cerr << "Usage: tc05_parser <input_records.txt> [groups.json] [group_dates.json]\n";
    std::cerr << "Assumptions: one 168-char record per line (program pads/truncates to 168).\n";
    return 1;
  }

  std::string inputPath = argv[1];
  std::string outGroups = (argc >= 3) ? argv[2] : "groups.json";
  std::string outDates = (argc >= 4) ? argv[3] : "group_dates.json";

  std::ifstream in(inputPath, std::ios::in);
  if (!in) {
    std::cerr << "ERROR: cannot open input file: " << inputPath << "\n";
    return 1;
  }

  Json groupsArr = Json::array();
  Json datesArr = Json::array();

  GroupState cur;
  bool inGroup = false;
  int groupCounter = 0;

  std::string rec;
  while (read_fixed_record(in, rec, 168)) {
    std::string tc = trim(tc_of(rec));
    if (tc != "05") {
      continue; // only TC05 requested
    }

    char tcr = tcr_of(rec);

    if (tcr == '0') {
      // finalize previous group
      if (inGroup) {
        groupsArr.push(group_to_json(cur));
        datesArr.push(group_dates_to_json(cur));
      }

      // start new group
      cur = GroupState{};
      cur.group_id = ++groupCounter;
      inGroup = true;

      Json tcr0Parsed = parse_tc05_record(rec, cur);
      cur.group_tag = make_group_tag(tcr0Parsed);

      // Add identifying text/tag to the TCR0 JSON itself (as requested)
      tcr0Parsed.put("group_tag", Json::string(cur.group_tag));
      tcr0Parsed.put("group_id", Json::string(std::to_string(cur.group_id)));

      cur.tcr0 = tcr0Parsed;
      cur.records.push_back(tcr0Parsed);

      extract_dates_for_group(cur, tcr0Parsed);
      continue;
    }

    // If file starts with non-TCR0 (shouldn't for a clean extract), create a synthetic group.
    if (!inGroup) {
      cur = GroupState{};
      cur.group_id = ++groupCounter;
      cur.group_tag = "TC05 GROUP=" + std::to_string(cur.group_id) + " (synthetic-start-no-TCR0)";
      inGroup = true;
    }

    Json parsed = parse_tc05_record(rec, cur);
    // attach group id/tag to each record too (handy for downstream)
    parsed.put("group_id", Json::string(std::to_string(cur.group_id)));
    parsed.put("group_tag", Json::string(cur.group_tag));

    cur.records.push_back(parsed);
    extract_dates_for_group(cur, parsed);
  }

  // finalize last group
  if (inGroup) {
    groupsArr.push(group_to_json(cur));
    datesArr.push(group_dates_to_json(cur));
  }

  // write files
  {
    std::ofstream out(outGroups, std::ios::out | std::ios::trunc);
    if (!out) {
      std::cerr << "ERROR: cannot open output file: " << outGroups << "\n";
      return 1;
    }
    out << groupsArr.dump(2) << "\n";
  }
  {
    std::ofstream out(outDates, std::ios::out | std::ios::trunc);
    if (!out) {
      std::cerr << "ERROR: cannot open output file: " << outDates << "\n";
      return 1;
    }
    out << datesArr.dump(2) << "\n";
  }

  std::cout << "OK\n";
  std::cout << "  Wrote: " << outGroups << "\n";
  std::cout << "  Wrote: " << outDates << "\n";
  std::cout << "  Groups: " << groupCounter << "\n";
  return 0;
}
