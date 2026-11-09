#include "FileIntegrityTask.h"
#include "FileIntegrityExceptions.h"
#include <gtest/gtest.h>
#include <filesystem>
#include <fstream>
#include <string>
using namespace integrity;
class FileTest : public ::testing::Test {
protected:
    std::filesystem::path path;
    void SetUp() override {
        path = std::filesystem::temp_directory_path() /
            ("integrity_test_" + std::to_string(reinterpret_cast<std::uintptr_t>(this)) + ".bin");
        std::ofstream output(path, std::ios::binary);
        output << "hello";
    }
    void TearDown() override { std::error_code ec; std::filesystem::remove(path, ec); }
};
TEST_F(FileTest, FNVKnownVector) { EXPECT_EQ(checksumFile(path, Algorithm::FNV1A64), "a430d84680aabd0b"); }
TEST_F(FileTest, CRC32KnownVector) { EXPECT_EQ(checksumFile(path, Algorithm::CRC32), "3610a686"); }
TEST_F(FileTest, AnalyzeUpdatesState) {
    FileIntegrityTask task; task.analyze(path, Algorithm::FNV1A64);
    EXPECT_TRUE(task.hasValue()); EXPECT_EQ(task.info().size, 5u);
    EXPECT_EQ(task.info().checksum, "a430d84680aabd0b");
}
TEST_F(FileTest, VerificationMatchesOrDiffers) {
    FileIntegrityTask task; task.analyze(path, Algorithm::CRC32);
    EXPECT_TRUE(task.verify("3610A686")); EXPECT_FALSE(task.verify("00000000"));
}
TEST_F(FileTest, MissingFileThrows) {
    EXPECT_THROW(checksumFile(path.string()+".missing", Algorithm::FNV1A64), FileAccessException);
}
TEST_F(FileTest, DirectoryThrows) {
    EXPECT_THROW(checksumFile(path.parent_path(), Algorithm::CRC32), FileAccessException);
}
TEST_F(FileTest, InvalidAlgorithmThrows) {
    EXPECT_THROW(checksumFile(path, static_cast<Algorithm>(99)), std::invalid_argument);
}
TEST_F(FileTest, MalformedChecksumThrows) {
    FileIntegrityTask task; task.analyze(path, Algorithm::CRC32);
    EXPECT_THROW(task.verify("wrong"), ChecksumFormatException);
    EXPECT_THROW(task.verify("XXXXXXXX"), ChecksumFormatException);
}
TEST_F(FileTest, NoInfoThrows) {
    FileIntegrityTask task; EXPECT_THROW(task.info(), std::logic_error);
}
TEST_F(FileTest, StrongGuaranteeMissingFile) {
    FileIntegrityTask task; task.analyze(path, Algorithm::FNV1A64);
    const FileInfo before = task.info();
    EXPECT_THROW(task.analyze(path.string()+".missing", Algorithm::CRC32), FileAccessException);
    EXPECT_EQ(task.info(), before);
}
TEST_F(FileTest, StrongGuaranteeInvalidAlgorithm) {
    FileIntegrityTask task; task.analyze(path, Algorithm::FNV1A64);
    const FileInfo before = task.info();
    EXPECT_THROW(task.analyze(path, static_cast<Algorithm>(99)), std::invalid_argument);
    EXPECT_EQ(task.info(), before);
}
TEST_F(FileTest, EmptyFileBoundary) {
    { std::ofstream output(path, std::ios::trunc | std::ios::binary); }
    FileIntegrityTask task; task.analyze(path, Algorithm::FNV1A64);
    EXPECT_EQ(task.info().size, 0u);
    EXPECT_EQ(task.info().checksum, "cbf29ce484222325");
}
TEST_F(FileTest, SameContentSameChecksum) {
    FileIntegrityTask task; task.analyze(path, Algorithm::CRC32);
    EXPECT_EQ(task.info().checksum, checksumFile(path, Algorithm::CRC32));
}
TEST_F(FileTest, ModifiedContentChangesChecksum) {
    FileIntegrityTask task; task.analyze(path, Algorithm::CRC32);
    const auto before = task.info().checksum;
    { std::ofstream output(path, std::ios::app | std::ios::binary); output << "!"; }
    task.analyze(path, Algorithm::CRC32);
    EXPECT_NE(task.info().checksum, before);
}
TEST_F(FileTest, BoundaryNumericComparison) {
    FileIntegrityTask task; task.analyze(path, Algorithm::CRC32);
    EXPECT_NEAR(static_cast<double>(task.info().size), 5.0, 1e-12);
}