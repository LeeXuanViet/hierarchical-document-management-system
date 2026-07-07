#include "sqlite_share_repository.hpp"
#include "domain/enums.hpp"

namespace dms {

static const char* safeStr(const unsigned char* p) {
    return p ? reinterpret_cast<const char*>(p) : "";
}

ShareRecord SqliteShareRepository::rowToShare(sqlite3_stmt* stmt) const
{
    ShareRecord r;
    r.id         = safeStr(sqlite3_column_text(stmt, 0));
    r.documentId = safeStr(sqlite3_column_text(stmt, 1));
    r.fromUserId = safeStr(sqlite3_column_text(stmt, 2));
    r.toUserId   = safeStr(sqlite3_column_text(stmt, 3));
    r.permission = stringToSharePermission(safeStr(sqlite3_column_text(stmt, 4)));
    return r;
}

void SqliteShareRepository::save(const ShareRecord& record)
{
    const char* sql = R"(
        INSERT OR REPLACE INTO shares (id, document_id, from_user_id, to_user_id, permission)
        VALUES (?, ?, ?, ?, ?);
    )";
    sqlite3_stmt* stmt = nullptr;
    sqlite3_prepare_v2(db_.handle(), sql, -1, &stmt, nullptr);
    sqlite3_bind_text(stmt, 1, record.id.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 2, record.documentId.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 3, record.fromUserId.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 4, record.toUserId.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 5, sharePermissionToString(record.permission).c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_step(stmt);
    sqlite3_finalize(stmt);
}

std::vector<ShareRecord> SqliteShareRepository::findByDocument(const std::string& documentId) const
{
    const char* sql = "SELECT id, document_id, from_user_id, to_user_id, permission FROM shares WHERE document_id = ?;";
    sqlite3_stmt* stmt = nullptr;
    sqlite3_prepare_v2(db_.handle(), sql, -1, &stmt, nullptr);
    sqlite3_bind_text(stmt, 1, documentId.c_str(), -1, SQLITE_TRANSIENT);
    std::vector<ShareRecord> results;
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        results.push_back(rowToShare(stmt));
    }
    sqlite3_finalize(stmt);
    return results;
}

std::vector<ShareRecord> SqliteShareRepository::findByToUser(const std::string& userId) const
{
    const char* sql = "SELECT id, document_id, from_user_id, to_user_id, permission FROM shares WHERE to_user_id = ?;";
    sqlite3_stmt* stmt = nullptr;
    sqlite3_prepare_v2(db_.handle(), sql, -1, &stmt, nullptr);
    sqlite3_bind_text(stmt, 1, userId.c_str(), -1, SQLITE_TRANSIENT);
    std::vector<ShareRecord> results;
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        results.push_back(rowToShare(stmt));
    }
    sqlite3_finalize(stmt);
    return results;
}

std::vector<ShareRecord> SqliteShareRepository::findByFromUser(const std::string& userId) const
{
    const char* sql = "SELECT id, document_id, from_user_id, to_user_id, permission FROM shares WHERE from_user_id = ?;";
    sqlite3_stmt* stmt = nullptr;
    sqlite3_prepare_v2(db_.handle(), sql, -1, &stmt, nullptr);
    sqlite3_bind_text(stmt, 1, userId.c_str(), -1, SQLITE_TRANSIENT);
    std::vector<ShareRecord> results;
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        results.push_back(rowToShare(stmt));
    }
    sqlite3_finalize(stmt);
    return results;
}

bool SqliteShareRepository::remove(const std::string& id)
{
    const char* sql = "DELETE FROM shares WHERE id = ?;";
    sqlite3_stmt* stmt = nullptr;
    sqlite3_prepare_v2(db_.handle(), sql, -1, &stmt, nullptr);
    sqlite3_bind_text(stmt, 1, id.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_step(stmt);
    int changes = sqlite3_changes(db_.handle());
    sqlite3_finalize(stmt);
    return changes > 0;
}

} // namespace dms
