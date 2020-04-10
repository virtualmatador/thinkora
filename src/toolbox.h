

#include <array>
#include <cstddef>

const int tile_size_ = 512;
const int zoom_limit_ = 64;
const int position_limit_ = 1000000000;
const int draw_level_limit_ = 6;
const int width_limit_ = 4;

std::array<std::array<int, 2>, 2> regionize(
    const std::array<std::array<int, 2>, 2>& frame);

std::array<std::array<int, 2>, 2> square(
    const std::array<std::array<int, 2>, 2>& frame);

bool touch(const std::array<std::array<int, 2>, 2>& first,
    const std::array<std::array<int, 2>, 2>& second);

std::array<int, 2> zoom(const std::array<int, 2>& point, const int& zoom_delta);

std::array<int, 2> transform(const std::array<int, 2>& point,
    const int& zoom_delta, const std::array<int, 2>& pad);
