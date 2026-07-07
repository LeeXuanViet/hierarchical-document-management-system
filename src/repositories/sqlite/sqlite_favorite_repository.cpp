#include "sqlite_favorite_repository.hpp"

namespace dms {

void SqliteFavoriteRepository::add(const FavoriteRecord& record)
{
    const char* sql = "INSERT OR IGNORE INTO favorites (user_id, document_id) VALUES (?, ?);";
    sqlite3_stmt* stmt = nullptr;
    sqlite3_prepare_v2(db_.handle(), sql, -1, &stmt, nullptr);
    sqlite3_bind_text(stmt, 1, record.userId.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 2, record.documentId.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_step(stmt);
    sqlite3_finalize(stmt);
}

void SqliteFavoriteRepository::remove(const std::string& userId, const std::string& documentId)
{
    const char* sql = "DELETE FROM favorites WHERE user_id = ? AND document_id = ?;";
    sqlite3_stmt* stmt = nullptr;
    sqlite3_prepare_v2(db_.handle(), sql, -1, &stmt, nullptr);
    sqlite3_bind_text(stmt, 1, userId.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 2, documentId.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_step(stmt);
    sqlite3_finalize(stmt);
}

bool SqliteFavoriteRepository::exists(const std::string& userId, const std::string& documentId) const
{
    const char* sql = "SELECT 1 FROM favorites WHERE user_id = ? AND document_id = ?;";
    sqlite3_stmt* stmt = nullptr;
    sqlite3_prepare_v2(db_.handle(), sql, -1, &stmt, nullptr);
    sqlite3_bind_text(stmt, 1, userId.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 2, documentId.c_str(), -1, SQLITE_TRANSIENT);
    bool found = (sqlite3_step(stmt) == SQLITE_ROW);
    sqlite3_finalize(stmt);
    return found;
}

std::vector<FavoriteRecord> SqliteFavoriteRepository::findByUser(const std::string& userId) const
{
    const char* sql = "SELECT user_id, document_id FROM favorites WHERE user_id = ?;";
    sqlite3_stmt* stmt = nullptr;
    sqlite3_prepare_v2(db_.handle(), sql, -1, &stmt, nullptr);
    sqlite3_bind_text(stmt, 1, userId.c_str(), -1, SQLITE_TRANSIENT);
    std::vector<FavoriteRecord> results;
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        FavoriteRecord r;
        r.userId     = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0));
        r.documentId = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
        results.push_back(r);
    }
    sqlite3_finalize(stmt);
    return results;
}

} // namespace dms
