#include "shared_types.h"

#include <iostream>

int main()
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

    std::cout << "Variant 7. Approximation\n";
    std::cout << "Points: " << data->points.size() << '\n';

    // ALGORITHM_CALLS
    std::cout << "Algorithms are not implemented yet.\n";

    return 0;
}