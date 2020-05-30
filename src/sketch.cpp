#include "toolbox.h"

#include "sketch.h"

Sketch::~Sketch()
{
    // TODO set job->sketch nullptr
}

void Sketch::set_sketch()
{
    frame_ =
    {
        std::numeric_limits<int>::max(), std::numeric_limits<int>::max(),
        std::numeric_limits<int>::min(), std::numeric_limits<int>::min()
    };
}

void Sketch::add_point(const std::array<int, 2>& point)
{
    if (points_.empty() || points_.back() != point)
    {
        if (points_.size() < 2 ||
            (points_[points_.size() - 1][0] - points_[points_.size() - 2][0]) *
            (point[1] - points_[points_.size() - 1][1]) !=
            (points_[points_.size() - 1][1] - points_[points_.size() - 2][1]) *
            (point[0] - points_[points_.size() - 1][0]))
        {
            points_.emplace_back(point);
        }
        else
        {
            points_.back() = point;
        }
        extend_frame(frame_, point);
    }
}

void Sketch::set_birth(const std::chrono::steady_clock::time_point& birth)
{
    birth_ = birth;
}

std::vector<std::array<int, 2>>& Sketch::get_points()
{
    return points_;
}

const std::chrono::steady_clock::time_point& Sketch::get_birth() const
{
    return birth_;
}

Shape::Type Sketch::get_type() const
{
    return Type::SKETCH;
}

void Sketch::draw_details(const Cairo::RefPtr<Cairo::Context>& cr,
        const int& zoom_delta, const std::array<int, 2>& pad) const
{
    std::vector<std::array<int, 2>> points;
    for (const auto& point: points_)
    {
        points.emplace_back(transform(point, zoom_delta, pad));
    }
    if (points.size() == 1)
    {
        cr->set_line_cap(Cairo::LineCap::LINE_CAP_ROUND);
    }
    cr->move_to(points[0][0], points[0][1]);
    for (int i = 0; i < points.size(); ++i)
    {
        cr->line_to(points[i][0], points[i][1]);
    }
    cr->stroke();
}

void Sketch::write_dtails(std::ostream& os) const
{
    os << birth_.time_since_epoch().count() << std::endl;
    os << points_.size() << std::endl;
    for (const auto& point: points_)
    {
        os << point[0] << ' ' << point[1] << std::endl;
    }
}

void Sketch::read_details(std::istream& is)
{
    std::chrono::steady_clock::rep birth_rep;
    is >> birth_rep;
    birth_ = std::chrono::steady_clock::time_point(
        std::chrono::nanoseconds(birth_rep));
    std::size_t size;
    is >> size;
    for (std::size_t i = 0; i < size; ++i)
    {
        int x, y;
        is >> x >> y;
        points_.push_back({x, y});
    }
}
