#include <array>

std::array<int, 2> zoom(const std::array<int, 2>& point, const int& zoom_delta);
std::array<int, 2> transform(const std::array<int, 2>& point,
    const int& zoom_delta, const std::array<int, 2>& pad);
