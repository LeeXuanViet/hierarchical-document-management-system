#pragma once

#include "sqlite_database.hpp"
#include "repositories/interfaces/i_document_repository.hpp"

namespace dms {

class SqliteDocumentRepository : public IDocumentRepository {
public:
    explicit SqliteDocumentRepository(SqliteDatabase& db) : db_(db) {}

    void save(const Document& doc) override;
    std::optional<Document> findById(const std::string& id) const override;
    std::vector<Document> findAll() const override;
    std::vector<Document> findByOwner(const std::string& userId) const override;
    std::vector<Document> findByUnitScope(const std::string& unitId) const override;
    bool remove(const std::string& id) override;

private:
    Document rowToDocument(struct sqlite3_stmt* stmt) const;
    SqliteDatabase& db_;
};

} // namespace dms
