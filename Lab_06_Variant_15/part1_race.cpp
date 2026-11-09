#include <atomic>
#include <chrono>
#include <iostream>
#include <mutex>
#include <string>
#include <thread>
#include <vector>

// Educational experiment: the "unsafe" mode intentionally contains a data race.
// The unsafe mode has undefined behavior and must never be used in production.
constexpr int LIST_NUMBER = 15; // Change if your actual number on the class roster differs.
constexpr int ITERATIONS = LIST_NUMBER * 100000;
int unsafeCounter = 0;
int mutexCounter = 0;
std::atomic<int> atomicCounter{0};
std::mutex counterMutex;

void incrementUnsafe() {
    for (int i = 0; i < ITERATIONS; ++i) { ++unsafeCounter; }
}
void incrementMutex() {
    for (int i = 0; i < ITERATIONS; ++i) {
        std::lock_guard<std::mutex> lock(counterMutex);
        ++mutexCounter;
    }
}
void incrementAtomic() {
    for (int i = 0; i < ITERATIONS; ++i) { atomicCounter.fetch_add(1, std::memory_order_relaxed); }
}
int main(int argc, char** argv) {
    std::string mode = argc > 1 ? argv[1] : "all";
    if (mode != "all" && mode != "unsafe" && mode != "mutex" && mode != "atomic") {
        std::cerr << "Usage: part1_race [all|unsafe|mutex|atomic]\n";
        return 1;
    }
    const int expected = ITERATIONS * 2;
    for (const std::string& variant : {"unsafe", "mutex", "atomic"}) {
        if (mode != "all" && mode != variant) continue;
        const int runs = variant == "unsafe" ? 10 : 5;
        int mismatch = 0;
        double totalMs = 0.0;
        for (int trial = 1; trial <= runs; ++trial) {
            unsafeCounter = 0; mutexCounter = 0; atomicCounter.store(0);
            const auto start = std::chrono::steady_clock::now();
            if (variant == "unsafe") {
                std::thread a(incrementUnsafe), b(incrementUnsafe); a.join(); b.join();
            } else if (variant == "mutex") {
                std::thread a(incrementMutex), b(incrementMutex); a.join(); b.join();
            } else {
                std::thread a(incrementAtomic), b(incrementAtomic); a.join(); b.join();
            }
            const auto end = std::chrono::steady_clock::now();
            const double ms = std::chrono::duration<double, std::milli>(end - start).count();
            totalMs += ms;
            const int actual = variant == "unsafe" ? unsafeCounter :
                (variant == "mutex" ? mutexCounter : atomicCounter.load());
            mismatch += (actual != expected);
            std::cout << variant << ",trial=" << trial << ",expected=" << expected
                      << ",actual=" << actual << ",ms=" << ms << '\n';
        }
        std::cout << "SUMMARY," << variant << ",mismatches=" << mismatch
                  << ",runs=" << runs << ",avg_ms=" << totalMs / runs << '\n';
    }
}
