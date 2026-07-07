#pragma once

#include "sqlite_database.hpp"
#include "repositories/interfaces/i_share_repository.hpp"

namespace dms {

class SqliteShareRepository : public IShareRepository {
public:
    explicit SqliteShareRepository(SqliteDatabase& db) : db_(db) {}

    void save(const ShareRecord& record) override;
    std::vector<ShareRecord> findByDocument(const std::string& documentId) const override;
    std::vector<ShareRecord> findByToUser(const std::string& userId) const override;
    std::vector<ShareRecord> findByFromUser(const std::string& userId) const override;
    bool remove(const std::string& id) override;

private:
    ShareRecord rowToShare(struct sqlite3_stmt* stmt) const;
    SqliteDatabase& db_;
};

} // namespace dms
