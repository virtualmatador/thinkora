#include "toolbox.h"

std::array<std::array<int, 2>, 2> regionize(
    const std::array<std::array<int, 2>, 2>& frame,
    const int& tile_size)
{
    auto divide = [&](const int& position)
    {
        return position / tile_size - (position % tile_size < 0 ? 1 : 0);
    };
    return
    {
        divide(frame[0][0]),
        divide(frame[0][1]),
        divide(frame[1][0]),
        divide(frame[1][1]),
    };
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
