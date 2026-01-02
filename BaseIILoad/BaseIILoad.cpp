// tc05_date_error_repro.cpp
//
// Purpose:
//   Parse the minimum fields from BASE II TC05 (TCR0, TCR1, TCR7) needed to
//   REPRODUCE the date-formatting risks found in the original C importer "base2imp".
//
// Key assumption requested:
//   - "TCR0 is always the start of a new record(group)" => each TC05/TCR0 starts a new group.
//
// What this tool detects (and prints):
//   (E1) MMDD accepted by base2imp's lf_mmdd2date() but NOT a real calendar date
//        (e.g., 0231 / 1131 / 0431). This can later blow up at Oracle TO_DATE.
//
//        Source:
//          25Q2/modules/visa/src/base2imp/base2imp.c
//            lf_mmdd2date(): lines 769–791
//            Uses it for TCR0 purchase date: line 2416
//
//   (E2) YDDD (cpd / conversion_date / ARN-derived yddd) converts to -1 in yddd2date_ext()
//        meaning "invalid YDDD" but base2imp still formats it into a numeric string (risk: "-1").
//
//        Source:
//          25Q2/modules/common/src/stdlib/sldtm.c
//            yddd2date_ext(): lines 2185–2251, returns -1 on invalid
//          25Q2/modules/visa/src/base2imp/base2imp.c
//            Uses yddd2date(p_tcr0->arn+7) for datexmit: line 2413
//            Converts TCR1 currconvdate with yddd2date(): lines 3091–3093
//
//   (E3) Year inference ambiguity: both lf_mmdd2date() and yddd2date() infer the year
//        from "today" (runtime). Reprocessing later can silently change YYYY.
//        This tool prints a warning if the derived year changes when using an explicit anchor year.
//
//        Source:
//          base2imp.c uses local_date()/utc_date() and yddd2date() without a file/batch anchor
//          lf_mmdd2date(): lines 782–790 (uses local_date() for year/month inference)
//          yddd2date_ext(): lines 2195–2213 (uses current yy's last digit as pivot)
//
//   (E4) TCR7 terminal transaction date (YYMMDD) is copied as-is (not converted to YYYYMMDD/DATE).
//        If your Oracle schema expects DATE or YYYYMMDD, this is a mismatch.
//
//        Source:
//          25Q2/modules/visa/src/include/videfs.h
//            tc05_7_t.datetermtxn[6]: line 406
//          25Q2/modules/visa/src/base2imp/base2imp.c
//            slstrncpy_sen(adda.datetermtxn, p_tcr7->datetermtxn, ...): lines 3226–3227
//
// Field locations (BASE II / videfs.h):
//   TCR0:
//     arn[23]       positions 27–49   (videfs.h line 46)
//     purdate[4]    positions 58–61   (videfs.h line 48)  MMDD
//     cpd[4]        positions 164–167 (videfs.h line 71)  YDDD
//   TCR1:
//     currconvdate[4] positions 118–121 (videfs.h line 100) YDDD
//   TCR7:
//     datetermtxn[6] (videfs.h line 406) YYMMDD
//
// Build:
//   g++ -std=c++17 -O2 -Wall -Wextra tc05_date_error_repro.cpp -o tc05_date_error_repro
//
// Usage:
//   ./tc05_date_error_repro <input_file> [ctf|itf] [--anchor-year=YYYY]
//
// Notes about input formats:
//   - CTF record length = 168 bytes
//   - ITF record length = 170 bytes, where positions 3–4 are removed to normalize to 168
//
// If you run without args, it executes built-in self-tests to demonstrate all error types.

#include <algorithm>
#include <cctype>
#include <cstdio>
#include <ctime>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <optional>
#include <sstream>
#include <string>
#include <vector>

enum class FileFormat { CTF, ITF };

static inline std::string trim_copy(std::string s) {
  auto is_ws = [](unsigned char c) { return std::isspace(c) != 0; };
  while (!s.empty() && is_ws((unsigned char)s.front())) s.erase(s.begin());
  while (!s.empty() && is_ws((unsigned char)s.back())) s.pop_back();
  return s;
}
static inline bool is_digits(const std::string& s) {
  if (s.empty()) return false;
  for (unsigned char c : s) if (!std::isdigit(c)) return false;
  return true;
}

