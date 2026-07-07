#include "sqlite_document_repository.hpp"
#include "domain/enums.hpp"

namespace dms {

static const char* safeStr(const unsigned char* p) {
    return p ? reinterpret_cast<const char*>(p) : "";
}

Document SqliteDocumentRepository::rowToDocument(sqlite3_stmt* stmt) const
{
    Document d;
    d.id          = safeStr(sqlite3_column_text(stmt, 0));
    d.ownerUserId = safeStr(sqlite3_column_text(stmt, 1));
    d.unitScopeId = safeStr(sqlite3_column_text(stmt, 2));
    d.visibility  = stringToVisibility(safeStr(sqlite3_column_text(stmt, 3)));
    d.title       = safeStr(sqlite3_column_text(stmt, 4));
    d.contentPath = safeStr(sqlite3_column_text(stmt, 5));
    d.contentType = safeStr(sqlite3_column_text(stmt, 6));
    d.size        = sqlite3_column_int64(stmt, 7);
    d.createdAt   = safeStr(sqlite3_column_text(stmt, 8));
    d.updatedAt   = safeStr(sqlite3_column_text(stmt, 9));
    d.isDeleted   = sqlite3_column_int(stmt, 10) != 0;
    return d;
}

void SqliteDocumentRepository::save(const Document& doc)
{
    const char* sql = R"(
        INSERT OR REPLACE INTO documents
            (id, owner_user_id, unit_scope_id, visibility, title, content_path,
             content_type, size, created_at, updated_at, is_deleted)
        VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?);
    )";
    sqlite3_stmt* stmt = nullptr;
    sqlite3_prepare_v2(db_.handle(), sql, -1, &stmt, nullptr);
    sqlite3_bind_text(stmt, 1, doc.id.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 2, doc.ownerUserId.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 3, doc.unitScopeId.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 4, visibilityToString(doc.visibility).c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 5, doc.title.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 6, doc.contentPath.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 7, doc.contentType.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_int64(stmt, 8, doc.size);
    sqlite3_bind_text(stmt, 9, doc.createdAt.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 10, doc.updatedAt.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_int(stmt, 11, doc.isDeleted ? 1 : 0);
    sqlite3_step(stmt);
    sqlite3_finalize(stmt);
}

std::optional<Document> SqliteDocumentRepository::findById(const std::string& id) const
{
    const char* sql = "SELECT id, owner_user_id, unit_scope_id, visibility, title, content_path, "
                      "content_type, size, created_at, updated_at, is_deleted FROM documents WHERE id = ?;";
    sqlite3_stmt* stmt = nullptr;
    sqlite3_prepare_v2(db_.handle(), sql, -1, &stmt, nullptr);
    sqlite3_bind_text(stmt, 1, id.c_str(), -1, SQLITE_TRANSIENT);
    std::optional<Document> result;
    if (sqlite3_step(stmt) == SQLITE_ROW) {
        result = rowToDocument(stmt);
    }
    sqlite3_finalize(stmt);
    return result;
}

std::vector<Document> SqliteDocumentRepository::findAll() const
{
    const char* sql = "SELECT id, owner_user_id, unit_scope_id, visibility, title, content_path, "
                      "content_type, size, created_at, updated_at, is_deleted FROM documents;";
    sqlite3_stmt* stmt = nullptr;
    sqlite3_prepare_v2(db_.handle(), sql, -1, &stmt, nullptr);
    std::vector<Document> results;
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        results.push_back(rowToDocument(stmt));
    }
    sqlite3_finalize(stmt);
    return results;
}

std::vector<Document> SqliteDocumentRepository::findByOwner(const std::string& userId) const
{
    const char* sql = "SELECT id, owner_user_id, unit_scope_id, visibility, title, content_path, "
                      "content_type, size, created_at, updated_at, is_deleted FROM documents WHERE owner_user_id = ?;";
    sqlite3_stmt* stmt = nullptr;
    sqlite3_prepare_v2(db_.handle(), sql, -1, &stmt, nullptr);
    sqlite3_bind_text(stmt, 1, userId.c_str(), -1, SQLITE_TRANSIENT);
    std::vector<Document> results;
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        results.push_back(rowToDocument(stmt));
    }
    sqlite3_finalize(stmt);
    return results;
}

std::vector<Document> SqliteDocumentRepository::findByUnitScope(const std::string& unitId) const
{
    const char* sql = "SELECT id, owner_user_id, unit_scope_id, visibility, title, content_path, "
                      "content_type, size, created_at, updated_at, is_deleted FROM documents WHERE unit_scope_id = ?;";
    sqlite3_stmt* stmt = nullptr;
    sqlite3_prepare_v2(db_.handle(), sql, -1, &stmt, nullptr);
    sqlite3_bind_text(stmt, 1, unitId.c_str(), -1, SQLITE_TRANSIENT);
    std::vector<Document> results;
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        results.push_back(rowToDocument(stmt));
    }
    sqlite3_finalize(stmt);
    return results;
}

bool SqliteDocumentRepository::remove(const std::string& id)
{
    const char* sql = "DELETE FROM documents WHERE id = ?;";
    sqlite3_stmt* stmt = nullptr;
    sqlite3_prepare_v2(db_.handle(), sql, -1, &stmt, nullptr);
    sqlite3_bind_text(stmt, 1, id.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_step(stmt);
    int changes = sqlite3_changes(db_.handle());
    sqlite3_finalize(stmt);
    return changes > 0;
}

} // namespace dms
