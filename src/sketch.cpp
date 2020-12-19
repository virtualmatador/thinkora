#include "toolbox.h"

#include "sketch.h"

void Sketch::set_sketch(int zoom)
{
    zoom_ = zoom;
    frame_ = empty_frame();
}

void Sketch::add_point(const Point& point)
{
    if (points_.empty() || points_.back() != point)
    {
        points_.emplace_back(point);
        extend_frame(frame_, point);
    }
}

const std::vector<Point>& Sketch::get_points() const
{
    return points_;
}

const int& Sketch::get_zoom() const
{
    return zoom_;
}

std::vector<Point> Sketch::simplify() const
{
    std::vector<Point> points;
    double tolerance = std::pow(get_distance(frame_[0], frame_[1]), 0.75) / 16.0;
    double angle_max = 165.0;
    auto it = points_.begin();
    for (;;)
    {
        double old_angle;
        if (points.size() >= 3)
        {
            old_angle = get_angle(
                *points.rbegin(),
                *(points.rbegin() + 1),
                *(points.rbegin() + 2));
            if (std::abs(old_angle) > angle_max)
            {
                *(points.rbegin() + 1) = *(points.rbegin());
                points.pop_back();
                continue;
            }
        }
        if (it == points_.end())
        {
            break;
        }
        if (points.size() < 3)
        {
            points.push_back(*it++);
            continue;
        }
        double new_angle = get_angle(
            *it,
            *points.rbegin(),
            *(points.rbegin() + 1));
        if (std::abs(new_angle) > angle_max)
        {
            points.back() = *it++;
            continue;
        }
        if (old_angle * new_angle < 0.0)
        {
            Point mid
            {
                ((*it)[0] + (*(points.rbegin() + 2))[0]) / 2.0,
                ((*it)[1] + (*(points.rbegin() + 2))[1]) / 2.0,
            };
            auto length = get_distance(*it, mid);
            auto n1 = get_nearst(*(points.rbegin() + 1),
                { *it, *(points.rbegin() + 2) });
            if (get_distance(n1, *(points.rbegin() + 1)) > tolerance / 2.0 ||
                get_distance(n1, mid) > length + tolerance)
            {
                points.push_back(*it++);
                continue;
            }
            auto n2 = get_nearst(*points.rbegin(),
                { *it, *(points.rbegin() + 2) });
            if (get_distance(n2, *points.rbegin()) > tolerance / 2.0 ||
                get_distance(n2, mid) > length + tolerance)
            {
                points.push_back(*it++);
                continue;
            }
            points.pop_back();
            points.pop_back();
        }
        points.push_back(*it++);
    }
    return points;
}

Shape::Type Sketch::get_type() const
{
    return Type::SKETCH;
}

void Sketch::draw_details(const Cairo::RefPtr<Cairo::Context>& cr,
        const int& zoom_delta, const Point& pad) const
{
    std::vector<Point> points;
    for (const auto& point: points_)
    {
        points.emplace_back(transform(point, zoom_delta, pad));
    }
    if (points.size() == 1)
    {
        cr->set_line_cap(Cairo::LineCap::LINE_CAP_ROUND);
    }
    cr->move_to(points[0][0], points[0][1]);
    for (std::size_t i = 0; i < points.size(); ++i)
    {
        cr->line_to(points[i][0], points[i][1]);
    }
    cr->stroke();
}

void Sketch::write_dtails(std::ostream& os) const
{
    os << points_.size() << std::endl;
    for (const auto& point: points_)
    {
        os << point[0] << ' ' << point[1] << std::endl;
    }
}

void Sketch::read_details(std::istream& is)
{
    std::size_t size;
    is >> size;
    for (std::size_t i = 0; i < size; ++i)
    {
        Point point;
        is >> point[0] >> point[1];
        points_.emplace_back(point);
    }
}
