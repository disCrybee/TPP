#include <chrono>
#include <cmath>
#include <future>
#include <iomanip>
#include <iostream>
#include <string>
#include <vector>

double partialPi(long long first, long long last, long long intervals) {
    const double h = 1.0 / static_cast<double>(intervals);
    double sum = 0;
    for (long long i = first; i < last; ++i) {
        const double x = (static_cast<double>(i) + 0.5) * h;
        sum += 4.0 / (1.0 + x * x);
    }
    return sum * h;
}
double sequential(long long n) { return partialPi(0, n, n); }
double parallel(long long n, int tasks) {
    std::vector<std::future<double>> futures;
    futures.reserve(tasks);
    for (int t = 0; t < tasks; ++t) {
        long long begin = n * t / tasks, end = n * (t + 1) / tasks;
        futures.push_back(std::async(std::launch::async, partialPi, begin, end, n));
    }
    double result = 0;
    for (auto& f : futures) result += f.get();
    return result;
}
int main(int argc, char** argv) {
    const int repeats = argc > 1 ? std::stoi(argv[1]) : 3;
    volatile double sink = 0.0;
    std::cout << std::setprecision(12);
    for (long long n : {1000000LL, 5000000LL, 20000000LL}) {
        for (int tasks : {2, 4, 8}) {
            double ts = 0, tp = 0, a = 0, b = 0;
            for (int r = 0; r < repeats; ++r) {
                auto s = std::chrono::steady_clock::now();
                a = sequential(n);
                auto m = std::chrono::steady_clock::now();
                b = parallel(n, tasks);
                auto e = std::chrono::steady_clock::now();
                ts += std::chrono::duration<double, std::milli>(m-s).count();
                tp += std::chrono::duration<double, std::milli>(e-m).count();
                sink = a + b;
            }
            ts /= repeats; tp /= repeats;
            std::cout << "RESULT,n=" << n << ",tasks=" << tasks << ",seq_ms=" << ts
                      << ",par_ms=" << tp << ",speedup=" << ts/tp << ",efficiency=" << ts/tp/tasks
                      << ",seq_pi=" << a << ",par_pi=" << b << ",diff=" << std::fabs(a-b) << '\n';
            if (std::fabs(a-b) > 1e-8) return 2;
        }
    }
    (void)sink;
}
