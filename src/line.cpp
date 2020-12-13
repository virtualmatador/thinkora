#include "toolbox.h"

#include "line.h"

void Line::set_line(const Rectangle& points)
{
    points_ = points;
    frame_ = initialize_frame(points_[0], points_[1]);
}

Shape::Type Line::get_type() const
{
    return Type::LINE;
}

void Line::draw_details(const Cairo::RefPtr<Cairo::Context>& cr,
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

void Line::write_dtails(std::ostream& os) const
{
    os <<
        points_[0][0] << ' ' <<
        points_[0][1] << ' ' <<
        points_[1][0] << ' ' <<
        points_[1][1] << std::endl;
}

void Line::read_details(std::istream& is)
{
    is >>
        points_[0][0] >>
        points_[0][1] >>
        points_[1][0] >>
        points_[1][1];
}
