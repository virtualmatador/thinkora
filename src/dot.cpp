#include "toolbox.h"

#include "dot.h"

void Dot::set_dot(const Point& point)
{
    point_ = point;
    frame_ =
    {
        point_,
        point_,
    };
}

Shape::Type Dot::get_type() const
{
    return Type::DOT;
}

void Dot::draw_details(const Cairo::RefPtr<Cairo::Context>& cr,
        const int& zoom_delta, const Point& pad) const
{
    auto point = transform(point_, zoom_delta, pad);
    cr->set_line_cap(Cairo::LineCap::LINE_CAP_ROUND);
    cr->move_to(point[0], point[1]);
    cr->line_to(point[0], point[1]);
    cr->stroke();
}

void Dot::write_dtails(std::ostream& os) const
{
    os << point_[0] << ' ' << point_[1] << std::endl;
}

void Dot::read_details(std::istream& is)
{
    is >> point_[0] >> point_[1];
}
