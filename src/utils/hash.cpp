#include "hash.hpp"

#include <cstdint>
#include <sstream>
#include <iomanip>

namespace dms {

// FNV-1a 64-bit, đủ cho mục đích demo (không dùng cho production).
std::string simpleHash(const std::string& input) {
    uint64_t hash = 1469598103934665603ULL; // FNV offset basis
    for (unsigned char c : input) {
        hash ^= c;
        hash *= 1099511628211ULL; // FNV prime
    }
    std::ostringstream oss;
    oss << std::hex << std::setw(16) << std::setfill('0') << hash;
    return oss.str();
}

bool verifyPassword(const std::string& password, const std::string& hash) {
    return simpleHash(password) == hash;
}

} // namespace dms
