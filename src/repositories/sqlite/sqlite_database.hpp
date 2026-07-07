#pragma once

#include <sqlite3.h>
#include <string>
#include <stdexcept>

namespace dms {

// Manages SQLite connection lifecycle and schema initialization.
class SqliteDatabase {
public:
    explicit SqliteDatabase(const std::string& dbPath = "dms.db");
    ~SqliteDatabase();

    // Non-copyable
    SqliteDatabase(const SqliteDatabase&) = delete;
    SqliteDatabase& operator=(const SqliteDatabase&) = delete;

    sqlite3* handle() const { return db_; }

    // Returns true if the database was just created (no users table existed).
    bool isNewDatabase() const { return isNew_; }

    // Execute a simple SQL statement (no results).
    void exec(const char* sql);

private:
    void createSchema();

    sqlite3* db_ = nullptr;
    bool isNew_ = false;
};

} // namespace dms
