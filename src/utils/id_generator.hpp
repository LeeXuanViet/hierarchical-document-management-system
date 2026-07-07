#pragma once

#include <string>

namespace dms {

// Sinh id ngắn dạng prefix + số thứ tự + timestamp, đủ duy nhất cho demo.
class IdGenerator {
public:
    explicit IdGenerator(std::string prefix);
    std::string next();

private:
    std::string prefix_;
    long long counter_ = 0;
};

} // namespace dms
