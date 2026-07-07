#pragma once

#include "domain/user.hpp"

#include <vector>
#include <optional>
#include <string>

namespace dms {

// Interface repository cho người dùng.
class IUserRepository {
public:
    virtual ~IUserRepository() = default;

    virtual void save(const User& user) = 0;
    virtual std::optional<User> findById(const std::string& id) const = 0;
    virtual std::optional<User> findByUsername(const std::string& username) const = 0;
    virtual std::vector<User> findAll() const = 0;
    virtual std::vector<User> findByUnit(const std::string& unitId) const = 0;
    virtual bool remove(const std::string& id) = 0;
};

} // namespace dms
