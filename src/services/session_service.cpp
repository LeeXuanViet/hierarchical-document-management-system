#include "session_service.hpp"

namespace dms {

SessionService::SessionService(IUserRepository& userRepo) : userRepo_(userRepo) {}

std::optional<std::string> SessionService::login(const std::string& username, const std::string& password) {
    auto userOpt = userRepo_.findByUsername(username);
    if (!userOpt) return std::nullopt;
    if (!verifyPassword(password, userOpt->passwordHash)) return std::nullopt;
    current_ = userOpt->id;
    return current_;
}

void SessionService::logout() {
    current_.reset();
}

std::optional<std::string> SessionService::currentUserId() const {
    return current_;
}

bool SessionService::isLoggedIn() const {
    return current_.has_value();
}

} // namespace dms
