#include <array>
#include <cstddef>

std::array<std::array<int, 2>, 2> regionize(
    const std::array<std::array<int, 2>, 2>& frame,
    const int& tile_size);

std::array<int, 2> zoom(const std::array<int, 2>& point, const int& zoom_delta);

std::array<int, 2> transform(const std::array<int, 2>& point,
    const int& zoom_delta, const std::array<int, 2>& pad);
