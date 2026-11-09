#include <exception>
#include <iostream>
#include <string>
int main() {
    for (int choice = 0; choice < 2; ++choice) {
        try {
            if (choice == 0) throw 42;
            throw std::string("string exception");
        } catch (const std::exception&) {
            std::cout << "Caught std::exception\n";
        } catch (...) {
            std::cout << "Caught via catch(...): " << (choice == 0 ? "int" : "std::string") << '\n';
        }
    }
}