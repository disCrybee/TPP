#include <barrier>
#include <iostream>
#include <mutex>
#include <string>
#include <thread>

std::mutex mutexA, mutexB, outputMutex;
std::barrier syncPoint(2);
void log(const std::string& msg) { std::lock_guard<std::mutex> l(outputMutex); std::cout << msg << std::endl; }
void worker(int id, const std::string& mode) {
    if (mode == "unsafe") {
        if (id == 1) {
            std::lock_guard<std::mutex> a(mutexA);
            log("Thread 1: mutexA locked");
            syncPoint.arrive_and_wait();
            log("Thread 1: waiting for mutexB");
            std::lock_guard<std::mutex> b(mutexB);
            log("Thread 1: both mutexes locked");
        } else {
            std::lock_guard<std::mutex> b(mutexB);
            log("Thread 2: mutexB locked");
            syncPoint.arrive_and_wait();
            log("Thread 2: waiting for mutexA");
            std::lock_guard<std::mutex> a(mutexA);
            log("Thread 2: both mutexes locked");
        }
    } else if (mode == "scoped") {
        syncPoint.arrive_and_wait();
        std::scoped_lock both(mutexA, mutexB);
        log("Thread " + std::to_string(id) + ": both mutexes locked");
    } else {
        syncPoint.arrive_and_wait();
        std::lock_guard<std::mutex> a(mutexA);
        std::lock_guard<std::mutex> b(mutexB);
        log("Thread " + std::to_string(id) + ": both mutexes locked");
    }
}
int main(int argc, char** argv) {
    const std::string mode = argc > 1 ? argv[1] : "scoped";
    if (mode != "unsafe" && mode != "scoped" && mode != "ordered") {
        std::cerr << "Usage: part2_deadlock [unsafe|scoped|ordered]\n"; return 1;
    }
    std::thread t1(worker, 1, mode), t2(worker, 2, mode);
    t1.join(); t2.join();
    log("Program finished");
}