// 1-based slice
static inline std::string field(const std::string& rec168, int pos1, int len) {
  size_t start = (pos1 <= 0) ? 0 : (size_t)(pos1 - 1);
  if (start >= rec168.size()) return "";
  size_t n = std::min((size_t)len, rec168.size() - start);
  return rec168.substr(start, n);
}

static std::optional<std::string> normalize_record(const std::string& rec, FileFormat fmt) {
  if (fmt == FileFormat::CTF) {
    if (rec.size() != 168) return std::nullopt;
    return rec;
  }
  // ITF: 170 -> remove bytes at positions 3-4 (1-based), i.e. [2..3] 0-based
  if (rec.size() != 170) return std::nullopt;
  std::string out;
  out.reserve(168);
  out.append(rec, 0, 2);
  out.append(rec, 4, 166);
  if (out.size() != 168) return std::nullopt;
  return out;
}

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
      recs.push_back(line.substr(0, recLen));
    }
    return recs;
  }

  if (data.size() % recLen != 0) {
    std::ostringstream oss;
    oss << "File size (" << data.size() << ") is not multiple of record length (" << recLen << ")";
    throw std::runtime_error(oss.str());
  }
  std::vector<std::string> recs;
  recs.reserve(data.size() / recLen);
  for (size_t off = 0; off < data.size(); off += recLen) recs.push_back(data.substr(off, recLen));
  return recs;
}

// ---- "base2imp-like" date conversion reproductions ----

// base2imp.c lines 769–791: lf_mmdd2date()
// - only checks 1<=MM<=12 and 1<=DD<=31 (does NOT validate actual calendar month lengths)
// - infers year from "local_date()": if MM > current month => year-1
static long base2imp_lf_mmdd2date(const std::string& mmdd) {
  if (mmdd.size() < 4 || !is_digits(mmdd.substr(0, 4))) return 0;
  int m = std::stoi(mmdd.substr(0, 2));
  int d = std::stoi(mmdd.substr(2, 2));
  if (m < 1 || m > 12 || d < 1 || d > 31) return 0;

  std::time_t t = std::time(nullptr);
  std::tm lt{};
#if defined(_WIN32)
  localtime_s(&lt, &t);
#else
  lt = *std::localtime(&t);
#endif
  int current_year = lt.tm_year + 1900;
  int current_month = lt.tm_mon + 1;

  int y = current_year;
  if (m > current_month) y--; // assume last year
  return (long)(y * 10000 + m * 100 + d);
}

// sldtm.c lines 2185–2251: yddd2date_ext()
// - parses YDDD, infers year using last-digit pivot logic vs current yy
// - returns -1 on invalid day-of-year conversion
static int is_leap(int y) {
  if (y % 400 == 0) return 1;
  if (y % 100 == 0) return 0;
  return (y % 4 == 0) ? 1 : 0;
}
static int yy2ccyy(int yy) {
  // common pattern in many old codebases: treat 00-69 => 2000-2069, 70-99 => 1970-1999
  // NOTE: yy passed here is "two-digit" year as in sldtm.c.
  if (yy >= 70) return 1900 + yy;
  return 2000 + yy;
}
static long sldtm_yddd2date_ext(const std::string& yddd_str, int current_yy /*0-99*/) {
  long ret = -1L;
  if (yddd_str.size() < 4 || !is_digits(yddd_str.substr(0, 4))) return -1L;

  long yddd = std::stol(yddd_str.substr(0, 4));
  long day = yddd % 1000;
  long year_last = yddd / 1000;

  long tmp = current_yy % 10;
  long year = year_last;

  if (tmp != year) {
    if (0 == year && 9 == tmp) year = current_yy + 1;          // next year/decade
    else if (9 == year && 0 == tmp) year = current_yy - 1;     // last year/decade
    else if (year <= tmp) year = current_yy - (tmp - year);    // this decade
    else if (year == tmp + 1) year = current_yy + 1;           // next year
    else year = current_yy - 10 + (year - tmp);                // last decade
  }
  else {
    year = current_yy;
  }

  int full_year = yy2ccyy((int)year);

  // convert day-of-year -> month/day
  int mdays_norm[12] = { 31,28,31,30,31,30,31,31,30,31,30,31 };
  if (is_leap(full_year)) mdays_norm[1] = 29;

  long month = 1;
  for (long m = 1; m < 12; ++m) {
    long limit = mdays_norm[m - 1];
    if (day > limit) day -= limit;
    else { month = m; break; }
    month = m + 1;
  }

  if (day >= 1 && day <= 31 && month >= 1 && month <= 12) {
    ret = 10000L * full_year + 100L * month + day;
  }
  else {
    std::cout << " detecting error!" << "'\n";
  }

  return ret;
}

