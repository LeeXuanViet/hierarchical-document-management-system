#pragma once

#include "domain/favorite_record.hpp"

#include <vector>
#include <string>

namespace dms {

// Interface repository cho bản ghi favorite.
class IFavoriteRepository {
public:
    virtual ~IFavoriteRepository() = default;

    virtual void add(const FavoriteRecord& record) = 0;
    virtual void remove(const std::string& userId, const std::string& documentId) = 0;
    virtual bool exists(const std::string& userId, const std::string& documentId) const = 0;
    virtual std::vector<FavoriteRecord> findByUser(const std::string& userId) const = 0;
};

} // namespace dms
