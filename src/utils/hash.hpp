#pragma once

#include <string>

namespace dms {

// Hash đơn giản (không an toàn mật mã, chỉ phục vụ demo).
// Dùng để không lưu plaintext password trong metadata.
std::string simpleHash(const std::string& input);

// Kiểm tra password có khớp với hash đã lưu hay không.
bool verifyPassword(const std::string& password, const std::string& hash);

} // namespace dms
