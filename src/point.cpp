#include "toolbox.h"

#include "point.h"

void Point::set_point(const std::array<int, 2>& point)
{
    point_ = point;
    frame_ =
    {
        point_,
        point_,
    };
}

Shape::Type Point::get_type() const
{
    return Type::POINT;
}

void Point::draw_details(const Cairo::RefPtr<Cairo::Context>& cr,
        const int& zoom_delta, const std::array<int, 2>& pad) const
{
    auto point = transform(point_, zoom_delta, pad);
    cr->set_line_cap(Cairo::LineCap::LINE_CAP_ROUND);
    cr->move_to(point[0], point[1]);
    cr->line_to(point[0], point[1]);
    cr->stroke();
}

void Point::write_dtails(std::ostream& os) const
{
    os << point_[0] << ' ' << point_[1] << std::endl;
}

void Point::read_details(std::istream& is)
{
    is >> point_[0] >> point_[1];
}
