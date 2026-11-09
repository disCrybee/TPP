# Лабораторна робота №6 — багатопотоковість та асинхронне програмування

Варіант 15. C++20.

## Завдання
1. Демонстрація race condition та виправлення через `std::mutex` і `std::atomic`.
2. Демонстрація deadlock та виправлення через `std::scoped_lock` або єдиний порядок захоплення м'ютексів.
3. Producer–Consumer: симуляція торгів на біржі; 12 заявок, спрощене зіставлення buy/sell.
4. Послідовне та паралельне обчислення числа π через `std::async` / `std::future`.

## Компіляція
Потрібен компілятор C++20. У Visual Studio обрати стандарт ISO C++20 і конфігурацію Release x64.

```bash
g++ -std=c++20 -O2 -pthread part1_race.cpp -o part1_race
g++ -std=c++20 -O2 -pthread part2_deadlock.cpp -o part2_deadlock
g++ -std=c++20 -O2 -pthread part3_stock_exchange.cpp -o part3_stock_exchange
g++ -std=c++20 -O2 -pthread part4_pi_async.cpp -o part4_pi_async
./part1_race all
./part2_deadlock scoped
./part2_deadlock ordered
./part3_stock_exchange 2 60
./part3_stock_exchange 1 60
./part3_stock_exchange 4 60
./part3_stock_exchange 2 200
./part3_stock_exchange 2 60 no-notify
./part4_pi_async 3
```

**Попередження:** `part2_deadlock unsafe` навмисно зависає — зупиніть через Ctrl+C. `part1_race unsafe` має невизначену поведінку. У частині 1 `LIST_NUMBER=15` є припущенням щодо номера за списком.

Форматовані звіти Word/PDF підготовлені окремо у чаті; ця папка містить вихідний код та інструкцію.
