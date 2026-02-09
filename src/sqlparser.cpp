// g++ -std=c++17 -O2 -Wall -Wextra -pedantic sql_table_parser.cpp -o sql_table_parser

#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <algorithm>
#include <cctype>
#include <sstream>

// ------------------------------------------------------------
// Small utility helpers
// ------------------------------------------------------------

// Trim from left (in-place)
static inline void ltrim(std::string &s) {
    s.erase(s.begin(), std::find_if(s.begin(), s.end(),
        [](unsigned char ch) { return !std::isspace(ch); }));
}

// Trim from right (in-place)
static inline void rtrim(std::string &s) {
    s.erase(std::find_if(s.rbegin(), s.rend(),
        [](unsigned char ch) { return !std::isspace(ch); }).base(),
        s.end());
}

// Trim both ends (returns copy)
static inline std::string trim(std::string s) {
    ltrim(s);
    rtrim(s);
    return s;
}

// Lowercase copy
static inline std::string toLowerCopy(const std::string &s) {
    std::string out = s;
    std::transform(out.begin(), out.end(), out.begin(),
                   [](unsigned char c) { return std::tolower(c); });
    return out;
}

// Case-insensitive "starts with"
static bool startsWithCI(const std::string &s, const std::string &prefix) {
    if (s.size() < prefix.size()) return false;
    return toLowerCopy(s.substr(0, prefix.size())) ==
           toLowerCopy(prefix);
}

// Case-insensitive "contains"
static bool containsCI(const std::string &s, const std::string &needle) {
    std::string ls = toLowerCopy(s);
    std::string ln = toLowerCopy(needle);
    return ls.find(ln) != std::string::npos;
}

// Simple whitespace split
static std::vector<std::string> splitWhitespace(const std::string &s) {
    std::vector<std::string> tokens;
    std::istringstream iss(s);
    std::string tok;
    while (iss >> tok) {
        tokens.push_back(tok);
    }
    return tokens;
}

// ------------------------------------------------------------
// Parsing helpers
// ------------------------------------------------------------

// Extract the table name after "alter table" or "create table".
static std::string extractTableName(const std::string &line,
                                    const std::string &keyword) {
    // keyword will be "alter table" or "create table"
    std::string lower = toLowerCopy(line);
    std::string lowerKey = toLowerCopy(keyword);
    std::size_t pos = lower.find(lowerKey);
    if (pos == std::string::npos) return "";

    // Move to just after the keyword
    pos += lowerKey.size();

    // Skip spaces/tabs
    while (pos < line.size() &&
           std::isspace(static_cast<unsigned char>(line[pos]))) {
        ++pos;
    }

    // Read until whitespace or "(" or ";" etc.
    std::string name;
    while (pos < line.size()) {
        char c = line[pos];
        if (std::isspace(static_cast<unsigned char>(c)) || c == '(' || c == ';')
            break;
        name.push_back(c);
        ++pos;
    }
    return trim(name);
}

// Parse a single column definition line into (name, type, size).
// Example input lines (already trimmed & cleaned):
//   "rspcode_ext char(3)"
//   "addsvc varchar2(255) default '-' not null"
static bool parseColumnLine(const std::string &line,
                            std::string &colName,
                            std::string &typeName,
                            std::string &sizeStr) {
    std::string work = trim(line);
    if (work.empty()) return false;

    // Remove a leading comma, if someone writes ", col char(3)" style.
    if (!work.empty() && work[0] == ',') {
        work = trim(work.substr(1));
    }
    if (work.empty()) return false;

    // Tokenize by whitespace
    auto tokens = splitWhitespace(work);
    if (tokens.size() < 2) {
        // Not enough to contain "name type"
        return false;
    }

    // Skip lines that clearly are constraints, not columns.
    {
        std::string firstLower = toLowerCopy(tokens[0]);
        if (firstLower == "constraint" ||
            firstLower == "primary" ||
            firstLower == "foreign" ||
            firstLower == "unique"  ||
            firstLower == "check")
        {
            return false;
        }
    }

    colName = tokens[0];
    std::string typeToken = tokens[1];

    typeName.clear();
    sizeStr.clear();

    // Case 1: type and size together: varchar2(255)
    auto parenPos = typeToken.find('(');
    if (parenPos != std::string::npos) {
        typeName = typeToken.substr(0, parenPos);

        auto closePos = typeToken.find(')', parenPos + 1);
        if (closePos == std::string::npos) {
            // Fallback: size until end of token
            sizeStr = typeToken.substr(parenPos + 1);
        } else {
            sizeStr = typeToken.substr(parenPos + 1,
                                       closePos - parenPos - 1);
        }
    } else {
        // Case 2: type without size in same token (e.g. "date")
        typeName = typeToken;

        // Try to see if size is in the rest of the line:
        // e.g. "varchar2 (255) default ...", so tokens[1] == "varchar2", tokens[2] == "(255)".
        std::string rest;
        if (tokens.size() > 2) {
            // Rebuild "rest" from tokens[2..]
            for (size_t i = 2; i < tokens.size(); ++i) {
                if (!rest.empty()) rest.push_back(' ');
                rest += tokens[i];
            }
            auto p = rest.find('(');
            if (p != std::string::npos) {
                auto q = rest.find(')', p + 1);
                if (q == std::string::npos) {
                    sizeStr = rest.substr(p + 1);
                } else {
                    sizeStr = rest.substr(p + 1, q - p - 1);
                }
            }
        }
    }

    // Clean type name & size
    typeName = trim(typeName);
    sizeStr  = trim(sizeStr);

    if (sizeStr.empty()) {
        sizeStr = "N/A";
    }

    return !colName.empty() && !typeName.empty();
}

