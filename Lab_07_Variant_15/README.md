# Lab 7 — File Integrity (Variant 15)

C++20 static library for checking file integrity using FNV-1a 64-bit and CRC32.

## Layout
- `include/` — public API and custom exception classes
- `src/` — library implementation
- `demo/` — exception propagation and RAII demonstration
- `antipattern/` — examples of catching non-standard exceptions
- `tests/` — Google Test suite (15 tests) and local smoke checks

## Build on Windows with Visual Studio
Install CMake, Ninja and vcpkg (with Google Test), then open this folder as a CMake project. Configure vcpkg using `CMAKE_TOOLCHAIN_FILE` or your Visual Studio vcpkg integration.

```powershell
cmake -S . -B build -DCMAKE_TOOLCHAIN_FILE="C:/path/to/vcpkg/scripts/buildsystems/vcpkg.cmake"
cmake --build build --config Debug
ctest --test-dir build --output-on-failure
```

For a build without Google Test, use `-DBUILD_TESTING=OFF`.

## Behavior
`FileIntegrityTask::analyze()` validates file paths and algorithms, obtains file size, calculates checksum and only then commits state changes. Failed analysis leaves previous state unchanged.

The accompanying laboratory report was produced separately; Google Test integration requires validation in the target Visual Studio environment.
