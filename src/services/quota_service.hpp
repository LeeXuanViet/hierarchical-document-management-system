#pragma once

#include "user_service.hpp"

#include <string>
#include <stdexcept>

namespace dms {

// Service kiểm đếm dung lượng đã dùng của người dùng.
class QuotaService {
public:
    explicit QuotaService(UserService& users);

    // Trả về dung lượng đã dùng (bytes).
    long long used(const std::string& userId) const;

    // Trả về giới hạn quota (bytes), 0 = không giới hạn.
    long long limit(const std::string& userId) const;

    // Trả về phần trăm đã dùng (0-100). Nếu limit=0 trả về 0.
    double usagePercent(const std::string& userId) const;

    // Kiểm tra user có vượt quota khi thêm thêm delta bytes hay không.
    bool wouldExceed(const std::string& userId, long long delta) const;

    // Định dạng bytes thành chuỗi dễ đọc (KB/MB).
    static std::string formatBytes(long long bytes);

private:
    UserService& users_;
};

} // namespace dms
