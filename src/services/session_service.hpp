#pragma once

#include "../repositories/interfaces/i_user_repository.hpp"
#include "../utils/hash.hpp"

#include <string>
#include <optional>

namespace dms {

// Service quản lý phiên đăng nhập (đơn giản, in-memory).
class SessionService {
public:
    explicit SessionService(IUserRepository& userRepo);

    // Đăng nhập. Trả về userId nếu thành công, nullopt nếu sai.
    std::optional<std::string> login(const std::string& username, const std::string& password);

    // Đăng xuất user hiện tại.
    void logout();

    // Lấy userId đang đăng nhập (nullopt nếu chưa).
    std::optional<std::string> currentUserId() const;

    // Kiểm tra đã đăng nhập chưa.
    bool isLoggedIn() const;

private:
    IUserRepository& userRepo_;
    std::optional<std::string> current_;
};

} // namespace dms
