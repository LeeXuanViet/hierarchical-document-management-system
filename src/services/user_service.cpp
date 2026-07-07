#include "user_service.hpp"

namespace dms {

UserService::UserService(IUserRepository& userRepo,
                         IOrgUnitRepository& orgRepo,
                         IdGenerator& idGen)
    : userRepo_(userRepo), orgRepo_(orgRepo), idGen_(idGen) {}

std::string UserService::createUser(const std::string& username,
                                    const std::string& password,
                                    Role role,
                                    const std::string& unitId,
                                    long long quotaLimit) {
    if (username.empty()) throw std::invalid_argument("Username must not be empty");
    if (password.empty()) throw std::invalid_argument("Password must not be empty");
    if (userRepo_.findByUsername(username).has_value()) {
        throw std::invalid_argument("Username already exists: " + username);
    }
    if (!orgRepo_.findById(unitId).has_value()) {
        throw std::invalid_argument("Unit does not exist: " + unitId);
    }

    User u;
    u.id = idGen_.next();
    u.username = username;
    u.passwordHash = simpleHash(password);
    u.role = role;
    u.unitId = unitId;
    u.quotaLimit = quotaLimit;
    u.quotaUsed = 0;
    userRepo_.save(u);
    return u.id;
}

void UserService::changeRole(const std::string& userId, Role newRole) {
    auto opt = userRepo_.findById(userId);
    if (!opt) throw std::invalid_argument("User not found: " + userId);
    opt->role = newRole;
    userRepo_.save(*opt);
}

void UserService::changeUnit(const std::string& userId, const std::string& newUnitId) {
    auto opt = userRepo_.findById(userId);
    if (!opt) throw std::invalid_argument("User not found: " + userId);
    if (!orgRepo_.findById(newUnitId).has_value()) {
        throw std::invalid_argument("Unit does not exist: " + newUnitId);
    }
    opt->unitId = newUnitId;
    userRepo_.save(*opt);
}

void UserService::setQuotaLimit(const std::string& userId, long long limit) {
    auto opt = userRepo_.findById(userId);
    if (!opt) throw std::invalid_argument("User not found: " + userId);
    opt->quotaLimit = limit;
    userRepo_.save(*opt);
}

void UserService::adjustQuotaUsed(const std::string& userId, long long delta) {
    auto opt = userRepo_.findById(userId);
    if (!opt) throw std::invalid_argument("User not found: " + userId);
    opt->quotaUsed += delta;
    if (opt->quotaUsed < 0) opt->quotaUsed = 0;
    userRepo_.save(*opt);
}

User UserService::getUser(const std::string& userId) const {
    auto opt = userRepo_.findById(userId);
    if (!opt) throw std::invalid_argument("User not found: " + userId);
    return *opt;
}

std::optional<User> UserService::findByUsername(const std::string& username) const {
    return userRepo_.findByUsername(username);
}

std::vector<User> UserService::findAll() const {
    return userRepo_.findAll();
}

std::vector<User> UserService::findByUnit(const std::string& unitId) const {
    return userRepo_.findByUnit(unitId);
}

bool UserService::remove(const std::string& userId) {
    return userRepo_.remove(userId);
}

} // namespace dms
