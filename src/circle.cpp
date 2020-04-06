#include "circle.h"

std::array<std::array<int, 2>, 2> Circle::draw_points(const Cairo::RefPtr<Cairo::Context>& cr,
    const std::vector<std::array<int, 2>>& points) const
{
    cr->arc((points[0][0] + points[1][0]) / 2.0, (points[0][1] + points[1][1]) / 2.0,
        (points[1][0] - points[0][0]) / 2.0, 0, 2 * M_PI);
    cr->stroke();
    return {points[0], points[1]};
}

Shape::Type Circle::get_type() const
{
    return Type::CIRCLE;
}
