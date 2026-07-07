#pragma once

#include <string>
#include <vector>
#include <cctype>

namespace dms {

// Tách chuỗi thành token (lowercase, chỉ giữ chữ cái/số).
std::vector<std::string> tokenize(const std::string& text);

// Chuẩn hóa: lowercase + trim.
std::string toLower(const std::string& s);
std::string trim(const std::string& s);

// Kiểm tra chuỗi con không phân biệt hoa thường.
bool containsIgnoreCase(const std::string& haystack, const std::string& needle);

} // namespace dms
