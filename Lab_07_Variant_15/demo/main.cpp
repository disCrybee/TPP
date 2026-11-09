#include "FileIntegrityTask.h"
#include "FileIntegrityExceptions.h"
#include <filesystem>
#include <fstream>
#include <iostream>
#include <string>
#include <utility>
using namespace integrity;
struct ScopeLogger {
    std::string name;
    explicit ScopeLogger(std::string value): name(std::move(value)) { std::cout << "Created: " << name << '\n'; }
    ~ScopeLogger() noexcept { std::cout << "Destroyed: " << name << '\n'; }
};
void level3(FileIntegrityTask& task, const std::filesystem::path& file) {
    ScopeLogger log("level3");
    task.analyze(file, Algorithm::FNV1A64);
}
void level2(FileIntegrityTask& task, const std::filesystem::path& file) {
    ScopeLogger log("level2");
    try { level3(task, file); }
    catch (const FileIntegrityException& error) {
        std::cout << "Level2 diagnostics: " << error.what() << '\n';
        throw;
    }
}
void level1(FileIntegrityTask& task, const std::filesystem::path& file) {
    ScopeLogger log("level1");
    level2(task, file);
}
int main() {
    const auto sample = std::filesystem::temp_directory_path() / "integrity_demo_sample.txt";
    { std::ofstream out(sample, std::ios::binary); out << "hello"; }
    FileIntegrityTask task;
    try {
        task.analyze(sample, Algorithm::FNV1A64);
        std::cout << "File size: " << task.info().size << '\n';
        std::cout << "FNV-1a: " << task.info().checksum << '\n';
        std::cout << "Verified: " << std::boolalpha << task.verify("a430d84680aabd0b") << '\n';
        std::cout << "Unwinding demonstration:\n";
        level1(task, sample.string() + ".missing");
    } catch (const FileAccessException& error) {
        std::cout << "Caught FileAccessException: " << error.what() << '\n';
    } catch (const FileIntegrityException& error) {
        std::cout << "Caught FileIntegrityException: " << error.what() << '\n';
    } catch (const std::exception& error) {
        std::cout << "Caught std::exception: " << error.what() << '\n';
    }
    std::error_code ec;
    std::filesystem::remove(sample, ec);
}