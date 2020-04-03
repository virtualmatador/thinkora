#include "line.h"

void Line::draw_points(const Cairo::RefPtr<Cairo::Context>& cr,
    const std::vector<std::array<int, 2>>& points) const
{
    cr->set_line_cap(Cairo::LineCap::LINE_CAP_ROUND);
    cr->move_to(points[0][0], points[0][1]);
    for (int i = 0; i < points.size(); ++i)
    {
        cr->line_to(points[i][0], points[i][1]);
    }
}

Shape::Type Line::get_type() const
{
    return Type::LINE;
}
