#include <iostream>
#include <format>
#include <array>
#include <vector>
#include <map>
#include <utility>
#include <ranges>
#include <algorithm>
#include <random>
#include <cmath>

int main()
{
    constexpr int rows = 80,
                  columns = 120,
                  pointCount = 100,
                  k = 10,
                  minX = 0,
                  maxX = columns - 1,
                  minY = 0,
                  maxY = rows - 1;

    std::random_device rd;
    std::uniform_int_distribution<int> xDist(minX, maxX),
                                       yDist(minY, maxY);

    using coordinates = std::pair<int, int>;

    std::array<coordinates, pointCount> points{};
    for (auto& [xRef, yRef] : points)
    {
        xRef = xDist(rd);
        yRef = yDist(rd);
    }

    auto distance = [](coordinates lArg, coordinates rArg)
    {
        const auto [x1, y1] = lArg;
        const auto [x2, y2] = rArg;

        return std::sqrt((x1 - x2) * (x1 - x2) + (y1 - y2) * (y1 - y2));
    };

    std::map<coordinates, std::vector<coordinates>> nearestNeighbors;
    for (const auto point : points)
    {
        std::vector<coordinates> sortedNeighbors;
        sortedNeighbors.reserve(pointCount - 1);

        for (const auto otherPoint : points)
            if (point != otherPoint)
                sortedNeighbors.push_back(otherPoint);

        std::ranges::sort(sortedNeighbors, [&](coordinates lArg, coordinates rArg)
        {
            return distance(point, lArg) < distance(point, rArg);
        });

        sortedNeighbors.erase(sortedNeighbors.begin() + k, sortedNeighbors.end());
        nearestNeighbors[point] = std::move(sortedNeighbors);
    }

    std::map<coordinates, double> pointDensities;
    for (const auto point : points)
    {
        double averageReachability = 0;
        for (const auto neighbor : nearestNeighbors[point])
        {
            const double localDistance = distance(point, neighbor),
                         kthDistance = distance(neighbor, nearestNeighbors[neighbor].back());

            const double reachabilityDistance = std::max(localDistance, kthDistance);
            averageReachability += reachabilityDistance / k;
        }

        const double localDensity = 1 / averageReachability;
        pointDensities[point] = localDensity;
    }

    auto moveCursor = [](int row, int column)
    {
        std::cout << std::format("\033[{};{}H", row + 1, column + 1);
    };

    auto changeColor = [](int red, int green, int blue)
    {
        std::cout << std::format("\033[38;2;{};{};{}m", red, green, blue);
    };

    std::cout << "\033[?25l";
    std::cout << "\033[2J";

    for (const auto [point, neighbors] : nearestNeighbors)
    {
        double localOutlierFactor = 0;

        for (const auto neighbor : neighbors)
            localOutlierFactor += pointDensities[neighbor] / pointDensities[point] / k;

        const auto [column, y] = point;
        const int row = rows - 1 - y;
        moveCursor(row, column);

        if (localOutlierFactor - 1 >= 0.15)
            changeColor(255, 0, 0);

        std::cout << 'o';
        std::cout << "\033[0m";
    }

    moveCursor(rows, 0);
    std::cout << "\033[?25h";
}