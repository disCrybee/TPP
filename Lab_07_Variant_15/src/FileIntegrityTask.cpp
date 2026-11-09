#include "FileIntegrityTask.h"
#include "FileIntegrityExceptions.h"
#include <array>
#include <fstream>
#include <iomanip>
#include <sstream>
#include <system_error>
#include <utility>
namespace integrity {
namespace {
void checkAlgorithm(Algorithm algorithm) {
    if (algorithm != Algorithm::FNV1A64 && algorithm != Algorithm::CRC32)
        throw std::invalid_argument("Unsupported checksum algorithm");
}
std::string hexValue(std::uint64_t value, int width) {
    std::ostringstream output;
    output << std::hex << std::setfill('0') << std::setw(width) << value;
    return output.str();
}
std::string calculate(std::istream& stream, Algorithm algorithm) {
    std::array<char, 4096> buffer{};
    std::uint64_t fnv = 14695981039346656037ULL;
    std::uint32_t crc = 0xFFFFFFFFu;
    while (stream) {
        stream.read(buffer.data(), static_cast<std::streamsize>(buffer.size()));
        const auto count = stream.gcount();
        for (std::streamsize i = 0; i < count; ++i) {
            const auto byte = static_cast<unsigned char>(buffer[static_cast<std::size_t>(i)]);
            if (algorithm == Algorithm::FNV1A64) {
                fnv ^= byte;
                fnv *= 1099511628211ULL;
            } else {
                crc ^= byte;
                for (int bit = 0; bit < 8; ++bit)
                    crc = (crc >> 1) ^ ((crc & 1u) ? 0xEDB88320u : 0u);
            }
        }
    }
    if (stream.bad() || (!stream.eof() && stream.fail()))
        throw FileAccessException("<input stream>", "read failed");
    return algorithm == Algorithm::FNV1A64 ? hexValue(fnv, 16) : hexValue(crc ^ 0xFFFFFFFFu, 8);
}
}
std::string checksumFile(const std::filesystem::path& path, Algorithm algorithm) {
    checkAlgorithm(algorithm);
    std::error_code ec;
    if (!std::filesystem::exists(path, ec) || ec)
        throw FileAccessException(path.string(), "file not found");
    if (!std::filesystem::is_regular_file(path, ec) || ec)
        throw FileAccessException(path.string(), "not a regular file");
    std::ifstream file(path, std::ios::binary);
    if (!file.is_open()) throw FileAccessException(path.string(), "cannot open");
    return calculate(file, algorithm);
}
const FileInfo& FileIntegrityTask::info() const {
    if (!has_value_) throw std::logic_error("File has not been analyzed");
    return current_;
}
void FileIntegrityTask::analyze(const std::filesystem::path& path, Algorithm algorithm) {
    checkAlgorithm(algorithm);
    std::error_code ec;
    if (!std::filesystem::exists(path, ec) || ec)
        throw FileAccessException(path.string(), "file not found");
    if (!std::filesystem::is_regular_file(path, ec) || ec)
        throw FileAccessException(path.string(), "not a regular file");
    const auto size = std::filesystem::file_size(path, ec);
    if (ec || size == static_cast<std::uintmax_t>(-1)) throw FileSizeException();
    // All potentially throwing work is performed before state mutation.
    FileInfo candidate{path, size, checksumFile(path, algorithm), algorithm};
    current_.path.swap(candidate.path);
    current_.checksum.swap(candidate.checksum);
    current_.size = candidate.size;
    current_.algorithm = candidate.algorithm;
    has_value_ = true;
}
bool FileIntegrityTask::verify(const std::string& expected) const {
    const auto& actual = info();
    const std::size_t width = actual.algorithm == Algorithm::FNV1A64 ? 16 : 8;
    if (expected.size() != width) throw ChecksumFormatException();
    for (char c : expected)
        if (!((c >= '0' && c <= '9') || (c >= 'a' && c <= 'f') || (c >= 'A' && c <= 'F')))
            throw ChecksumFormatException();
    std::string lower = expected;
    for (char& c : lower) if (c >= 'A' && c <= 'F') c = static_cast<char>(c - 'A' + 'a');
    return lower == actual.checksum;
}
}