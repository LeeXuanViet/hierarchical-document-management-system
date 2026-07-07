#pragma once

#include "../repositories/interfaces/i_user_repository.hpp"
#include "../repositories/interfaces/i_org_unit_repository.hpp"
#include "../utils/id_generator.hpp"
#include "../utils/hash.hpp"

#include <vector>
#include <string>
#include <stdexcept>

namespace dms {

// Service quản lý người dùng: tạo, gán vai trò, đổi đơn vị, lấy thông tin.
class UserService {
public:
    UserService(IUserRepository& userRepo,
                IOrgUnitRepository& orgRepo,
                IdGenerator& idGen);

    // Tạo người dùng mới. Trả về id.
    std::string createUser(const std::string& username,
                           const std::string& password,
                           Role role,
                           const std::string& unitId,
                           long long quotaLimit = 0);

    // Đổi vai trò (chỉ SysAdmin nên gọi).
    void changeRole(const std::string& userId, Role newRole);

    // Đổi đơn vị của người dùng.
    void changeUnit(const std::string& userId, const std::string& newUnitId);

    // Đặt quota giới hạn (bytes). 0 = không giới hạn.
    void setQuotaLimit(const std::string& userId, long long limit);

    // Cập nhật dung lượng đã dùng (tăng/giảm delta bytes).
    void adjustQuotaUsed(const std::string& userId, long long delta);

    // Lấy thông tin người dùng.
    User getUser(const std::string& userId) const;
    std::optional<User> findByUsername(const std::string& username) const;

    // Lấy tất cả người dùng.
    std::vector<User> findAll() const;

    // Lấy người dùng thuộc một đơn vị.
    std::vector<User> findByUnit(const std::string& unitId) const;

    // Xóa người dùng.
    bool remove(const std::string& userId);

private:
    IUserRepository& userRepo_;
    IOrgUnitRepository& orgRepo_;
    IdGenerator& idGen_;
};

} // namespace dms