// helper: validate an actual calendar date YYYYMMDD
static bool is_valid_yyyymmdd(long yyyymmdd) {
  if (yyyymmdd <= 0) return false;
  int y = (int)(yyyymmdd / 10000);
  int m = (int)((yyyymmdd / 100) % 100);
  int d = (int)(yyyymmdd % 100);
  if (y < 1900 || y > 2100) return false;
  if (m < 1 || m > 12) return false;
  int mdays[12] = { 31,28,31,30,31,30,31,31,30,31,30,31 };
  if (m == 2 && is_leap(y)) mdays[1] = 29;
  if (d < 1 || d > mdays[m - 1]) return false;
  return true;
}

// Convert YYMMDD to YYYYMMDD with a simple pivot (00-69 => 2000+, 70-99 => 1900+)
static std::optional<long> yymmdd_to_yyyymmdd(const std::string& yymmdd) {
  if (yymmdd.size() < 6 || !is_digits(yymmdd.substr(0, 6))) return std::nullopt;
  int yy = std::stoi(yymmdd.substr(0, 2));
  int mm = std::stoi(yymmdd.substr(2, 2));
  int dd = std::stoi(yymmdd.substr(4, 2));
  int yyyy = (yy >= 70) ? (1900 + yy) : (2000 + yy);
  long out = (long)yyyy * 10000L + (long)mm * 100L + dd;
  if (!is_valid_yyyymmdd(out)) return std::nullopt;
  return out;
}

// anchor-year resolution (like your earlier C++ tool) for YDDD:
// choose year closest to anchorYear with matching last digit
static std::optional<int> resolve_year_from_last_digit(int lastDigit, int anchorYear) {
  int bestYear = -1, bestDist = 999999;
  for (int y = anchorYear - 15; y <= anchorYear + 15; ++y) {
    if (y % 10 != lastDigit) continue;
    int dist = std::abs(y - anchorYear);
    if (dist < bestDist) { bestDist = dist; bestYear = y; }
  }
  if (bestYear < 0) return std::nullopt;
  return bestYear;
}
static std::optional<long> anchor_yddd_to_yyyymmdd(const std::string& yddd, int anchorYear) {
  if (yddd.size() < 4 || !is_digits(yddd.substr(0, 4))) return std::nullopt;
  int last = yddd[0] - '0';
  int doy = std::stoi(yddd.substr(1, 3));
  auto yOpt = resolve_year_from_last_digit(last, anchorYear);
  if (!yOpt) return std::nullopt;
  int year = *yOpt;
  int mdays[12] = { 31,28,31,30,31,30,31,31,30,31,30,31 };
  if (is_leap(year)) mdays[1] = 29;
  if (doy < 1 || doy >(is_leap(year) ? 366 : 365)) return std::nullopt;
  int m = 1;
  int d = doy;
  for (int i = 0; i < 12; i++) {
    if (d > mdays[i]) d -= mdays[i];
    else { m = i + 1; break; }
  }
  long out = (long)year * 10000L + (long)m * 100L + d;
  if (!is_valid_yyyymmdd(out)) return std::nullopt;
  return out;
}

