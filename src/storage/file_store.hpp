#pragma once

#include <string>

namespace dms {

// Lớp lưu trữ nội dung file text trên đĩa.
// Mỗi tài liệu được lưu thành một file riêng trong thư mục data/files.
class FileStore {
public:
    explicit FileStore(std::string baseDir);

    // Ghi nội dung text vào file, trả về đường dẫn tương đối dùng làm contentPath.
    // Trả về rỗng nếu thất bại.
    std::string write(const std::string& documentId, const std::string& content);

    // Đọc nội dung text từ contentPath. Trả về rỗng nếu không đọc được.
    std::string read(const std::string& contentPath) const;

    // Xóa file theo contentPath. Trả về true nếu xóa thành công hoặc file không tồn tại.
    bool remove(const std::string& contentPath) const;

    // Kích thước file theo byte, -1 nếu không tồn tại.
    long long size(const std::string& contentPath) const;

    const std::string& baseDir() const { return baseDir_; }

private:
    std::string baseDir_;
};

} // namespace dms
