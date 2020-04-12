#include <cmath>

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

std::array<std::array<int, 2>, 2> square(
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

bool touch(const std::array<std::array<int, 2>, 2>& first,
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

std::array<int, 2> zoom(const std::array<int, 2>& point, const int& zoom_delta)
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
    std::array<int, 2> transformed = zoom(point, zoom_delta);
        transformed[0] -= pad[0];
        transformed[1] -= pad[1];
    return transformed;
}

double diameter(const std::array<std::array<int, 2>, 2>& frame)
{
    return std::pow(
        std::pow(frame[1][0] - frame[0][0], 2) + 
        std::pow(frame[1][1] - frame[0][1], 2), 0.5);
}