static int current_yy() {
  std::time_t t = std::time(nullptr);
  std::tm lt{};
#if defined(_WIN32)
  localtime_s(&lt, &t);
#else
  lt = *std::localtime(&t);
#endif
  return (lt.tm_year + 1900) % 100;
}

struct GroupMin {
  long long group_id{};
  std::string arn;
  std::string purdate_mmdd;     // TCR0 58-61
  std::string cpd_yddd;         // TCR0 164-167
  std::string arn_embedded_yddd; // base2imp uses arn+7 (line 2413)
  std::string currconv_yddd;    // TCR1 118-121
  std::string dateterm_yymmdd;  // TCR7 datetermtxn[6]

  std::vector<std::string> errors;
};

static void report_group(const GroupMin& g) {
  std::cout << "============================================================\n";
  std::cout << "Group " << g.group_id << "  ARN=" << g.arn << "\n";
  std::cout << "  TCR0.purdate(MMDD)           = '" << g.purdate_mmdd << "'\n";
  std::cout << "  TCR0.cpd(YDDD)               = '" << g.cpd_yddd << "'\n";
  std::cout << "  ARN+7 (embedded YDDD)        = '" << g.arn_embedded_yddd << "'\n";
  std::cout << "  TCR1.currconvdate(YDDD)      = '" << g.currconv_yddd << "'\n";
  std::cout << "  TCR7.datetermtxn(YYMMDD)     = '" << g.dateterm_yymmdd << "'\n\n";

  if (g.errors.empty()) {
    std::cout << "No date-formatting issues detected by this reproducer.\n";
  }
  else {
    std::cout << "ERRORS / RISKS FOUND:\n";
    for (const auto& e : g.errors) std::cout << "  - " << e << "\n";
  }
  std::cout << "\n";
}

