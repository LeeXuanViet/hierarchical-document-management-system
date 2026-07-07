#include "file_store.hpp"

#include <sys/stat.h>
#include <sys/types.h>

#include <fstream>
#include <sstream>
#include <cstdio>

namespace dms {

namespace {

// Tạo thư mục đệ quy (giới hạn POSIX).
bool makeDirs(const std::string& path) {
    std::string current;
    for (size_t i = 0; i < path.size(); ++i) {
        current += path[i];
        if (path[i] == '/' && !current.empty()) {
            ::mkdir(current.c_str(), 0755);
        }
    }
    ::mkdir(path.c_str(), 0755);
    struct stat st;
    return ::stat(path.c_str(), &st) == 0 && S_ISDIR(st.st_mode);
}

} // namespace

FileStore::FileStore(std::string baseDir) : baseDir_(std::move(baseDir)) {
    makeDirs(baseDir_);
}

std::string FileStore::write(const std::string& documentId, const std::string& content) {
    std::string filename = documentId + ".txt";
    std::string fullPath = baseDir_ + "/" + filename;
    std::ofstream out(fullPath, std::ios::binary);
    if (!out) return "";
    out.write(content.data(), static_cast<std::streamsize>(content.size()));
    out.close();
    if (!out) return "";
    return filename; // contentPath tương đối
}

std::string FileStore::read(const std::string& contentPath) const {
    std::string fullPath = baseDir_ + "/" + contentPath;
    std::ifstream in(fullPath, std::ios::binary);
    if (!in) return "";
    std::ostringstream ss;
    ss << in.rdbuf();
    return ss.str();
}

bool FileStore::remove(const std::string& contentPath) const {
    std::string fullPath = baseDir_ + "/" + contentPath;
    if (::remove(fullPath.c_str()) == 0) return true;
    struct stat st;
    return ::stat(fullPath.c_str(), &st) != 0; // không tồn tại cũng coi như xóa xong
}

long long FileStore::size(const std::string& contentPath) const {
    std::string fullPath = baseDir_ + "/" + contentPath;
    struct stat st;
    if (::stat(fullPath.c_str(), &st) != 0) return -1;
    return static_cast<long long>(st.st_size);
}

} // namespace dms
