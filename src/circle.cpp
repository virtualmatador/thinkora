#include "toolbox.h"

#include "circle.h"

void Circle::set_circle(const std::array<int, 2>& center, const int& radius)
{
    circle_ = {center, {center[0] + radius, center[1] + radius}};
}

Shape::Type Circle::get_type() const
{
    return Type::CIRCLE;
}

void Circle::set_frame()
{
    frame_ =
    {
        2 * circle_[0][0] - circle_[1][0], 2 * circle_[0][1] - circle_[1][1],
        circle_[1]
    };
}

void Circle::draw_details(const Cairo::RefPtr<Cairo::Context>& cr,
        const int& zoom_delta, const std::array<int, 2>& pad) const
{
    std::vector<std::array<int, 2>> points;
    for (const auto& point: circle_)
    {
        points.emplace_back(transform(point, zoom_delta, pad));
    }
    cr->arc(points[0][0], points[0][1], points[1][0] - points[0][0], 0, 2 * M_PI);
    cr->stroke();
}

void Circle::write_dtails(std::ostream& os) const
{
    os << circle_[0][0] << ' ' << circle_[0][1] << ' ' <<
        circle_[1][0] << ' ' << circle_[1][1] << std::endl;
}

void Circle::read_details(std::istream& is)
{
    is >> circle_[0][0] >> circle_[0][1] >> circle_[1][0] >> circle_[1][1];
}
