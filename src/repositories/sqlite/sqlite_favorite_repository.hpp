#pragma once

#include "sqlite_database.hpp"
#include "repositories/interfaces/i_favorite_repository.hpp"

namespace dms {

class SqliteFavoriteRepository : public IFavoriteRepository {
public:
    explicit SqliteFavoriteRepository(SqliteDatabase& db) : db_(db) {}

    void add(const FavoriteRecord& record) override;
    void remove(const std::string& userId, const std::string& documentId) override;
    bool exists(const std::string& userId, const std::string& documentId) const override;
    std::vector<FavoriteRecord> findByUser(const std::string& userId) const override;

private:
    SqliteDatabase& db_;
};

} // namespace dms
