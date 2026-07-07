#pragma once

#include "domain/share_record.hpp"

#include <vector>
#include <optional>
#include <string>

namespace dms {

// Interface repository cho bản ghi chia sẻ.
class IShareRepository {
public:
    virtual ~IShareRepository() = default;

    virtual void save(const ShareRecord& record) = 0;
    virtual std::vector<ShareRecord> findByDocument(const std::string& documentId) const = 0;
    virtual std::vector<ShareRecord> findByToUser(const std::string& userId) const = 0;
    virtual std::vector<ShareRecord> findByFromUser(const std::string& userId) const = 0;
    virtual bool remove(const std::string& id) = 0;
};

} // namespace dms
