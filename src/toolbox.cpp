#include <cmath>
#include <numbers>

#include "toolbox.h"

Rectangle regionize(const Rectangle& frame)
{
    return
    {
        std::floor(frame[0][0] / tile_size_),
        std::ceil(frame[0][1] / tile_size_),
        std::floor(frame[1][0] / tile_size_),
        std::ceil(frame[1][1] / tile_size_),
    };
}

Point apply_zoom(const Point& point, const int& zoom)
{
    Point dest;
    if (zoom > 0)
    {
        dest[0] = point[0] * std::pow(2.0, zoom);
        dest[1] = point[1] * std::pow(2.0, zoom);
    }
    else if (zoom < 0)
    {
        dest[0] = point[0] / std::pow(2.0, -zoom);
        dest[1] = point[1] / std::pow(2.0, -zoom);
    }
    else
    {
        dest = point;
    }
    return dest;
}

Point transform(const Point& point, const int& zoom, const Point& pad)
{
    Point transformed = apply_zoom(point, zoom);
    transformed[0] -= pad[0];
    transformed[1] -= pad[1];
    return transformed;
}

double get_distance(const Point& point1, const Point& point2)
{
    return std::pow(std::pow(point1[0] - point2[0], 2.0) +
        std::pow(point1[1] - point2[1], 2.0), 0.5);
}

double get_angle(const Point& point1, const Point& point2, const Point& point3,
    double* out_len1, double* out_len2)
{
    double len1 = get_distance(point1, point2);
    if (out_len1)
    {
        *out_len1 = len1;
    }
    double len2 = get_distance(point3, point2);
    if (out_len2)
    {
        *out_len2 = len2;
    }
    double dot_product = (point1[0] - point2[0]) * (point3[0] - point2[0]) +
        (point1[1] - point2[1]) * (point3[1] - point2[1]);
    return std::acos(dot_product / (len1 * len2));
}

double get_angle(const Point& vector)
{
    return std::atan2(vector[1], vector[0]) * 180.0 / std::numbers::pi;
}

double get_rotation(const double& first_angle, const double& second_angle)
{
    double r = second_angle - first_angle;
    if (r < -180.0)
    {
        r += 360.0;
    }
    else if (r > 180.0)
    {
        r -= 360.0;
    }
    return r;
}

void extend_frame(Rectangle& frame, const Point& point)
{
    if (frame[0][0] > point[0])
    {
        frame[0][0] = point[0];
    }
    if (frame[0][1] > point[1])
    {
        frame[0][1] = point[1];
    }
    if (frame[1][0] < point[0])
    {
        frame[1][0] = point[0];
    }
    if (frame[1][1] < point[1])
    {
        frame[1][1] = point[1];
    }
}

Rectangle initialize_frame(const Point& point1, const Point& point2)
{
    return
    {
        std::min(point1[0], point2[0]),
        std::min(point1[1], point2[1]),
        std::max(point1[0], point2[0]),
        std::max(point1[1], point2[1]),
    };
}
