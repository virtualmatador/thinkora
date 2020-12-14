#ifndef THINKORA_SRC_TOOLBOX_H
#define THINKORA_SRC_TOOLBOX_H

#include <array>
#include <cstddef>

const int zoom_limit_ = 64;
const int draw_level_limit_ = 6;
const double tile_size_ = 512.0;
const double position_limit_ = 1000000000;
const double width_limit_ = 4.0;

using Point = std::array<double, 2>;
using Rectangle = std::array<Point, 2>;

Rectangle regionize(const Rectangle& frame);
Point apply_zoom(const Point& point, const int& zoom);
Point transform(const Point& point, const int& zoom, const Point& pad);
double get_distance_point(const Point& point1, const Point& point2);
double get_distance_line(const Point& point, const Rectangle& line);
double get_angle(const Point& point1, const Point& point2, const Point& point3,
    double* out_len1, double* out_len2);
double get_angle(const Point& vector);
double get_rotation(const double& first_angle, const double& second_angle);
void extend_frame(Rectangle& frame, const Point& point);
Rectangle initialize_frame(const Point& point1, const Point& point2);
Rectangle empty_frame();

#endif // THINKORA_SRC_TOOLBOX_H
