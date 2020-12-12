#include "toolbox.h"

#include "sketch.h"

void Sketch::set_sketch(int zoom)
{
    zoom_ = zoom;
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
        points_.emplace_back(point);
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

const int& Sketch::get_zoom() const
{
    return zoom_;
}

std::vector<std::array<int, 2UL>> Sketch::simplify() const
{
    double tolerance = get_diameter(frame_) / 24.0;
    std::vector<std::tuple<double, double, std::size_t>> redondents;
    std::vector<std::array<int, 2UL>> points = points_;
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
/*
std::vector<Convex> Sketch::get_convexes() const
{
    std::vector<Convex> convexes;
    std::vector<std::array<int, 2UL>> points = simplify();
    for (std::size_t end = 0; end < points.size();)
    {
        if (points.size() - end == 1)
        {
            convexes.emplace_back(points[0], frame_);
            end = points.size();
        }
        else
        {
            std::array<std::array<int, 2>, 2> convex_frame =
                initialize_frame(points[end], points[end + 1]);
            if (points.size() - end == 2)
            {
                convexes.emplace_back(points, end, end + 2, 0.0, convex_frame,
                    frame_);
                end = points.size();
            }
            else
            {
                std::size_t begin = end;
                extend_frame(convex_frame, points[begin + 2]);
                double first_angle = get_angle(
                {
                    points[begin + 1][0] - points[begin][0],
                    points[begin + 1][1] - points[begin][1]
                });
                double second_angle = get_angle(
                {
                    points[begin + 2][0] - points[begin + 1][0],
                    points[begin + 2][1] - points[begin + 1][1]
                });
                int d_r = get_rotation(first_angle, second_angle);
                bool clockwise = d_r < 0;
                for (end = begin + 3; end < points.size(); ++end)
                {
                    first_angle = second_angle;
                    second_angle = get_angle(
                    {
                        points[end][0] - points[end - 1][0],
                        points[end][1] - points[end - 1][1]
                    });
                    int r = get_rotation(first_angle, second_angle);
                    if (clockwise != (r < 0))
                    {
                        break;
                    }
                    d_r += r;
                    extend_frame(convex_frame, points[end]);
                }
                convexes.emplace_back(points, begin, end, d_r, convex_frame,
                    frame_);
                if (end != points.size())
                {
                    --end;
                }
            }
        }
    }
    return convexes;
}
*/
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
        int x, y;
        is >> x >> y;
        points_.push_back({x, y});
    }
}
