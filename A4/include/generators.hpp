#pragma once
#include <stack>
#include <vector>
#include "glm/glm.hpp"

const int WIDTH = 75;
const int HEIGHT = 75;

inline std::vector<std::pair<glm::vec2, glm::vec2>> generateMaze() {
    std::vector<std::vector<bool>> grid(HEIGHT, std::vector<bool>(WIDTH, true));
    std::vector<std::pair<glm::vec2, glm::vec2>> walls;

    auto inBounds = [](int x, int y) {
        return x > 0 && x < WIDTH && y > 0 && y < HEIGHT;
        };

    std::stack<std::pair<int, int>> stack;
    int startX = (rand() % (WIDTH / 2)) * 2;
    int startY = (rand() % (HEIGHT / 2)) * 2;

    stack.push({ startX, startY });
    grid[startY][startX] = false;

    while (!stack.empty()) {
        auto [x, y] = stack.top();
        stack.pop();

        std::vector<std::pair<int, int>> directions = { {0, -2}, {0, 2}, {-2, 0}, {2, 0} };
        std::shuffle(directions.begin(), directions.end(), std::mt19937{ std::random_device{}() });

        for (const auto& dir : directions) {
            int nx = x + dir.first, ny = y + dir.second;

            if (inBounds(nx, ny) && grid[ny][nx]) {
                walls.emplace_back(
                    glm::vec2(x * 25 - 950, y * 25 - 950),
                    glm::vec2((x + dir.first / 2) * 25 - 950, (y + dir.second / 2) * 25 - 950)
                );
                grid[y + dir.second / 2][x + dir.first / 2] = false;
                grid[ny][nx] = false;
                stack.push({ nx, ny });
            }
        }
    }

    for (int y = 0; y < HEIGHT; ++y) {
        for (int x = 0; x < WIDTH; ++x) {
            if (grid[y][x]) {
                walls.emplace_back(
                    glm::vec2(x * 25 - 950, y * 25 - 950),
                    glm::vec2(x * 25 - 950 + 25, y * 25 - 950)
                );
            }
        }
    }

    return walls;
}