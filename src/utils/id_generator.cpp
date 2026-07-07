#include "id_generator.hpp"

#include <sstream>

namespace dms {

IdGenerator::IdGenerator(std::string prefix) : prefix_(std::move(prefix)) {}

std::string IdGenerator::next() {
    ++counter_;
    std::ostringstream oss;
    oss << prefix_ << "_" << counter_;
    return oss.str();
}

} // namespace dms
