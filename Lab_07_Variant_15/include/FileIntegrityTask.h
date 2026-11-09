#pragma once
#include <cstdint>
#include <filesystem>
#include <string>
namespace integrity {
enum class Algorithm { FNV1A64, CRC32 };
struct FileInfo {
    std::filesystem::path path;
    std::uintmax_t size{};
    std::string checksum;
    Algorithm algorithm{Algorithm::FNV1A64};
    bool operator==(const FileInfo&) const = default;
};
class FileIntegrityTask {
    FileInfo current_{};
    bool has_value_{false};
public:
    bool hasValue() const noexcept { return has_value_; }
    const FileInfo& info() const;
    // Strong guarantee: prepare in local temporaries; commit with noexcept swaps.
    void analyze(const std::filesystem::path& path, Algorithm algorithm);
    bool verify(const std::string& expected) const;
};
std::string checksumFile(const std::filesystem::path& path, Algorithm algorithm);
}