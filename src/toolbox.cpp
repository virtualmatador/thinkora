#include <cmath>
#include <numbers>

#include "toolbox.h"

std::array<std::array<int, 2>, 2> regionize(
    const std::array<std::array<int, 2>, 2>& frame)
{
    auto divide = [&](const int& position)
    {
        return position / tile_size_ - (position % tile_size_ < 0 ? 1 : 0);
    };
    return
    {
        divide(frame[0][0]),
        divide(frame[0][1]),
        divide(frame[1][0]),
        divide(frame[1][1]),
    };
}

std::array<std::array<int, 2>, 2> make_square(
    const std::array<std::array<int, 2>, 2>& frame)
{
    int d = (frame[1][0] - frame[0][0]) - (frame[1][1] - frame[0][1]);
    if (d < 0)
    {
        return
        {
            frame[0][0] + d / 2,
            frame[0][1],
            frame[1][0] - d / 2,
            frame[1][1],
        };
    }
    else
    {
        return
        {
            frame[0][0],
            frame[0][1] - d / 2,
            frame[1][0],
            frame[1][1] + d / 2,
        };
    }    
}

bool check_touch(const std::array<std::array<int, 2>, 2>& first,
    const std::array<std::array<int, 2>, 2>& second)
{
    if ((first[0][0] > second[1][0] || second[0][0] > first[1][0]) ||
        (first[0][1] > second[1][1] || second[0][1] > first[1][1]))
    {
        return false;
    }
    else
    {
        return true;
    }
}

std::array<int, 2> apply_zoom(const std::array<int, 2>& point,
    const int& zoom_delta)
{
    std::array<int, 2> dest;
    if (zoom_delta > 0)
    {
        dest[0] = point[0] << zoom_delta;
        dest[1] = point[1] << zoom_delta;
    }
    else if (zoom_delta < 0)
    {
        dest[0] = point[0] >> -zoom_delta;
        dest[1] = point[1] >> -zoom_delta;
    }
    else
    {
        dest = point;
    }
    return dest;
}

std::array<int, 2> transform(const std::array<int, 2>& point,
    const int& zoom_delta, const std::array<int, 2>& pad)
{
    std::array<int, 2> transformed = apply_zoom(point, zoom_delta);
        transformed[0] -= pad[0];
        transformed[1] -= pad[1];
    return transformed;
}

double get_diameter(const std::array<std::array<int, 2>, 2>& frame)
{
    return std::pow(
        std::pow(frame[1][0] - frame[0][0], 2) + 
        std::pow(frame[1][1] - frame[0][1], 2), 0.5);
}

int get_area(const std::array<std::array<int, 2>, 2>& frame)
{
    return (frame[1][0] - frame[0][0]) * (frame[1][1] - frame[0][1]);
}

double get_distance(const std::array<int, 2>& point1,
    const std::array<int, 2>& point2)
{
    return std::pow(std::pow(point1[0] - point2[0], 2) +
        std::pow(point1[1] - point2[1], 2), 0.5);
}

double get_angle(const std::array<int, 2>& point1,
    const std::array<int, 2>& point2, const std::array<int, 2>& point3,
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

double get_angle(const std::array<int, 2>& vector)
{
    return std::atan2(double(vector[1]), double(vector[0])) * 180.0 / std::numbers::pi;
}

int get_rotation(const int& first_angle, const int& second_angle)
{
    int r = second_angle - first_angle;
    if (r < -180)
    {
        r += 360;
    }
    else if (r > 180)
    {
        r -= 360;
    }
    return r;
}

std::array<int, 2> get_center(const std::array<std::array<int, 2>, 2>& frame)
{
    return {(frame[0][0] + frame[1][0]) / 2, (frame[0][1] + frame[1][1]) / 2};
}

void extend_frame(std::array<std::array<int, 2>, 2>& frame,
    const std::array<int, 2>& point)
{
    if (frame[0][0] > point[0])
    {
        frame[0][0] = point[0];
    }
    if (frame[0][1] > point[1])
    {
        frame[0][1] = point[1];
    }
    if (frame[1][0] < point[0] + 1)
    {
        frame[1][0] = point[0] + 1;
    }
    if (frame[1][1] < point[1] + 1)
    {
        frame[1][1] = point[1] + 1;
    }
}

std::array<std::array<int, 2>, 2> initialize_frame(
    const std::array<int, 2>& point1, const std::array<int, 2>& point2)
{
    return
    {
        std::min(point1[0], point2[0]),
        std::min(point1[1], point2[1]),
        std::max(point1[0], point2[0]) + 1,
        std::max(point1[1], point2[1]) + 1,
    };
}