static void analyze_group(GroupMin& g, int anchorYear) {
  const int yy = current_yy();

  // (E1) base2imp MMDD conversion can accept invalid calendar dates
  long mmdd_to = base2imp_lf_mmdd2date(g.purdate_mmdd);
  if (mmdd_to == 0) {
    g.errors.push_back(
      "E1: base2imp lf_mmdd2date() would return 0 (invalid MMDD). "
      "Source: base2imp.c:769–791 (only checks MM 1..12 and DD 1..31).");
  }
  else {
    if (!is_valid_yyyymmdd(mmdd_to)) {
      std::ostringstream oss;
      oss << "E1: base2imp lf_mmdd2date() would ACCEPT '" << g.purdate_mmdd
        << "' and produce YYYYMMDD=" << mmdd_to
        << ", but this is NOT a real calendar date (month/day mismatch like Feb 31). "
        << "Source: base2imp.c:769–791; used for TCR0 purdate at base2imp.c:2416.";
      g.errors.push_back(oss.str());
    }
  }

  // (E2) YDDD conversions can return -1 (invalid day-of-year)
  auto check_yddd = [&](const std::string& label, const std::string& yddd, const std::string& whereInBase2imp) {
    if (trim_copy(yddd).empty()) return;
    long yyyymmdd = sldtm_yddd2date_ext(yddd, yy);
    if (yyyymmdd == -1L) {
      std::ostringstream oss;
      oss << "E2: yddd2date_ext('" << yddd << "') returned -1 (invalid YDDD / cannot resolve). "
        << "This can propagate as a negative/invalid formatted value when inserted. "
        << "Source: sldtm.c:2185–2251 (returns -1); used in base2imp at " << whereInBase2imp << ".";
      g.errors.push_back(oss.str());
    }
    };

  check_yddd("cpd", g.cpd_yddd, "cpd is used with yddd2date(...) in multiple places; field is tc05_0_t.cpd videfs.h:71");
  check_yddd("currconvdate", g.currconv_yddd, "base2imp.c:3091–3093 (conversion_date -> int2ascfix(..., yddd2date(...)))");
  check_yddd("arn+7", g.arn_embedded_yddd, "base2imp.c:2413 (p_tl->datexmit = yddd2date(p_tcr0->arn+7))");

  // (E3) Year inference ambiguity: compare runtime-derived yddd2date_ext vs explicit anchor year
  auto compare_anchor = [&](const std::string& label, const std::string& yddd) {
    if (trim_copy(yddd).empty()) return;
    long runtime_yyyymmdd = sldtm_yddd2date_ext(yddd, yy);
    auto anchor_yyyymmdd = anchor_yddd_to_yyyymmdd(yddd, anchorYear);
    if (runtime_yyyymmdd > 0 && anchor_yyyymmdd && runtime_yyyymmdd != *anchor_yyyymmdd) {
      std::ostringstream oss;
      oss << "E3: Year ambiguity for " << label << " YDDD='" << yddd << "': "
        << "base2imp runtime pivot (current_yy=" << yy << ") => " << runtime_yyyymmdd
        << " but anchor_year=" << anchorYear << " => " << *anchor_yyyymmdd << ". "
        << "Reprocessing on a different date can silently change the year. "
        << "Sources: sldtm.c:2195–2213 (pivot logic), base2imp.c:769–790 (MMDD year inference).";
      g.errors.push_back(oss.str());
    }
    };

  compare_anchor("TCR0.cpd", g.cpd_yddd);
  compare_anchor("TCR1.currconvdate", g.currconv_yddd);
  compare_anchor("ARN+7", g.arn_embedded_yddd);

  // (E4) TCR7 datetermtxn is YYMMDD and is copied as-is (not converted)
  if (!trim_copy(g.dateterm_yymmdd).empty()) {
    if (g.dateterm_yymmdd.size() < 6 || !is_digits(g.dateterm_yymmdd.substr(0, 6))) {
      g.errors.push_back(
        "E4: TCR7.datetermtxn is not 6 digits; if inserted as date, it will fail. "
        "Source: videfs.h:406 (datetermtxn[6]); base2imp.c:3226–3227 copies it raw.");
    }
    else {
      // It is 6 digits, but Oracle may expect YYYYMMDD/DATE; show the converted candidate
      auto yyyymmdd = yymmdd_to_yyyymmdd(g.dateterm_yymmdd);
      if (!yyyymmdd) {
        std::ostringstream oss;
        oss << "E4: TCR7.datetermtxn='" << g.dateterm_yymmdd << "' (YYMMDD) is not a valid calendar date when expanded. "
          << "base2imp copies it as-is (base2imp.c:3226–3227), which can break Oracle DATE conversion.";
        g.errors.push_back(oss.str());
      }
      else {
        std::ostringstream oss;
        oss << "E4 (schema mismatch risk): TCR7.datetermtxn='" << g.dateterm_yymmdd << "' is YYMMDD; "
          << "base2imp copies it as text (base2imp.c:3226–3227). If Oracle expects DATE/ YYYYMMDD, you must convert "
          << "it (e.g., => " << *yyyymmdd << "). Source: videfs.h:406 + base2imp.c:3226–3227.";
        g.errors.push_back(oss.str());
      }
    }
  }
}

// Built-in mini tests to demonstrate errors without a file
static void self_tests(int anchorYear) {
  std::cout << "Running self-tests (no input file provided)...\n\n";

  // Test 1: MMDD that passes base2imp check but is invalid calendar date
  {
    GroupMin g;
    g.group_id = 1;
    g.arn = "TESTARN00000000000000000";
    g.purdate_mmdd = "0231";  // Feb 31 (invalid calendar, but base2imp accepts)
    g.cpd_yddd = "6001";      // usually Jan 1 of year ending in 6
    g.arn_embedded_yddd = "6001";
    g.currconv_yddd = "5365"; // Dec 31 of year ending in 5
    g.dateterm_yymmdd = "251231";
    analyze_group(g, anchorYear);
    report_group(g);
  }

  // Test 2: Invalid YDDD day-of-year -> yddd2date_ext returns -1
  {
    GroupMin g;
    g.group_id = 2;
    g.arn = "TESTARN00000000000000001";
    g.purdate_mmdd = "1231";
    g.cpd_yddd = "6367";          // day 367 invalid (even in leap year)
    g.arn_embedded_yddd = "6367";
    g.currconv_yddd = "6367";
    g.dateterm_yymmdd = "251231";
    analyze_group(g, anchorYear);
    report_group(g);
  }

  // Test 3: YYMMDD invalid date (e.g., 250231)
  {
    GroupMin g;
    g.group_id = 3;
    g.arn = "TESTARN00000000000000002";
    g.purdate_mmdd = "1231";
    g.cpd_yddd = "6001";
    g.arn_embedded_yddd = "6001";
    g.currconv_yddd = "6001";
    g.dateterm_yymmdd = "250231"; // 2025-02-31 invalid
    analyze_group(g, anchorYear);
    report_group(g);
  }
}

