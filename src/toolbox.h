#ifndef TOOLBOX_H
#define TOOLBOX_H

#include <array>
#include <cstddef>

const int tile_size_ = 512;
const int zoom_limit_ = 64;
const int position_limit_ = 1000000000;
const int draw_level_limit_ = 6;
const int width_limit_ = 4;

std::array<std::array<int, 2>, 2> regionize(
    const std::array<std::array<int, 2>, 2>& frame);

std::array<std::array<int, 2>, 2> make_square(
    const std::array<std::array<int, 2>, 2>& frame);

bool check_touch(const std::array<std::array<int, 2>, 2>& first,
    const std::array<std::array<int, 2>, 2>& second);

std::array<int, 2> apply_zoom(const std::array<int, 2>& point,
    const int& zoom_delta);

std::array<int, 2> transform(const std::array<int, 2>& point,
    const int& zoom_delta, const std::array<int, 2>& pad);

double get_diameter(const std::array<std::array<int, 2>, 2>& frame);

int get_area(const std::array<std::array<int, 2>, 2>& frame);

double get_distance(const std::array<int, 2>& point1,
    const std::array<int, 2>& point2);

double get_angle(const std::array<int, 2>& point1,
    const std::array<int, 2>& point2, const std::array<int, 2>& point3,
    double* out_len1, double* out_len2);

double get_angle(const std::array<int, 2>& vector);

int get_rotation(const int& first_angle, const int& second_angle);

std::array<int, 2> get_center(const std::array<std::array<int, 2>, 2>& frame);

void extend_frame(std::array<std::array<int, 2>, 2>& frame,
    const std::array<int, 2>& point);

#endif // TOOLBOX_H
