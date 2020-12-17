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
    // TODO simplify based on min dist
    double tolerance = get_distance_point(frame_[0], frame_[1]) / 48.0;
    std::vector<std::tuple<double, double, std::size_t>> redondents;
    std::vector<Point> points = points_;
    do
    {
        redondents.clear();
        for (std::size_t i = 2; i < points.size(); ++i)
        {
            double len1, len2;
            auto angle = get_angle(points[i - 2], points[i - 1], points[i],
                &len1, &len2);
            if (std::pow(std::cos(angle) + 1.0, 0.6) * std::min(len1, len2) <
                tolerance)
            {
                redondents.emplace_back(angle, len1 * len2, i - 1);
                ++i;
            }
        }
        std::sort(redondents.begin(), redondents.end(), [](auto& a, auto&b)
        {
            if (std::get<0>(a) == std::get<0>(b))
            {
                return std::get<1>(a) < std::get<1>(b);
            }
            return std::get<0>(a) > std::get<0>(b);
        });
        redondents.resize((redondents.size() + 1) / 2);
        std::sort(redondents.begin(), redondents.end(), [](auto& a, auto&b)
        {
            return std::get<2>(a) > std::get<2>(b);
        });
        for (const auto& redondent: redondents)
        {
            points.erase(points.begin() + std::get<2>(redondent));
        }
    } while (redondents.size() > 0);
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
