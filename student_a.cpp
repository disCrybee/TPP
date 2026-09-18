#include "shared_types.h"

#include <chrono>
#include <cmath>
#include <stdexcept>

std::unique_ptr<Result> calculateA(
    std::shared_ptr<const InputData> data)
{
    const auto start = std::chrono::steady_clock::now();

    if (!data || data->points.size() < 2)
    {
        throw std::invalid_argument(
            "At least two points are required.");
    }

    // 1. Обчислюємо середні значення x та y.
    double meanX = 0.0;
    double meanY = 0.0;

    for (const auto& [x, y] : data->points)
    {
        if (!std::isfinite(x) || !std::isfinite(y))
        {
            throw std::invalid_argument(
                "Coordinates must be finite.");
        }

        meanX += x;
        meanY += y;
    }

    const double n =
        static_cast<double>(data->points.size());

    meanX /= n;
    meanY /= n;

    // 2. Обчислюємо суми для методу найменших квадратів.
    double sxx = 0.0;
    double sxy = 0.0;

    for (const auto& [x, y] : data->points)
    {
        const double dx = x - meanX;

        sxx += dx * dx;
        sxy += dx * (y - meanY);
    }

    if (sxx == 0.0)
    {
        throw std::invalid_argument(
            "At least two different x values are required.");
    }

    // 3. Знаходимо коефіцієнти y = a0 + a1*x.
    const double a1 = sxy / sxx;
    const double a0 = meanY - a1 * meanX;

    // 4. Обчислюємо похибку на вхідних точках.
    double sumSquaredErrors = 0.0;

    for (const auto& [x, y] : data->points)
    {
        const double predictedY = a0 + a1 * x;
        const double residual = y - predictedY;

        sumSquaredErrors += residual * residual;
    }

    const double mse = sumSquaredErrors / n;
    const double rmse = std::sqrt(mse);

    const auto stop = std::chrono::steady_clock::now();

    const double elapsed =
        std::chrono::duration<double, std::micro>(
            stop - start
        ).count();

    return std::make_unique<Result>(Result{
        "Linear least squares",
        {a0, a1},
        mse,
        rmse,
        elapsed
        });
}