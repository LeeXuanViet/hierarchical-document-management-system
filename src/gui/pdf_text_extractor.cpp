#include "pdf_text_extractor.hpp"

#include <algorithm>
#include <cctype>
#include <cstring>
#include <sstream>

namespace dms {

namespace {

// Case-insensitive find of `needle` within `hay` starting at `pos`.
size_t findCI(const std::string& hay, const std::string& needle, size_t pos = 0)
{
    if (needle.empty()) return pos;
    auto it = std::search(
        hay.begin() + static_cast<std::ptrdiff_t>(pos), hay.end(),
        needle.begin(), needle.end(),
        [](char a, char b) {
            return std::toupper(static_cast<unsigned char>(a)) ==
                   std::toupper(static_cast<unsigned char>(b));
        });
    if (it == hay.end()) return std::string::npos;
    return static_cast<size_t>(it - hay.begin());
}

// Extract the value portion of "/Key (value)" or "/Key <hex>" from a PDF dict.
std::string extractPdfString(const std::string& data, const std::string& key)
{
    std::string token = "/" + key;
    size_t k = findCI(data, token);
    while (k != std::string::npos) {
        size_t i = k + token.size();
        // skip whitespace
        while (i < data.size() && std::isspace(static_cast<unsigned char>(data[i]))) ++i;
        if (i < data.size() && data[i] == '(') {
            // literal string, possibly with nested parens and escapes
            ++i;
            std::string out;
            int depth = 1;
            while (i < data.size() && depth > 0) {
                char c = data[i];
                if (c == '\\' && i + 1 < data.size()) {
                    char n = data[i + 1];
                    switch (n) {
                        case 'n': out.push_back('\n'); i += 2; break;
                        case 'r': out.push_back('\r'); i += 2; break;
                        case 't': out.push_back('\t'); i += 2; break;
                        case 'b': out.push_back('\b'); i += 2; break;
                        case 'f': out.push_back('\f'); i += 2; break;
                        case '\\': out.push_back('\\'); i += 2; break;
                        case '(': out.push_back('('); i += 2; break;
                        case ')': out.push_back(')'); i += 2; break;
                        default:
                            if (n >= '0' && n <= '7') {
                                // octal escape up to 3 digits
                                std::string oct;
                                oct.push_back(n);
                                i += 2;
                                for (int d = 0; d < 2 && i < data.size() &&
                                     data[i] >= '0' && data[i] <= '7'; ++d) {
                                    oct.push_back(data[i++]);
                                }
                                int code = 0;
                                for (char oc : oct) code = code * 8 + (oc - '0');
                                out.push_back(static_cast<char>(code & 0xFF));
                            } else {
                                out.push_back(n);
                                i += 2;
                            }
                            break;
                    }
                } else if (c == '(') {
                    ++depth;
                    out.push_back(c);
                    ++i;
                } else if (c == ')') {
                    --depth;
                    if (depth == 0) { ++i; break; }
                    out.push_back(c);
                    ++i;
                } else {
                    out.push_back(c);
                    ++i;
                }
            }
            return out;
        } else if (i < data.size() && data[i] == '<') {
            // hex string
            ++i;
            std::string hex;
            while (i < data.size() && data[i] != '>') {
                if (!std::isspace(static_cast<unsigned char>(data[i])))
                    hex.push_back(data[i]);
                ++i;
            }
            std::string out;
            for (size_t j = 0; j + 1 < hex.size(); j += 2) {
                int hi = std::isdigit(static_cast<unsigned char>(hex[j]))
                             ? (hex[j] - '0')
                             : (std::toupper(static_cast<unsigned char>(hex[j])) - 'A' + 10);
                int lo = std::isdigit(static_cast<unsigned char>(hex[j + 1]))
                             ? (hex[j + 1] - '0')
                             : (std::toupper(static_cast<unsigned char>(hex[j + 1])) - 'A' + 10);
                out.push_back(static_cast<char>((hi << 4) | lo));
            }
            return out;
        }
        // not a string here; look for next occurrence
        k = findCI(data, token, k + 1);
    }
    return "";
}

// Count occurrences of "/Type /Page" (not /Pages) to estimate page count.
int countPages(const std::string& data)
{
    int count = 0;
    size_t pos = 0;
    while ((pos = findCI(data, "/Type", pos)) != std::string::npos) {
        size_t i = pos + 5;
        while (i < data.size() && std::isspace(static_cast<unsigned char>(data[i]))) ++i;
        // match /Page not /Pages
        if (i + 5 <= data.size() &&
            std::strncmp(data.c_str() + i, "/Page", 5) == 0 &&
            (i + 5 == data.size() || data[i + 5] != 's')) {
            ++count;
        }
        pos += 5;
    }
    return count;
}

// Decode a PDF text-showing operand: a string literal "(...)" or array "[...]".
// Appends decoded text to `out`. Returns true if something was decoded.
bool decodeTextOperand(const std::string& s, size_t& i, std::string& out)
{
    if (i >= s.size()) return false;
    char c = s[i];
    if (c == '(') {
        ++i;
        int depth = 1;
        while (i < s.size() && depth > 0) {
            char ch = s[i];
            if (ch == '\\' && i + 1 < s.size()) {
                char n = s[i + 1];
                switch (n) {
                    case 'n': out.push_back('\n'); i += 2; break;
                    case 'r': out.push_back('\r'); i += 2; break;
                    case 't': out.push_back('\t'); i += 2; break;
                    case 'b': out.push_back('\b'); i += 2; break;
                    case 'f': out.push_back('\f'); i += 2; break;
                    case '\\': out.push_back('\\'); i += 2; break;
                    case '(': out.push_back('('); i += 2; break;
                    case ')': out.push_back(')'); i += 2; break;
                    default:
                        if (n >= '0' && n <= '7') {
                            std::string oct;
                            oct.push_back(n);
                            i += 2;
                            for (int d = 0; d < 2 && i < s.size() &&
                                 s[i] >= '0' && s[i] <= '7'; ++d) {
                                oct.push_back(s[i++]);
                            }
                            int code = 0;
                            for (char oc : oct) code = code * 8 + (oc - '0');
                            out.push_back(static_cast<char>(code & 0xFF));
                        } else {
                            out.push_back(n);
                            i += 2;
                        }
                        break;
                }
            } else if (ch == '(') {
                ++depth;
                out.push_back(ch);
                ++i;
            } else if (ch == ')') {
                --depth;
                if (depth == 0) { ++i; break; }
                out.push_back(ch);
                ++i;
            } else {
                out.push_back(ch);
                ++i;
            }
        }
        return true;
    }
    if (c == '<') {
        // hex string
        ++i;
        std::string hex;
        while (i < s.size() && s[i] != '>') {
            if (!std::isspace(static_cast<unsigned char>(s[i])))
                hex.push_back(s[i]);
            ++i;
        }
        if (i < s.size()) ++i; // skip '>'
        for (size_t j = 0; j + 1 < hex.size(); j += 2) {
            int hi = std::isdigit(static_cast<unsigned char>(hex[j]))
                         ? (hex[j] - '0')
                         : (std::toupper(static_cast<unsigned char>(hex[j])) - 'A' + 10);
            int lo = std::isdigit(static_cast<unsigned char>(hex[j + 1]))
                         ? (hex[j + 1] - '0')
                         : (std::toupper(static_cast<unsigned char>(hex[j + 1])) - 'A' + 10);
            out.push_back(static_cast<char>((hi << 4) | lo));
        }
        return true;
    }
    return false;
}

// Scan a content stream region for text operators and extract text.
void extractTextFromStream(const std::string& stream, std::string& out, size_t maxChars)
{
    size_t i = 0;
    size_t n = stream.size();
    while (i < n && out.size() < maxChars) {
        char c = stream[i];
        if (c == '(' || c == '<') {
            size_t before = out.size();
            std::string token;
            size_t save = i;
            if (decodeTextOperand(stream, i, token)) {
                // peek ahead for an operator: Tj, TJ, ', "
                size_t j = i;
                while (j < n && std::isspace(static_cast<unsigned char>(stream[j]))) ++j;
                if (j + 1 < n) {
                    std::string op;
                    while (j < n && std::isalpha(static_cast<unsigned char>(stream[j]))) {
                        op.push_back(stream[j++]);
                    }
                    if (op == "Tj" || op == "TJ" || op == "'" || op == "\"") {
                        out.append(token);
                        if (op == "TJ" || op == "Tj") {
                            // TJ arrays may contain multiple strings; spacing handled below
                        }
                        if (op == "'" || op == "\"") out.push_back('\n');
                        (void)before;
                    } else {
                        // not a text op; discard the decoded token
                        (void)save;
                    }
                }
            }
        } else if (c == '[') {
            // TJ array: [ (s1) -250 (s2) ... ] TJ
            size_t arrStart = i;
            ++i;
            std::string arrText;
            while (i < n && stream[i] != ']' && arrText.size() < maxChars) {
                if (stream[i] == '(' || stream[i] == '<') {
                    decodeTextOperand(stream, i, arrText);
                } else {
                    ++i;
                }
            }
            if (i < n && stream[i] == ']') ++i;
            // check following operator
            size_t j = i;
            while (j < n && std::isspace(static_cast<unsigned char>(stream[j]))) ++j;
            std::string op;
            while (j < n && std::isalpha(static_cast<unsigned char>(stream[j]))) {
                op.push_back(stream[j++]);
            }
            if (op == "TJ" || op == "Tj") {
                out.append(arrText);
            }
            (void)arrStart;
        } else {
            ++i;
        }
    }
}

// Find all "stream" ... "endstream" regions and extract text from each.
void extractAllStreamText(const std::string& data, std::string& out, size_t maxChars)
{
    size_t pos = 0;
    while (out.size() < maxChars &&
           (pos = findCI(data, "stream", pos)) != std::string::npos) {
        // ensure this is a stream keyword, not part of a longer token
        size_t endkw = pos + 6;
        // skip the EOL after "stream"
        size_t s = endkw;
        if (s < data.size() && data[s] == '\r') ++s;
        if (s < data.size() && data[s] == '\n') ++s;
        else if (s < data.size() && data[s] == '\n') ++s;
        size_t endpos = findCI(data, "endstream", s);
        if (endpos == std::string::npos) break;
        std::string chunk = data.substr(s, endpos - s);
        extractTextFromStream(chunk, out, maxChars);
        pos = endpos + 9;
    }
}

} // namespace

PdfInfo extractPdfInfo(const std::string& data, size_t maxTextChars)
{
    PdfInfo info;
    if (data.size() < 5 || data.compare(0, 5, "%PDF-") != 0) {
        return info;
    }

    // Encryption flag
    if (findCI(data, "/Encrypt") != std::string::npos) {
        info.encrypted = true;
    }

    // Metadata from document Info dictionary
    info.title = extractPdfString(data, "Title");
    info.author = extractPdfString(data, "Author");
    info.subject = extractPdfString(data, "Subject");
    info.producer = extractPdfString(data, "Producer");
    info.creator = extractPdfString(data, "Creator");
    info.creationDate = extractPdfString(data, "CreationDate");

    // Page count
    info.pageCount = countPages(data);
    if (info.pageCount == 0) {
        // fallback: count /Page tokens loosely
        size_t pos = 0;
        int loose = 0;
        while ((pos = findCI(data, "/Page", pos)) != std::string::npos) {
            if (pos + 6 > data.size() || data[pos + 5] != 's') ++loose;
            pos += 5;
        }
        info.pageCount = loose;
    }

    // Text extraction (only meaningful for uncompressed streams)
    if (!info.encrypted) {
        extractAllStreamText(data, info.text, maxTextChars);
    }

    return info;
}

} // namespace dms
