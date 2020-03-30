#include "toolbox.h"

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
