#include "toolbox.h"

#include "polyline.h"

void Polyline::add_point(const std::array<int, 2>& point)
{
    points_.emplace_back(point);
}

Shape::Type Polyline::get_type() const
{
    return Type::POLYLINE;
}

void Polyline::set_frame()
{
    frame_[0] = {std::numeric_limits<int>::max(),
        std::numeric_limits<int>::max()};
    frame_[1] = {std::numeric_limits<int>::min(),
        std::numeric_limits<int>::min()};;
    for (const auto& point: points_)
    {
        frame_[0][0] = std::min(frame_[0][0], point[0]);
        frame_[0][1] = std::min(frame_[0][1], point[1]);
        frame_[1][0] = std::max(frame_[1][0], point[0]);
        frame_[1][1] = std::max(frame_[1][1], point[1]);
    }
}

void Polyline::draw_details(const Cairo::RefPtr<Cairo::Context>& cr,
        const int& zoom_delta, const std::array<int, 2>& pad) const
{
    std::vector<std::array<int, 2>> points;
    for (const auto& point: points_)
    {
        points.emplace_back(transform(point, zoom_delta, pad));
    }
    cr->move_to(points[0][0], points[0][1]);
    for (int i = 0; i < points.size(); ++i)
    {
        cr->line_to(points[i][0], points[i][1]);
    }
    cr->stroke();
}

void Polyline::write_dtails(std::ostream& os) const
{
    os << points_.size() << std::endl;
    for (const auto& point: points_)
    {
        os << point[0] << ' ' << point[1] << std::endl;
    }
}

void Polyline::read_details(std::istream& is)
{
    std::size_t size;
    is >> size;
    for (std::size_t i = 0; i < size; ++i)
    {
        int x, y;
        is >> x >> y;
        points_.push_back({x, y});
    }
}
