#include "shared_types.h"

#include <algorithm>
#include <array>
#include <chrono>
#include <cmath>
#include <limits>
#include <numeric>
#include <stdexcept>

namespace
{
using Matrix = std::array<std::array<long double, 4>, 3>;

// Метод Гаусса з вибором найбільшого за модулем елемента у стовпці.
std::array<long double, 3> solveNormalEquations(Matrix matrix)
{
    const long double tolerance =
        16 * std::numeric_limits<long double>::epsilon() * matrix[0][0];

    for (int col = 0; col < 3; ++col)
    {
        int pivot = col;
        for (int row = col + 1; row < 3; ++row)
        {
            if (std::abs(matrix[row][col]) > std::abs(matrix[pivot][col]))
                pivot = row;
        }
        if (std::abs(matrix[pivot][col]) <= tolerance)
            throw std::invalid_argument("Потрібні щонайменше три чисельно розрізнювані значення x.");

        std::swap(matrix[col], matrix[pivot]);
        for (int row = col + 1; row < 3; ++row)
        {
            const long double factor = matrix[row][col] / matrix[col][col];
            for (int k = col + 1; k < 4; ++k)
                matrix[row][k] -= factor * matrix[col][k];
            matrix[row][col] = 0;
        }
    }

    std::array<long double, 3> solution{};
    for (int row = 2; row >= 0; --row)
    {
        long double rhs = matrix[row][3];
        for (int k = row + 1; k < 3; ++k)
            rhs -= matrix[row][k] * solution[k];
        solution[row] = rhs / matrix[row][row];
    }
    return solution;
}
}

std::unique_ptr<Result> calculateB(std::shared_ptr<const InputData> data)
{
    const auto start = std::chrono::steady_clock::now();
    if (!data || data->points.size() < 3)
        throw std::invalid_argument("Для квадратичної моделі потрібно щонайменше три точки.");

    long double minX = data->points.front().x;
    long double maxX = minX;
    for (const auto& [x, y] : data->points)
    {
        if (!std::isfinite(x) || !std::isfinite(y))
            throw std::invalid_argument("Координати точок мають бути скінченними.");
        minX = std::min(minX, static_cast<long double>(x));
        maxX = std::max(maxX, static_cast<long double>(x));
    }
    if (minX == maxX)
        throw std::invalid_argument("Усі точки мають однакове значення x.");

    // Нормуємо x до [-1, 1], щоб великі значення x не підносити до четвертого степеня.
    const long double center = std::midpoint(minX, maxX);
    const long double scale = std::max(maxX - center, center - minX);
    Matrix matrix{};
    for (const auto& [x, y] : data->points)
    {
        const long double z = (x - center) / scale;
        const std::array<long double, 3> basis{1, z, z * z};
        // Ліворуч накопичуємо XᵀX, у четвертому стовпці - Xᵀy.
        for (int row = 0; row < 3; ++row)
        {
            for (int col = 0; col < 3; ++col)
                matrix[row][col] += basis[row] * basis[col];
            matrix[row][3] += basis[row] * y;
        }
    }
    const auto [b0, b1, b2] = solveNormalEquations(matrix);

    // Перехід від b0 + b1*z + b2*z² до a0 + a1*x + a2*x².
    const long double a2 = (b2 / scale) / scale;
    const long double a1 = b1 / scale - 2 * (center * a2);
    const long double a0 = b0 - center * (a1 + center * a2);

    long double squaredError = 0;
    for (const auto& [x, y] : data->points)
    {
        const long double z = (x - center) / scale;
        const long double predicted = (b2 * z + b1) * z + b0;
        const long double residual = y - predicted;
        squaredError += residual * residual;
    }
    const long double mse = squaredError / data->points.size();
    const auto elapsed = std::chrono::steady_clock::now() - start;

    return std::make_unique<Result>(Result{
        "Quadratic least squares",
        {static_cast<double>(a0), static_cast<double>(a1), static_cast<double>(a2)},
        static_cast<double>(mse),
        static_cast<double>(std::sqrt(mse)),
        std::chrono::duration<double, std::micro>(elapsed).count()
    });
}
