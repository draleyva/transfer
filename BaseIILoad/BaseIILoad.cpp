// tc05_date_error_repro_stream_defs.cpp
//
// Same streaming program as before, but now ALL offsets/lengths are defined via macros
// (1-based positions as in BASE II layouts / videfs.h comments).
//
// Build:
//   g++ -std=c++17 -O2 -Wall -Wextra tc05_date_error_repro_stream_defs.cpp -o tc05_date_error_repro_stream
//
// Usage:
//   ./tc05_date_error_repro_stream <input_file> [ctf|itf] [--anchor-year=YYYY]

#include <algorithm>
#include <cctype>
#include <ctime>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <optional>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

// =============================
// BASE II RECORD CONSTANTS
// =============================

// Raw record lengths
#define BASE2_LEN_CTF              168
#define BASE2_LEN_ITF              170

// Common header offsets (1-based)
#define BASE2_POS_TC               1
#define BASE2_LEN_TC               2

#define BASE2_POS_TCQ              3
#define BASE2_LEN_TCQ              1

#define BASE2_POS_TCR              4
#define BASE2_LEN_TCR              1

// -----------------------------
// TC05 / TCR0 offsets (1-based)
// Source: 25Q2/modules/visa/src/include/videfs.h
//   tc05_0_t.arn[23]      /* 27-49 */
//   tc05_0_t.purdate[4]   /* 58-61 MMDD */
//   tc05_0_t.cpd[4]       /* 164-167 YDDD */
// -----------------------------
#define TC05_TCR0_POS_ARN           27
#define TC05_TCR0_LEN_ARN           23

#define TC05_TCR0_POS_PURDATE       58
#define TC05_TCR0_LEN_PURDATE       4   // MMDD

#define TC05_TCR0_POS_CPD           164
#define TC05_TCR0_LEN_CPD           4   // YDDD

// base2imp.c line 2413 uses yddd2date(p_tcr0->arn + 7)
// That means: within ARN string (0-based) offset 7, length 4
#define TC05_TCR0_ARN_EMBED_OFS0     7   // 0-based within ARN
#define TC05_TCR0_ARN_EMBED_LEN      4   // YDDD embedded

// -----------------------------
// TC05 / TCR1 offsets (1-based)
// Source: videfs.h
//   tc05_1_t.currconvdate[4] /* 118-121 Currency Conversion Date (YDDD) */
// -----------------------------
#define TC05_TCR1_POS_CURRCONVDATE  118
#define TC05_TCR1_LEN_CURRCONVDATE  4   // YDDD

// -----------------------------
// TC05 / TCR7 offsets (1-based)
// You previously extracted terminal transaction date YYMMDD at 10–15.
// The code model in base2imp copies tc05_7_t.datetermtxn[6] raw (YYMMDD).
// Keep same positions to reproduce schema mismatch risk.
// -----------------------------
#define TC05_TCR7_POS_DATETERM      10
#define TC05_TCR7_LEN_DATETERM      6   // YYMMDD

// ITF normalization removal bytes: remove positions 3-4 (1-based)
#define ITF_REMOVE_POS1             3
#define ITF_REMOVE_LEN              2

// =============================

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

// 1-based slice helper
static inline std::string field_1based(const std::string& rec, int pos1, int len) {
  size_t start = (pos1 <= 0) ? 0 : (size_t)(pos1 - 1);
  if (start >= rec.size()) return "";
  size_t n = std::min((size_t)len, rec.size() - start);
  return rec.substr(start, n);
}

// Normalize ITF (170) -> CTF (168): remove bytes at positions 3-4 (1-based)
static std::optional<std::string> normalize_record(const std::string& rec, FileFormat fmt) {
  if (fmt == FileFormat::CTF) {
    if (rec.size() != BASE2_LEN_CTF) return std::nullopt;
    return rec;
  }
  if (rec.size() != BASE2_LEN_ITF) return std::nullopt;

  const size_t cut_start0 = (size_t)(ITF_REMOVE_POS1 - 1);
  const size_t cut_len = (size_t)ITF_REMOVE_LEN;

  std::string out;
  out.reserve(BASE2_LEN_CTF);
  out.append(rec, 0, cut_start0);
  out.append(rec, cut_start0 + cut_len, rec.size() - (cut_start0 + cut_len));
  if (out.size() != BASE2_LEN_CTF) return std::nullopt;
  return out;
}

// -------- base2imp-like date conversions (repro) --------
// base2imp.c: lf_mmdd2date() (769–791)
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
  if (m > current_month) y--; // last year
  return (long)(y * 10000 + m * 100 + d);
}

