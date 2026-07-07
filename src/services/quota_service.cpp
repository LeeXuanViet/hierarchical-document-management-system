#include "quota_service.hpp"

#include <sstream>
#include <iomanip>
#include <cmath>

namespace dms {

QuotaService::QuotaService(UserService& users) : users_(users) {}

long long QuotaService::used(const std::string& userId) const {
    return users_.getUser(userId).quotaUsed;
}

long long QuotaService::limit(const std::string& userId) const {
    return users_.getUser(userId).quotaLimit;
}

double QuotaService::usagePercent(const std::string& userId) const {
    auto u = users_.getUser(userId);
    if (u.quotaLimit <= 0) return 0.0;
    return (static_cast<double>(u.quotaUsed) / static_cast<double>(u.quotaLimit)) * 100.0;
}

bool QuotaService::wouldExceed(const std::string& userId, long long delta) const {
    auto u = users_.getUser(userId);
    if (u.quotaLimit <= 0) return false;
    return (u.quotaUsed + delta) > u.quotaLimit;
}

std::string QuotaService::formatBytes(long long bytes) {
    const char* units[] = {"B", "KB", "MB", "GB", "TB"};
    int idx = 0;
    double value = static_cast<double>(bytes);
    while (value >= 1024.0 && idx < 4) {
        value /= 1024.0;
        ++idx;
    }
    std::ostringstream oss;
    if (idx == 0) {
        oss << bytes << " " << units[idx];
    } else {
        oss << std::fixed << std::setprecision(2) << value << " " << units[idx];
    }
    return oss.str();
}

} // namespace dms
