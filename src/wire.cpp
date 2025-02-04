#include "toolbox.h"

#include "wire.h"

void Wire::set_wire(const Rectangle& points)
{
    points_ = points;
    frame_ = empty_frame();
    extend_frame(frame_, points_[0]);
    extend_frame(frame_, points_[1]);
}

Shape::Type Wire::get_type() const
{
    return Type::WIRE;
}

void Wire::draw_details(const Cairo::RefPtr<Cairo::Context>& cr,
        const int& zoom_delta, const Point& pad) const
{
    Point points[2];
    for (std::size_t i = 0; i < 2; ++i)
    {
        points[i] = transform(points_[i], zoom_delta, pad);
    }
    cr->move_to(points[0][0], points[0][1]);
    cr->line_to(points[1][0], points[1][1]);
    cr->stroke();
}

void Wire::write_dtails(std::ostream& os) const
{
    os <<
        points_[0][0] << ' ' <<
        points_[0][1] << ' ' <<
        points_[1][0] << ' ' <<
        points_[1][1] << std::endl;
}

void Wire::read_details(std::istream& is)
{
    is >>
        points_[0][0] >>
        points_[0][1] >>
        points_[1][0] >>
        points_[1][1];
}
