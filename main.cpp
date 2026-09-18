#include "shared_types.h"

#include <exception>
#include <iomanip>
#include <iostream>

int main()
{
    try
    {
        const auto data = std::make_shared<const InputData>(InputData{
            {
                {0.0, 1.0},
                {1.0, 2.0},
                {2.0, 5.0},
                {3.0, 10.0},
                {4.0, 17.0}
            }
            });

        std::cout << std::fixed << std::setprecision(6);
        std::cout << "Variant 7. Approximation\n";

        auto resultA = calculateA(data);

        const auto& [
            methodA,
            coefficientsA,
            mseA,
            rmseA,
            timeA
        ] = *resultA;

        std::cout
            << "Method: " << methodA << '\n'
            << "a0 = " << coefficientsA.at(0) << '\n'
            << "a1 = " << coefficientsA.at(1) << '\n'
            << "MSE = " << mseA << '\n'
            << "RMSE = " << rmseA << '\n'
            << "Time (us) = " << timeA << '\n';
    }
    catch (const std::exception& error)
    {
        std::cerr << "Error: " << error.what() << '\n';
        return 1;
    }

    return 0;
}