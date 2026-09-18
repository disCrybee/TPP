#pragma once

#include <memory>
#include <string>
#include <vector>

struct Point
{
    double x;
    double y;
};

struct InputData
{
    std::vector<Point> points;
};

struct Result
{
    std::string method;

    // Для А: {a0, a1}.
    // Для Б: {a0, a1, a2}.
    std::vector<double> coefficients;

    double mse;
    double rmse;
    double timeMicroseconds;
};

std::unique_ptr<Result> calculateA(
    std::shared_ptr<const InputData> data);

std::unique_ptr<Result> calculateB(
    std::shared_ptr<const InputData> data);