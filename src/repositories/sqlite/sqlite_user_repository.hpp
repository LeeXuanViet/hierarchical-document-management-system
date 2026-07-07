#pragma once

#include "sqlite_database.hpp"
#include "repositories/interfaces/i_user_repository.hpp"

namespace dms {

class SqliteUserRepository : public IUserRepository {
public:
    explicit SqliteUserRepository(SqliteDatabase& db) : db_(db) {}

    void save(const User& user) override;
    std::optional<User> findById(const std::string& id) const override;
    std::optional<User> findByUsername(const std::string& username) const override;
    std::vector<User> findAll() const override;
    std::vector<User> findByUnit(const std::string& unitId) const override;
    bool remove(const std::string& id) override;

private:
    User rowToUser(struct sqlite3_stmt* stmt) const;
    SqliteDatabase& db_;
};

} // namespace dms