static int is_leap(int y) {
  if (y % 400 == 0) return 1;
  if (y % 100 == 0) return 0;
  return (y % 4 == 0) ? 1 : 0;
}
static int yy2ccyy(int yy) {
  if (yy >= 70) return 1900 + yy;
  return 2000 + yy;
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

// sldtm.c: yddd2date_ext() (2185–2251)
static long sldtm_yddd2date_ext(const std::string& yddd_str, int curYY) {
  if (yddd_str.size() < 4 || !is_digits(yddd_str.substr(0, 4))) return -1L;

  long yddd = std::stol(yddd_str.substr(0, 4));
  long day = yddd % 1000;
  long year_last = yddd / 1000;

  long tmp = curYY % 10;
  long year = year_last;

  if (tmp != year) {
    if (0 == year && 9 == tmp) year = curYY + 1;
    else if (9 == year && 0 == tmp) year = curYY - 1;
    else if (year <= tmp) year = curYY - (tmp - year);
    else if (year == tmp + 1) year = curYY + 1;
    else year = curYY - 10 + (year - tmp);
  }
  else {
    year = curYY;
  }

  int full_year = yy2ccyy((int)year);

  int mdays[12] = { 31,28,31,30,31,30,31,31,30,31,30,31 };
  if (is_leap(full_year)) mdays[1] = 29;

  long month = 1;
  for (long m = 1; m < 12; ++m) {
    long lim = mdays[m - 1];
    if (day > lim) day -= lim;
    else { month = m; break; }
    month = m + 1;
  }

  if (day >= 1 && day <= 31 && month >= 1 && month <= 12) {
    return 10000L * full_year + 100L * month + day;
  }
  return -1L;
}

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

// -------- group minimal fields --------
struct GroupMin {
  long long group_id{};
  long long start_record_index{};
  std::string arn;
  std::string purdate_mmdd;
  std::string cpd_yddd;
  std::string arn_embedded_yddd;
  std::string currconv_yddd;
  std::string dateterm_yymmdd;
  std::vector<std::string> errors;
};

static void add_err(GroupMin& g, std::string msg) { g.errors.push_back(std::move(msg)); }

static void analyze_group(GroupMin& g, int anchorYear) {
  const int yy = current_yy();

  // E1: MMDD weakness
  long mmdd_to = base2imp_lf_mmdd2date(g.purdate_mmdd);
  if (trim_copy(g.purdate_mmdd).empty()) {
    add_err(g, "E1: Missing TCR0.purdate(MMDD). (videfs.h tc05_0_t.purdate /*58-61*/)");
  }
  else if (mmdd_to == 0) {
    add_err(g,
      "E1: base2imp lf_mmdd2date() would return 0 for purdate (invalid format). "
      "Source: base2imp.c:769–791; used at base2imp.c:2416.");
  }
  else if (!is_valid_yyyymmdd(mmdd_to)) {
    std::ostringstream oss;
    oss << "E1: base2imp lf_mmdd2date() would ACCEPT purdate='" << g.purdate_mmdd
      << "' and output YYYYMMDD=" << mmdd_to
      << ", but it's NOT a real calendar date. "
      << "Source: base2imp.c:769–791; used at base2imp.c:2416.";
    add_err(g, oss.str());
  }

  // E2: YDDD -> -1
  auto check_yddd = [&](const char* label, const std::string& yddd, const char* where) {
    if (trim_copy(yddd).empty()) return;
    long out = sldtm_yddd2date_ext(yddd, yy);
    if (out == -1L) {
      std::ostringstream oss;
      oss << "E2: yddd2date_ext(" << label << "='" << yddd << "') returned -1 (invalid YDDD). "
        << "Source: sldtm.c:2185–2251; base2imp usage: " << where << ".";
      add_err(g, oss.str());
    }
    };
  check_yddd("cpd", g.cpd_yddd, "TCR0.cpd (videfs.h /*164-167*/) used for indaterec/compare");
  check_yddd("currconvdate", g.currconv_yddd, "base2imp.c:3091–3093 (int2ascfix(... yddd2date(currconvdate) ...))");
  check_yddd("arn+7", g.arn_embedded_yddd, "base2imp.c:2413 (datexmit = yddd2date(p_tcr0->arn+7))");

  // E3: runtime vs anchor mismatch
  auto compare_anchor = [&](const char* label, const std::string& yddd) {
    if (trim_copy(yddd).empty()) return;
    long runtime_yyyymmdd = sldtm_yddd2date_ext(yddd, yy);
    auto anchor_yyyymmdd = anchor_yddd_to_yyyymmdd(yddd, anchorYear);
    if (runtime_yyyymmdd > 0 && anchor_yyyymmdd && runtime_yyyymmdd != *anchor_yyyymmdd) {
      std::ostringstream oss;
      oss << "E3: Year ambiguity for " << label << " YDDD='" << yddd << "': "
        << "runtime(current_yy=" << yy << ") => " << runtime_yyyymmdd
        << " but anchor_year=" << anchorYear << " => " << *anchor_yyyymmdd
        << ". Sources: sldtm.c:2195–2213; base2imp.c:782–790.";
      add_err(g, oss.str());
    }
    };
  compare_anchor("TCR0.cpd", g.cpd_yddd);
  compare_anchor("TCR1.currconvdate", g.currconv_yddd);
  compare_anchor("ARN+7", g.arn_embedded_yddd);

  // E4: YYMMDD copied raw
  if (!trim_copy(g.dateterm_yymmdd).empty()) {
    if (g.dateterm_yymmdd.size() < 6 || !is_digits(g.dateterm_yymmdd.substr(0, 6))) {
      add_err(g,
        "E4: TCR7.datetermtxn is not 6 digits; Oracle DATE conversion can fail. "
        "Source: videfs.h tc05_7_t.datetermtxn[6]; base2imp.c:3226–3227 copies raw.");
    }
    else {
      auto yyyymmdd = yymmdd_to_yyyymmdd(g.dateterm_yymmdd);
      if (!yyyymmdd) {
        std::ostringstream oss;
        oss << "E4: TCR7.datetermtxn='" << g.dateterm_yymmdd << "' expands to invalid calendar date. "
          << "base2imp copies raw (base2imp.c:3226–3227).";
        add_err(g, oss.str());
      }
      else {
        std::ostringstream oss;
        oss << "E4 (schema mismatch risk): datetermtxn='" << g.dateterm_yymmdd
          << "' is YYMMDD and is copied as text; if Oracle expects DATE/ YYYYMMDD convert it (e.g., "
          << *yyyymmdd << "). Source: base2imp.c:3226–3227.";
        add_err(g, oss.str());
      }
    }
  }
}

static void print_group_report(const GroupMin& g) {
  std::cout << "============================================================\n";
  std::cout << "Group " << g.group_id << " (starts at record_index=" << g.start_record_index << ")\n";
  std::cout << "  ARN                      : '" << g.arn << "'\n";
  std::cout << "  TCR0.purdate (MMDD)       : '" << g.purdate_mmdd << "'   [pos " << TC05_TCR0_POS_PURDATE << " len " << TC05_TCR0_LEN_PURDATE << "]\n";
  std::cout << "  TCR0.cpd (YDDD)           : '" << g.cpd_yddd << "'   [pos " << TC05_TCR0_POS_CPD << " len " << TC05_TCR0_LEN_CPD << "]\n";
  std::cout << "  ARN+7 embedded (YDDD)     : '" << g.arn_embedded_yddd << "'   [ARN ofs0 " << TC05_TCR0_ARN_EMBED_OFS0 << " len " << TC05_TCR0_ARN_EMBED_LEN << "]\n";
  std::cout << "  TCR1.currconvdate (YDDD)  : '" << g.currconv_yddd << "'   [pos " << TC05_TCR1_POS_CURRCONVDATE << " len " << TC05_TCR1_LEN_CURRCONVDATE << "]\n";
  std::cout << "  TCR7.datetermtxn (YYMMDD) : '" << g.dateterm_yymmdd << "'   [pos " << TC05_TCR7_POS_DATETERM << " len " << TC05_TCR7_LEN_DATETERM << "]\n\n";

  if (g.errors.empty()) {
    std::cout << "No date-formatting issues detected by this streaming reproducer.\n\n";
    return;
  }

  std::cout << "ERRORS / RISKS FOUND:\n";
  for (const auto& e : g.errors) std::cout << "  - " << e << "\n";
  std::cout << "\n";
}

// Streaming reader
class RecordStream {
public:
  RecordStream(const std::string& path, size_t recLen)
    : in_(path, std::ios::binary), recLen_(recLen) {
    if (!in_) throw std::runtime_error("Cannot open input file: " + path);
    std::string peek;
    peek.resize(std::min<size_t>(8192, file_size_()));
    in_.read(&peek[0], (std::streamsize)peek.size());
    in_.clear();
    in_.seekg(0);
    lineMode_ = (peek.find('\n') != std::string::npos);
  }

  bool next(std::string& out) {
    if (!in_) return false;
    if (lineMode_) {
      std::string line;
      while (std::getline(in_, line)) {
        if (!line.empty() && line.back() == '\r') line.pop_back();
        if (line.empty()) continue;
        if (line.size() < recLen_) line.append(recLen_ - line.size(), ' ');
        out = line.substr(0, recLen_);
        return true;
      }
      return false;
    }
    else {
      out.resize(recLen_);
      in_.read(&out[0], (std::streamsize)recLen_);
      if (in_.gcount() == 0) return false;
      if ((size_t)in_.gcount() != recLen_) return false;
      return true;
    }
  }

private:
  std::ifstream in_;
  size_t recLen_;
  bool lineMode_{ false };

  size_t file_size_() {
    auto pos = in_.tellg();
    in_.seekg(0, std::ios::end);
    auto end = in_.tellg();
    in_.seekg(pos);
    if (end < 0) return 0;
    return (size_t)end;
  }
};

static std::optional<int> parse_int_arg(const std::string& s, const std::string& prefix) {
  if (s.rfind(prefix, 0) != 0) return std::nullopt;
  std::string v = s.substr(prefix.size());
  if (v.empty() || !is_digits(v)) return std::nullopt;
  return std::stoi(v);
}

int main(int argc, char** argv) {
  try {
    if (argc < 2) {
      std::cerr << "Usage: " << argv[0] << " <input_file> [ctf|itf] [--anchor-year=YYYY]\n";
      return 2;
    }

    std::string input = argv[1];
    FileFormat fmt = FileFormat::CTF;
    int anchorYear = 2026;

    for (int i = 2; i < argc; ++i) {
      std::string a = argv[i];
      std::string low = a;
      std::transform(low.begin(), low.end(), low.begin(), [](unsigned char c) { return std::tolower(c); });

      if (low == "ctf") fmt = FileFormat::CTF;
      else if (low == "itf") fmt = FileFormat::ITF;
      else if (auto v = parse_int_arg(low, "--anchor-year=")) anchorYear = *v;
      else throw std::runtime_error("Unknown argument: " + a);
    }

    const size_t rawLen = (fmt == FileFormat::CTF) ? (size_t)BASE2_LEN_CTF : (size_t)BASE2_LEN_ITF;
    RecordStream rs(input, rawLen);

    std::optional<GroupMin> current;
    long long gid = 0;
    long long record_index = 0;

    auto flush = [&]() {
      if (!current) return;
      analyze_group(*current, anchorYear);
      print_group_report(*current);
      current.reset();
      };

    std::string rawRec;
    while (rs.next(rawRec)) {
      ++record_index;

      auto normOpt = normalize_record(rawRec, fmt);
      if (!normOpt) continue;
      const std::string& r = *normOpt;

      std::string tc = trim_copy(field_1based(r, BASE2_POS_TC, BASE2_LEN_TC));
      std::string tcr = trim_copy(field_1based(r, BASE2_POS_TCR, BASE2_LEN_TCR));
      if (tc != "05" || tcr.empty()) continue;

      char tcrch = tcr[0];

      if (tcrch == '0') {
        flush();
        GroupMin g;
        g.group_id = ++gid;
        g.start_record_index = record_index;

        g.arn = trim_copy(field_1based(r, TC05_TCR0_POS_ARN, TC05_TCR0_LEN_ARN));
        g.purdate_mmdd = trim_copy(field_1based(r, TC05_TCR0_POS_PURDATE, TC05_TCR0_LEN_PURDATE));
        g.cpd_yddd = trim_copy(field_1based(r, TC05_TCR0_POS_CPD, TC05_TCR0_LEN_CPD));

        if ((int)g.arn.size() >= (TC05_TCR0_ARN_EMBED_OFS0 + TC05_TCR0_ARN_EMBED_LEN)) {
          g.arn_embedded_yddd = g.arn.substr((size_t)TC05_TCR0_ARN_EMBED_OFS0, (size_t)TC05_TCR0_ARN_EMBED_LEN);
        }

        current = std::move(g);
        continue;
      }

      if (!current) continue;

      if (tcrch == '1') {
        current->currconv_yddd = trim_copy(field_1based(r, TC05_TCR1_POS_CURRCONVDATE, TC05_TCR1_LEN_CURRCONVDATE));
      }
      else if (tcrch == '7') {
        current->dateterm_yymmdd = trim_copy(field_1based(r, TC05_TCR7_POS_DATETERM, TC05_TCR7_LEN_DATETERM));
      }
    }

    flush();
    return 0;
  }
  catch (const std::exception& ex) {
    std::cerr << "ERROR: " << ex.what() << "\n";
    return 1;
  }
}
