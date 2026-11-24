#include "shared_types.h"

#include <exception>
#include <iomanip>
#include <iostream>

#ifdef _WIN32
#include <windows.h>
#endif

int main()
{
#ifdef _WIN32
    SetConsoleCP(CP_UTF8);
    SetConsoleOutputCP(CP_UTF8);
#endif
    try
    {
        const auto data = std::make_shared<const InputData>(InputData{
            {{0.0, 1.0}, {1.0, 2.0}, {2.0, 5.0}, {3.0, 10.0}, {4.0, 17.0}}
        });

        const auto result = calculateB(data);
        const auto& [method, coefficients, mse, rmse, timeMicroseconds] = *result;

        std::cout << "Варіант 7. Студент Б\nТочки (x, y):";
        for (const auto& [x, y] : data->points)
            std::cout << " (" << x << ", " << y << ')';

        std::cout << std::fixed << std::setprecision(6)
                  << "\nМетод: " << method
                  << "\nМодель: y = a0 + a1*x + a2*x^2"
                  << "\na0 = " << coefficients.at(0)
                  << "\na1 = " << coefficients.at(1)
                  << "\na2 = " << coefficients.at(2)
                  << "\nMSE = " << mse
                  << "\nRMSE = " << rmse
                  << "\nЧас (мкс) = " << timeMicroseconds << '\n';
    }
    catch (const std::exception& error)
    {
        std::cerr << "Помилка: " << error.what() << '\n';
        return 1;
    }
    return 0;
}
