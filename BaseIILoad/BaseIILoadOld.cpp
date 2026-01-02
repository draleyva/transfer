// tc05_tcr0_tcr7_processor.cpp
// Build: g++ -std=c++17 -O2 -Wall -Wextra tc05_tcr0_tcr7_processor.cpp -o tc05proc
//
// Usage:
//   ./tc05proc <input_file> [ctf|itf]
//
// Output:
//   JSON lines to stdout for each parsed TC05 TCR0 and TCR7 record.
// Notes:
// - CTF: fixed 168-byte records. Each TCR is 168 bytes. :contentReference[oaicite:5]{index=5}
// - ITF: fixed 170-byte EBCDIC file; differs by 2-byte hash at positions 3-4. :contentReference[oaicite:6]{index=6}
//   This program "normalizes" ITF records by removing bytes at positions 3-4 so you can use CTF positions.
//

#include <algorithm>
#include <cctype>
#include <cstdint>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <optional>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

enum class FileFormat { CTF, ITF };

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

// 1-based [pos..pos+len-1] -> 0-based substr
static inline std::string field(const std::string& rec168, int pos1, int len) {
  if (pos1 <= 0 || len < 0) return {};
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

static inline bool is_all_digits(const std::string& s) {
  return std::all_of(s.begin(), s.end(), [](unsigned char c) { return std::isdigit(c) != 0; });
}

static inline std::optional<long long> to_int_opt(const std::string& s) {
  std::string t = trim_copy(s);
  if (t.empty() || !is_all_digits(t)) return std::nullopt;
  try {
    return std::stoll(t);
  }
  catch (...) {
    return std::nullopt;
  }
}

// Normalize ITF(170) -> CTF-like(168) by removing bytes at positions 3-4 (1-based).
static std::optional<std::string> normalize_record(const std::string& rec, FileFormat fmt) {
  if (fmt == FileFormat::CTF) {
    if (rec.size() != 168) return std::nullopt;
    return rec;
  }
  // ITF
  if (rec.size() != 170) return std::nullopt;
  std::string out;
  out.reserve(168);
  // keep positions 1-2 (0..1)
  out.append(rec, 0, 2);
  // skip positions 3-4 (2..3)
  // keep positions 5..170 (4..169)
  out.append(rec, 4, 166);
  if (out.size() != 168) return std::nullopt;
  return out;
}

struct Tc05Tcr0 {
  std::string tc;              // 1-2
  std::string tcq;             // 3
  std::string tcr;             // 4
  std::string accountNumber;   // 5-20
  std::string acctExt;         // 21-23
  std::string floorLimitInd;   // 24
  std::string crbExceptionInd; // 25
  std::string reserved26;      // 26
  std::string arn;             // 27-49
  std::string acquirerBizId;   // 50-57
  std::string purchaseDate;    // 58-61 MMDD
  std::string destAmount;      // 62-73
  std::string destCcy;         // 74-76
  std::string srcAmount;       // 77-88
  std::string srcCcy;          // 89-91
  std::string merchantName;    // 92-116
  std::string merchantCity;    // 117-129
  std::string merchantCountry; // 130-132
  std::string mcc;             // 133-136
  std::string merchantZip;     // 137-141
  std::string merchantState;   // 142-144
  std::string requestedPmtSvc; // 145
  std::string numPmtForms;     // 146
  std::string usageCode;       // 147
  std::string reasonCode;      // 148-149
  std::string settlementFlag;  // 150
  std::string authCharInd;     // 151
  std::string authCode;        // 152-157
  std::string posTermCap;      // 158
  std::string reserved159;     // 159
  std::string cardholderIdMeth;// 160
  std::string collectionOnly;  // 161
  std::string posEntryMode;    // 162-163
  std::string centralProcDate; // 164-167 YDDD
  std::string reimbAttr;       // 168
};

struct Tc05Tcr7 {
  std::string tc;                 // 1-2
  std::string tcq;                // 3
  std::string tcr;                // 4
  std::string transactionType;    // 5-6
  std::string cardSeq;            // 7-9
  std::string terminalTxnDate;    // 10-15
  std::string terminalCapProfile; // 16-21 (DX)
  std::string terminalCountry;    // 22-24
  std::string terminalSerial;     // 25-32
  std::string unpredictableNum;   // 33-40 (DX)
  std::string atc;                // 41-44 (DX)
  std::string aip;                // 45-48 (DX)
  std::string cryptogram;         // 49-64 (DX)
  std::string iad_b2;             // 65-66 (DX)
  std::string iad_b3;             // 67-68 (DX)
  std::string tvr;                // 69-78 (DX)
  std::string iad_b4_7;           // 79-86 (DX)
  std::string cryptogramAmount;   // 87-98 (UN)
  std::string iad_b8;             // 99-100 (DX)
  std::string iad_b9_16;          // 101-116 (DX)
  std::string iad_b1;             // 117-118 (DX)
  std::string iad_b17;            // 119-120 (DX)
  std::string iad_b18_32;         // 121-150 (DX)
  std::string formFactorInd;      // 151-158 (DX)
  std::string issuerScript1Res;   // 159-168 (DX)
};

static Tc05Tcr0 parse_tc05_tcr0(const std::string& r) {
  // Layout per TC05 TCR0 table. :contentReference[oaicite:7]{index=7}
  Tc05Tcr0 x;
  x.tc = field(r, 1, 2);
  x.tcq = field(r, 3, 1);
  x.tcr = field(r, 4, 1);
  x.accountNumber = field(r, 5, 16);
  x.acctExt = field(r, 21, 3);
  x.floorLimitInd = field(r, 24, 1);
  x.crbExceptionInd = field(r, 25, 1);
  x.reserved26 = field(r, 26, 1);
  x.arn = field(r, 27, 23);
  x.acquirerBizId = field(r, 50, 8);
  x.purchaseDate = field(r, 58, 4);
  x.destAmount = field(r, 62, 12);
  x.destCcy = field(r, 74, 3);
  x.srcAmount = field(r, 77, 12);
  x.srcCcy = field(r, 89, 3);
  x.merchantName = field(r, 92, 25);
  x.merchantCity = field(r, 117, 13);
  x.merchantCountry = field(r, 130, 3);
  x.mcc = field(r, 133, 4);
  x.merchantZip = field(r, 137, 5);
  x.merchantState = field(r, 142, 3);
  x.requestedPmtSvc = field(r, 145, 1);
  x.numPmtForms = field(r, 146, 1);
  x.usageCode = field(r, 147, 1);
  x.reasonCode = field(r, 148, 2);
  x.settlementFlag = field(r, 150, 1);
  x.authCharInd = field(r, 151, 1);
  x.authCode = field(r, 152, 6);
  x.posTermCap = field(r, 158, 1);
  x.reserved159 = field(r, 159, 1);
  x.cardholderIdMeth = field(r, 160, 1);
  x.collectionOnly = field(r, 161, 1);
  x.posEntryMode = field(r, 162, 2);
  x.centralProcDate = field(r, 164, 4);
  x.reimbAttr = field(r, 168, 1);
  return x;
}

static Tc05Tcr7 parse_tc05_tcr7(const std::string& r) {
  // Layout per TC05 TCR7 Chip Card Transaction Data. :contentReference[oaicite:8]{index=8} :contentReference[oaicite:9]{index=9}
  Tc05Tcr7 x;
  x.tc = field(r, 1, 2);
  x.tcq = field(r, 3, 1);
  x.tcr = field(r, 4, 1);
  x.transactionType = field(r, 5, 2);
  x.cardSeq = field(r, 7, 3);
  x.terminalTxnDate = field(r, 10, 6);
  x.terminalCapProfile = field(r, 16, 6);
  x.terminalCountry = field(r, 22, 3);
  x.terminalSerial = field(r, 25, 8);
  x.unpredictableNum = field(r, 33, 8);
  x.atc = field(r, 41, 4);
  x.aip = field(r, 45, 4);
  x.cryptogram = field(r, 49, 16);
  x.iad_b2 = field(r, 65, 2);
  x.iad_b3 = field(r, 67, 2);
  x.tvr = field(r, 69, 10);
  x.iad_b4_7 = field(r, 79, 8);
  x.cryptogramAmount = field(r, 87, 12);
  x.iad_b8 = field(r, 99, 2);
  x.iad_b9_16 = field(r, 101, 16);
  x.iad_b1 = field(r, 117, 2);
  x.iad_b17 = field(r, 119, 2);
  x.iad_b18_32 = field(r, 121, 30);
  x.formFactorInd = field(r, 151, 8);
  x.issuerScript1Res = field(r, 159, 10);
  return x;
}

static void print_json(const Tc05Tcr0& x, long long recordIndex) {
  auto J = [](const std::string& v) { return "\"" + json_escape(rtrim_copy(v)) + "\""; };

  std::cout << "{"
    << "\"record_index\":" << recordIndex << ","
    << "\"tc\":" << J(x.tc) << ","
    << "\"tcr\":" << J(x.tcr) << ","
    << "\"tcq\":" << J(x.tcq) << ","
    << "\"account_number\":" << J(x.accountNumber) << ","
    << "\"account_number_extension\":" << J(x.acctExt) << ","
    << "\"arn\":" << J(x.arn) << ","
    << "\"acquirer_business_id\":" << J(x.acquirerBizId) << ","
    << "\"purchase_date_mmdd\":" << J(x.purchaseDate) << ","
    << "\"dest_amount\":" << J(x.destAmount) << ","
    << "\"dest_currency\":" << J(x.destCcy) << ","
    << "\"src_amount\":" << J(x.srcAmount) << ","
    << "\"src_currency\":" << J(x.srcCcy) << ","
    << "\"merchant_name\":" << J(x.merchantName) << ","
    << "\"merchant_city\":" << J(x.merchantCity) << ","
    << "\"merchant_country\":" << J(x.merchantCountry) << ","
    << "\"mcc\":" << J(x.mcc) << ","
    << "\"merchant_zip\":" << J(x.merchantZip) << ","
    << "\"merchant_state\":" << J(x.merchantState) << ","
    << "\"requested_payment_service\":" << J(x.requestedPmtSvc) << ","
    << "\"number_of_payment_forms\":" << J(x.numPmtForms) << ","
    << "\"usage_code\":" << J(x.usageCode) << ","
    << "\"reason_code\":" << J(x.reasonCode) << ","
    << "\"settlement_flag\":" << J(x.settlementFlag) << ","
    << "\"authorization_characteristics_indicator\":" << J(x.authCharInd) << ","
    << "\"authorization_code\":" << J(x.authCode) << ","
    << "\"pos_terminal_capability\":" << J(x.posTermCap) << ","
    << "\"cardholder_id_method\":" << J(x.cardholderIdMeth) << ","
    << "\"collection_only_flag\":" << J(x.collectionOnly) << ","
    << "\"pos_entry_mode\":" << J(x.posEntryMode) << ","
    << "\"central_processing_date_yddd\":" << J(x.centralProcDate) << ","
    << "\"reimbursement_attribute\":" << J(x.reimbAttr)
    << "}\n";
}

static void print_json(const Tc05Tcr7& x, long long recordIndex) {
  auto J = [](const std::string& v) { return "\"" + json_escape(rtrim_copy(v)) + "\""; };

  std::cout << "{"
    << "\"record_index\":" << recordIndex << ","
    << "\"tc\":" << J(x.tc) << ","
    << "\"tcr\":" << J(x.tcr) << ","
    << "\"tcq\":" << J(x.tcq) << ","
    << "\"transaction_type\":" << J(x.transactionType) << ","
    << "\"card_sequence_number\":" << J(x.cardSeq) << ","
    << "\"terminal_transaction_date\":" << J(x.terminalTxnDate) << ","
    << "\"terminal_capability_profile_dx\":" << J(x.terminalCapProfile) << ","
    << "\"terminal_country_code\":" << J(x.terminalCountry) << ","
    << "\"terminal_serial_number\":" << J(x.terminalSerial) << ","
    << "\"unpredictable_number_dx\":" << J(x.unpredictableNum) << ","
    << "\"application_transaction_counter_dx\":" << J(x.atc) << ","
    << "\"application_interchange_profile_dx\":" << J(x.aip) << ","
    << "\"cryptogram_dx\":" << J(x.cryptogram) << ","
    << "\"iad_byte2_dx\":" << J(x.iad_b2) << ","
    << "\"iad_byte3_dx\":" << J(x.iad_b3) << ","
    << "\"terminal_verification_results_dx\":" << J(x.tvr) << ","
    << "\"iad_bytes4_7_dx\":" << J(x.iad_b4_7) << ","
    << "\"cryptogram_amount\":" << J(x.cryptogramAmount) << ","
    << "\"iad_byte8_dx\":" << J(x.iad_b8) << ","
    << "\"iad_bytes9_16_dx\":" << J(x.iad_b9_16) << ","
    << "\"iad_byte1_dx\":" << J(x.iad_b1) << ","
    << "\"iad_byte17_dx\":" << J(x.iad_b17) << ","
    << "\"iad_bytes18_32_dx\":" << J(x.iad_b18_32) << ","
    << "\"form_factor_indicator_dx\":" << J(x.formFactorInd) << ","
    << "\"issuer_script1_results_dx\":" << J(x.issuerScript1Res)
    << "}\n";
}

// Reads either:
// - fixed-block (size multiple of recLen), OR
// - line-delimited records (each line should be recLen bytes; CRLF tolerated)
static std::vector<std::string> read_records(const std::string& path, size_t recLen) {
  std::ifstream in(path, std::ios::binary);
  if (!in) throw std::runtime_error("Cannot open input file: " + path);

  // Load full file
  std::string data((std::istreambuf_iterator<char>(in)), std::istreambuf_iterator<char>());

  // Heuristic: if file contains '\n', assume line-delimited
  if (data.find('\n') != std::string::npos) {
    std::vector<std::string> recs;
    std::istringstream iss(data);
    std::string line;
    while (std::getline(iss, line)) {
      if (!line.empty() && line.back() == '\r') line.pop_back();
      if (line.empty()) continue;
      // If line is longer (some exports include trailing spaces trimmed), pad right with spaces
      if (line.size() < recLen) line.append(recLen - line.size(), ' ');
      if (line.size() >= recLen) recs.push_back(line.substr(0, recLen));
    }
    return recs;
  }

  // Else: fixed-block
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

int main(int argc, char** argv) {
  try {
    if (argc < 2) {
      std::cerr << "Usage: " << argv[0] << " <input_file> [ctf|itf]\n";
      return 2;
    }

    std::string path = argv[1];
    FileFormat fmt = FileFormat::CTF;
    if (argc >= 3) {
      std::string f = argv[2];
      std::transform(f.begin(), f.end(), f.begin(), [](unsigned char c) { return std::tolower(c); });
      if (f == "ctf") fmt = FileFormat::CTF;
      else if (f == "itf") fmt = FileFormat::ITF;
      else throw std::runtime_error("Unknown format: " + f + " (expected ctf or itf)");
    }

    const size_t rawLen = (fmt == FileFormat::CTF) ? 168 : 170;
    auto rawRecords = read_records(path, rawLen);

    long long recIndex = 0;
    for (const auto& raw : rawRecords) {
      recIndex++;

      auto normOpt = normalize_record(raw, fmt);
      if (!normOpt) continue;
      const std::string& r = *normOpt;

      std::string tc = field(r, 1, 2);
      std::string tcr = field(r, 4, 1);

      if (tc == "05" && tcr == "0") {
        if (tcr == "0")
        {
          std::ostringstream oss;
          oss << "********************************************\n";
        }
        auto t = parse_tc05_tcr0(r);
        print_json(t, recIndex);
      }
      else if (tc == "05" && tcr == "7") {
        auto t = parse_tc05_tcr7(r);
        print_json(t, recIndex);
      }
    }

    return 0;
  }
  catch (const std::exception& ex) {
    std::cerr << "ERROR: " << ex.what() << "\n";
    return 1;
  }
}