static std::optional<int> parse_int_arg(const std::string& s, const std::string& prefix) {
  if (s.rfind(prefix, 0) != 0) return std::nullopt;
  std::string v = s.substr(prefix.size());
  if (v.empty() || !is_digits(v)) return std::nullopt;
  return std::stoi(v);
}

int main(int argc, char** argv) {
  try {
    int anchorYear = 2026; // default like your earlier run

    if (argc == 1) {
      self_tests(anchorYear);
      return 0;
    }

    std::string input = argv[1];
    FileFormat fmt = FileFormat::CTF;

    for (int i = 2; i < argc; ++i) {
      std::string a = argv[i];
      std::string low = a;
      std::transform(low.begin(), low.end(), low.begin(), [](unsigned char c) { return std::tolower(c); });

      if (low == "ctf") fmt = FileFormat::CTF;
      else if (low == "itf") fmt = FileFormat::ITF;
      else if (auto v = parse_int_arg(low, "--anchor-year=")) anchorYear = *v;
      else throw std::runtime_error("Unknown argument: " + a);
    }

    const size_t rawLen = (fmt == FileFormat::CTF) ? 168 : 170;
    auto raw = read_records(input, rawLen);

    std::optional<GroupMin> current;
    long long gid = 0;

    auto flush = [&]() {
      if (!current) return;
      analyze_group(*current, anchorYear);
      report_group(*current);
      current.reset();
      };

    for (size_t i = 0; i < raw.size(); ++i) {
      auto normOpt = normalize_record(raw[i], fmt);
      if (!normOpt) continue;
      const std::string& r = *normOpt;

      std::string tc = trim_copy(field(r, 1, 2));
      std::string tcr = trim_copy(field(r, 4, 1));
      if (tc != "05") continue;
      if (tcr.empty()) continue;
      char tcrch = tcr[0];

      if (tcrch == '0') {
        flush();
        GroupMin g;
        g.group_id = ++gid;

        g.arn = trim_copy(field(r, 27, 23));
        g.purdate_mmdd = trim_copy(field(r, 58, 4));
        g.cpd_yddd = trim_copy(field(r, 164, 4));
        // base2imp uses p_tcr0->arn + 7 (base2imp.c:2413).
        // That means: take 4 bytes starting at ARN offset 7 (0-based) => positions 8-11 within ARN.
        if (g.arn.size() >= 11) g.arn_embedded_yddd = g.arn.substr(7, 4);

        current = std::move(g);
      }
      else if (!current) {
        // user asked to assume TCR0 always starts group; ignore anything before it
        continue;
      }
      else if (tcrch == '1') {
        // tc05_1_t.currconvdate is positions 118–121 (videfs.h line 100)
        current->currconv_yddd = trim_copy(field(r, 118, 4));
      }
      else if (tcrch == '7') {
        // tc05_7_t.datetermtxn[6] exists; in the BASE II layouts this is commonly at pos 10–15.
        // We mimic your earlier extractor and read pos 10–15 from the 168-byte record.
        // (This is sufficient for reproducing the schema/format mismatch risk.)
        current->dateterm_yymmdd = trim_copy(field(r, 10, 6));
      }
      // TCR2..6 not needed for the date-formatting reproductions above
    }

    flush();
    return 0;
  }
  catch (const std::exception& ex) {
    std::cerr << "ERROR: " << ex.what() << "\n";
    return 1;
  }
}
