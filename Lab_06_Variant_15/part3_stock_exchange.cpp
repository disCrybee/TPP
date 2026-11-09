#include <algorithm>
#include <chrono>
#include <condition_variable>
#include <iostream>
#include <mutex>
#include <queue>
#include <string>
#include <thread>
#include <vector>

struct Order { int id; bool buy; int price; int quantity; };
std::queue<Order> queueOrders;
std::mutex queueMutex, bookMutex, outputMutex;
std::condition_variable condition;
bool finished = false;
std::vector<Order> buys, sells;
int processed = 0, matchedQuantity = 0;
constexpr int ORDER_COUNT = 12;
int processDelay = 60;
bool notifyEach = true;
void log(const std::string& text) { std::lock_guard<std::mutex> l(outputMutex); std::cout << text << '\n'; }
void producer() {
    for (int i = 1; i <= ORDER_COUNT; ++i) {
        Order o{i, (i % 2 == 1), 100 + (i % 3), 2 + (i % 4)};
        {
            std::lock_guard<std::mutex> l(queueMutex);
            queueOrders.push(o);
        }
        log("Created #" + std::to_string(o.id) + " " + (o.buy ? "BUY" : "SELL") + " price=" + std::to_string(o.price) + " qty=" + std::to_string(o.quantity));
        if (notifyEach) condition.notify_one();
        std::this_thread::sleep_for(std::chrono::milliseconds(20));
    }
    {
        std::lock_guard<std::mutex> l(queueMutex);
        finished = true;
    }
    condition.notify_all();
}
void match(Order incoming) {
    std::lock_guard<std::mutex> l(bookMutex);
    auto& opposite = incoming.buy ? sells : buys;
    while (incoming.quantity > 0) {
        auto it = std::find_if(opposite.begin(), opposite.end(), [&](const Order& old) {
            return incoming.buy ? incoming.price >= old.price : incoming.price <= old.price;
        });
        if (it == opposite.end()) break;
        const int traded = std::min(incoming.quantity, it->quantity);
        incoming.quantity -= traded;
        it->quantity -= traded;
        matchedQuantity += traded;
        log("MATCH: #" + std::to_string(incoming.id) + " <-> #" + std::to_string(it->id) + " qty=" + std::to_string(traded));
        if (it->quantity == 0) opposite.erase(it);
    }
    if (incoming.quantity > 0) (incoming.buy ? buys : sells).push_back(incoming);
    ++processed;
}
void consumer(int id) {
    while (true) {
        Order o;
        {
            std::unique_lock<std::mutex> l(queueMutex);
            condition.wait(l, [] { return finished || !queueOrders.empty(); });
            if (queueOrders.empty() && finished) break;
            o = queueOrders.front(); queueOrders.pop();
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(processDelay));
        match(o);
        log("Consumer " + std::to_string(id) + " processed #" + std::to_string(o.id));
    }
    log("Consumer " + std::to_string(id) + " finished");
}
int main(int argc, char** argv) {
    int consumers = argc > 1 ? std::stoi(argv[1]) : 2;
    processDelay = argc > 2 ? std::stoi(argv[2]) : 60;
    notifyEach = !(argc > 3 && std::string(argv[3]) == "no-notify");
    if (consumers < 1 || consumers > 16 || processDelay < 0) return 1;
    const auto start = std::chrono::steady_clock::now();
    std::thread p(producer);
    std::vector<std::thread> workers;
    for (int i = 1; i <= consumers; ++i) workers.emplace_back(consumer, i);
    p.join(); for (auto& t : workers) t.join();
    const double ms = std::chrono::duration<double, std::milli>(std::chrono::steady_clock::now() - start).count();
    int openQty = 0;
    for (const auto& o : buys) openQty += o.quantity;
    for (const auto& o : sells) openQty += o.quantity;
    log("SUMMARY processed=" + std::to_string(processed) + " matched_units=" + std::to_string(matchedQuantity) +
        " open_orders=" + std::to_string(buys.size() + sells.size()) +
        " open_units=" + std::to_string(openQty) + " ms=" + std::to_string(ms));
    return processed == ORDER_COUNT ? 0 : 2;
}
