#include "circle.h"

void Circle::draw_points(const Cairo::RefPtr<Cairo::Context>& cr,
    const std::vector<std::array<int, 2>>& points) const
{
    cr->set_source_rgba(color_[0], color_[1], color_[2], color_[3]);
    cr->arc((points[0][0] + points[1][0]) / 2.0, (points[0][1] + points[1][1]) / 2.0,
        (points[1][0] - points[0][0]) / 2.0, 0, 2 * M_PI);
    cr->stroke();
}
