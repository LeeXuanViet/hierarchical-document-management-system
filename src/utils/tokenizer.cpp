#include "tokenizer.hpp"

#include <algorithm>

namespace dms {

std::string toLower(const std::string& s) {
    std::string out = s;
    std::transform(out.begin(), out.end(), out.begin(),
                   [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
    return out;
}

std::string trim(const std::string& s) {
    auto begin = s.begin();
    while (begin != s.end() && std::isspace(static_cast<unsigned char>(*begin))) ++begin;
    auto end = s.end();
    while (end != begin && std::isspace(static_cast<unsigned char>(*(end - 1)))) --end;
    return std::string(begin, end);
}

std::vector<std::string> tokenize(const std::string& text) {
    std::vector<std::string> tokens;
    std::string current;
    for (char c : text) {
        unsigned char uc = static_cast<unsigned char>(c);
        if (std::isalnum(uc)) {
            current.push_back(static_cast<char>(std::tolower(uc)));
        } else if (!current.empty()) {
            tokens.push_back(current);
            current.clear();
        }
    }
    if (!current.empty()) tokens.push_back(current);
    return tokens;
}

bool containsIgnoreCase(const std::string& haystack, const std::string& needle) {
    if (needle.empty()) return true;
    auto h = toLower(haystack);
    auto n = toLower(needle);
    return h.find(n) != std::string::npos;
}

} // namespace dms
