#pragma once
#include <stdexcept>
#include <string>
#include <utility>
class FileIntegrityException : public std::runtime_error {
public: explicit FileIntegrityException(const std::string& message): std::runtime_error(message) {}
};
class FileAccessException : public FileIntegrityException {
    std::string path_;
public:
    FileAccessException(std::string path, std::string reason): FileIntegrityException("File access error: " + reason + ": " + path), path_(std::move(path)) {}
    const std::string& path() const noexcept { return path_; }
};
class ChecksumFormatException : public FileIntegrityException {
public: ChecksumFormatException(): FileIntegrityException("Invalid checksum format") {}
};
class FileSizeException : public FileIntegrityException {
public: FileSizeException(): FileIntegrityException("Unable to determine file size") {}
};