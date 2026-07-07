#include "sqlite_database.hpp"
#include <stdexcept>

namespace dms {

SqliteDatabase::SqliteDatabase(const std::string& dbPath)
{
    // Check if DB file exists to determine if it's new
    FILE* f = fopen(dbPath.c_str(), "r");
    if (f) {
        fclose(f);
        isNew_ = false;
    } else {
        isNew_ = true;
    }

    int rc = sqlite3_open(dbPath.c_str(), &db_);
    if (rc != SQLITE_OK) {
        std::string err = sqlite3_errmsg(db_);
        sqlite3_close(db_);
        db_ = nullptr;
        throw std::runtime_error("Cannot open SQLite database: " + err);
    }

    // Enable WAL mode for better concurrent read performance
    exec("PRAGMA journal_mode=WAL;");
    exec("PRAGMA foreign_keys=ON;");

    createSchema();
}

SqliteDatabase::~SqliteDatabase()
{
    if (db_) {
        sqlite3_close(db_);
        db_ = nullptr;
    }
}

void SqliteDatabase::exec(const char* sql)
{
    char* errMsg = nullptr;
    int rc = sqlite3_exec(db_, sql, nullptr, nullptr, &errMsg);
    if (rc != SQLITE_OK) {
        std::string err = errMsg ? errMsg : "unknown error";
        sqlite3_free(errMsg);
        throw std::runtime_error("SQLite exec error: " + err);
    }
}

void SqliteDatabase::createSchema()
{
    const char* schema = R"SQL(
        CREATE TABLE IF NOT EXISTS org_units (
            id TEXT PRIMARY KEY,
            name TEXT NOT NULL,
            parent_id TEXT
        );

        CREATE TABLE IF NOT EXISTS users (
            id TEXT PRIMARY KEY,
            username TEXT UNIQUE NOT NULL,
            password_hash TEXT NOT NULL,
            role TEXT NOT NULL DEFAULT 'USER',
            unit_id TEXT NOT NULL,
            quota_limit INTEGER DEFAULT 0,
            quota_used INTEGER DEFAULT 0
        );

        CREATE TABLE IF NOT EXISTS documents (
            id TEXT PRIMARY KEY,
            owner_user_id TEXT NOT NULL,
            unit_scope_id TEXT,
            visibility TEXT NOT NULL DEFAULT 'PERSONAL',
            title TEXT NOT NULL,
            content_path TEXT NOT NULL,
            content_type TEXT,
            size INTEGER DEFAULT 0,
            created_at TEXT NOT NULL,
            updated_at TEXT NOT NULL,
            is_deleted INTEGER DEFAULT 0
        );

        CREATE TABLE IF NOT EXISTS shares (
            id TEXT PRIMARY KEY,
            document_id TEXT NOT NULL,
            from_user_id TEXT NOT NULL,
            to_user_id TEXT NOT NULL,
            permission TEXT NOT NULL DEFAULT 'READ'
        );

        CREATE TABLE IF NOT EXISTS favorites (
            user_id TEXT NOT NULL,
            document_id TEXT NOT NULL,
            PRIMARY KEY (user_id, document_id)
        );

        CREATE INDEX IF NOT EXISTS idx_documents_owner ON documents(owner_user_id);
        CREATE INDEX IF NOT EXISTS idx_documents_unit ON documents(unit_scope_id);
        CREATE INDEX IF NOT EXISTS idx_shares_doc ON shares(document_id);
        CREATE INDEX IF NOT EXISTS idx_shares_to ON shares(to_user_id);
        CREATE INDEX IF NOT EXISTS idx_users_unit ON users(unit_id);
    )SQL";

    exec(schema);

    // Determine isNew_ more accurately: check if users table has rows
    if (!isNew_) {
        sqlite3_stmt* stmt = nullptr;
        sqlite3_prepare_v2(db_, "SELECT COUNT(*) FROM users", -1, &stmt, nullptr);
        if (sqlite3_step(stmt) == SQLITE_ROW) {
            isNew_ = (sqlite3_column_int(stmt, 0) == 0);
        }
        sqlite3_finalize(stmt);
    }
}

} // namespace dms