// ------------------------------------------------------------
// Core logic
// ------------------------------------------------------------

static void processFile(const std::string &path) {
    std::ifstream in(path);
    if (!in) {
        std::cerr << "Error: cannot open file '" << path << "'.\n";
        return;
    }

    std::cout << "============================================================\n";
    std::cout << "File: " << path << "\n";
    std::cout << "============================================================\n\n";

    std::string line;
    bool inTableCommand = false;  // true after "alter table" or "create table"
    bool inColumnsBlock = false;  // true when we're between "(" and ");"
    std::string currentTableName;
    bool headerPrintedForCurrentTable = false;

    // We keep a small buffer of the last relevant line to help detect "("
    while (std::getline(in, line)) {
        std::string rawLine = line;
        std::string trimmedLine = trim(rawLine);
        std::string lowerLine = toLowerCopy(trimmedLine);

        // Ignore full-line comments
        if (startsWithCI(trimmedLine, "--")) {
            continue;
        }

        // Detect start of a table statement (ALTER TABLE or CREATE TABLE)
        if (!inTableCommand) {
            if (containsCI(lowerLine, "alter table")) {
                currentTableName = extractTableName(rawLine, "alter table");
                if (!currentTableName.empty()) {
                    inTableCommand = true;
                    inColumnsBlock = false;
                    headerPrintedForCurrentTable = false;
                }
            } else if (containsCI(lowerLine, "create table")) {
                currentTableName = extractTableName(rawLine, "create table");
                if (!currentTableName.empty()) {
                    inTableCommand = true;
                    inColumnsBlock = false;
                    headerPrintedForCurrentTable = false;
                }
            }

            // If we just entered a table command and also have "(" on same line,
            // we might already be in the columns block.
            if (inTableCommand && lowerLine.find('(') != std::string::npos) {
                inColumnsBlock = true;
                // The current line may already contain the first column after "("
                // so we fall through and let the column parser see it.
            } else {
                // Not in a table statement yet, nothing more to do with this line
                if (!inTableCommand) {
                    continue;
                }
            }
        } else {
            // We are already inside a table command (ALTER or CREATE).
            // If we haven't reached the "(" that opens the column list yet,
            // check for it.
            if (!inColumnsBlock && lowerLine.find('(') != std::string::npos) {
                inColumnsBlock = true;
                // We don't return; this line might also contain a column.
            }
        }

        // If we're inside the columns block, parse each column line.
        if (inTableCommand && inColumnsBlock) {
            // Detect end of columns block: looking for ");"
            bool endsColumns = (lowerLine.find(");") != std::string::npos);

            // Remove the leading "(" and trailing characters like "," or ");"
            std::string colLine = trimmedLine;

            // Strip leading "(" if present
            if (!colLine.empty() && colLine[0] == '(') {
                colLine = trim(colLine.substr(1));
            }

            // Strip trailing ";" or ")" or "," characters
            while (!colLine.empty()) {
                char c = colLine.back();
                if (c == ';' || c == ')' || c == ',') {
                    colLine.pop_back();
                } else if (std::isspace(static_cast<unsigned char>(c))) {
                    colLine.pop_back();
                } else {
                    break;
                }
            }
            colLine = trim(colLine);

            if (!colLine.empty()) {
                std::string colName, typeName, sizeStr;
                if (parseColumnLine(colLine, colName, typeName, sizeStr)) {
                    if (!headerPrintedForCurrentTable) {
                        std::cout << "Table: " << currentTableName << "\n";
                        std::cout << "--------------------------------------------\n";
                        std::cout << "  Column Name           Type         Size\n";
                        std::cout << "--------------------------------------------\n";
                        headerPrintedForCurrentTable = true;
                    }
                    std::cout << "  " << colName;

                    // A bit of manual padding to keep it readable
                    if (colName.size() < 20) {
                        std::cout << std::string(20 - colName.size(), ' ');
                    } else {
                        std::cout << ' ';
                    }

                    std::cout << typeName;
                    if (typeName.size() < 12) {
                        std::cout << std::string(12 - typeName.size(), ' ');
                    } else {
                        std::cout << ' ';
                    }

                    std::cout << sizeStr << "\n";
                }
            }

            if (endsColumns) {
                // End of this table's column list.
                inColumnsBlock = false;
                inTableCommand = false;

                if (headerPrintedForCurrentTable) {
                    std::cout << "--------------------------------------------\n\n";
                }

                currentTableName.clear();
                headerPrintedForCurrentTable = false;
            }
        }
    }

    // Just a visual separator between files
    std::cout << "\n";
}

// ------------------------------------------------------------
// main
// ------------------------------------------------------------

int main(int argc, char *argv[]) {
    if (argc < 2) {
        std::cerr << "Usage: " << argv[0]
                  << " <sql_file1> [sql_file2 ...]\n\n";
        std::cerr << "This tool scans ALTER TABLE / CREATE TABLE statements\n"
                     "and prints the table name, column names, types, and sizes.\n";
        return 1;
    }

    for (int i = 1; i < argc; ++i) {
        processFile(argv[i]);
    }

    return 0;
}
