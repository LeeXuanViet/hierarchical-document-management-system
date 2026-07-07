#include "sqlite_user_repository.hpp"
#include "domain/enums.hpp"

namespace dms {

User SqliteUserRepository::rowToUser(sqlite3_stmt* stmt) const
{
    User u;
    u.id           = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0));
    u.username     = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
    u.passwordHash = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2));
    u.role         = stringToRole(reinterpret_cast<const char*>(sqlite3_column_text(stmt, 3)));
    u.unitId       = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 4));
    u.quotaLimit   = sqlite3_column_int64(stmt, 5);
    u.quotaUsed    = sqlite3_column_int64(stmt, 6);
    return u;
}

void SqliteUserRepository::save(const User& user)
{
    const char* sql = R"(
        INSERT OR REPLACE INTO users (id, username, password_hash, role, unit_id, quota_limit, quota_used)
        VALUES (?, ?, ?, ?, ?, ?, ?);
    )";
    sqlite3_stmt* stmt = nullptr;
    sqlite3_prepare_v2(db_.handle(), sql, -1, &stmt, nullptr);
    sqlite3_bind_text(stmt, 1, user.id.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 2, user.username.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 3, user.passwordHash.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 4, roleToString(user.role).c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 5, user.unitId.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_int64(stmt, 6, user.quotaLimit);
    sqlite3_bind_int64(stmt, 7, user.quotaUsed);
    sqlite3_step(stmt);
    sqlite3_finalize(stmt);
}

std::optional<User> SqliteUserRepository::findById(const std::string& id) const
{
    const char* sql = "SELECT id, username, password_hash, role, unit_id, quota_limit, quota_used FROM users WHERE id = ?;";
    sqlite3_stmt* stmt = nullptr;
    sqlite3_prepare_v2(db_.handle(), sql, -1, &stmt, nullptr);
    sqlite3_bind_text(stmt, 1, id.c_str(), -1, SQLITE_TRANSIENT);
    std::optional<User> result;
    if (sqlite3_step(stmt) == SQLITE_ROW) {
        result = rowToUser(stmt);
    }
    sqlite3_finalize(stmt);
    return result;
}

std::optional<User> SqliteUserRepository::findByUsername(const std::string& username) const
{
    const char* sql = "SELECT id, username, password_hash, role, unit_id, quota_limit, quota_used FROM users WHERE username = ?;";
    sqlite3_stmt* stmt = nullptr;
    sqlite3_prepare_v2(db_.handle(), sql, -1, &stmt, nullptr);
    sqlite3_bind_text(stmt, 1, username.c_str(), -1, SQLITE_TRANSIENT);
    std::optional<User> result;
    if (sqlite3_step(stmt) == SQLITE_ROW) {
        result = rowToUser(stmt);
    }
    sqlite3_finalize(stmt);
    return result;
}

std::vector<User> SqliteUserRepository::findAll() const
{
    const char* sql = "SELECT id, username, password_hash, role, unit_id, quota_limit, quota_used FROM users;";
    sqlite3_stmt* stmt = nullptr;
    sqlite3_prepare_v2(db_.handle(), sql, -1, &stmt, nullptr);
    std::vector<User> results;
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        results.push_back(rowToUser(stmt));
    }
    sqlite3_finalize(stmt);
    return results;
}

std::vector<User> SqliteUserRepository::findByUnit(const std::string& unitId) const
{
    const char* sql = "SELECT id, username, password_hash, role, unit_id, quota_limit, quota_used FROM users WHERE unit_id = ?;";
    sqlite3_stmt* stmt = nullptr;
    sqlite3_prepare_v2(db_.handle(), sql, -1, &stmt, nullptr);
    sqlite3_bind_text(stmt, 1, unitId.c_str(), -1, SQLITE_TRANSIENT);
    std::vector<User> results;
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        results.push_back(rowToUser(stmt));
    }
    sqlite3_finalize(stmt);
    return results;
}

bool SqliteUserRepository::remove(const std::string& id)
{
    const char* sql = "DELETE FROM users WHERE id = ?;";
    sqlite3_stmt* stmt = nullptr;
    sqlite3_prepare_v2(db_.handle(), sql, -1, &stmt, nullptr);
    sqlite3_bind_text(stmt, 1, id.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_step(stmt);
    int changes = sqlite3_changes(db_.handle());
    sqlite3_finalize(stmt);
    return changes > 0;
}

} // namespace dms
