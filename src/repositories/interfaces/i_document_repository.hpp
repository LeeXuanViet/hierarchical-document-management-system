#pragma once

#include "domain/document.hpp"

#include <vector>
#include <optional>
#include <string>

namespace dms {

// Interface repository cho tài liệu.
class IDocumentRepository {
public:
    virtual ~IDocumentRepository() = default;

    virtual void save(const Document& doc) = 0;
    virtual std::optional<Document> findById(const std::string& id) const = 0;
    virtual std::vector<Document> findAll() const = 0;
    virtual std::vector<Document> findByOwner(const std::string& userId) const = 0;
    virtual std::vector<Document> findByUnitScope(const std::string& unitId) const = 0;
    virtual bool remove(const std::string& id) = 0;
};

} // namespace dms
