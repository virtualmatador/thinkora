#include "toolbox.h"

#include "job.h"

std::vector<Pattern> Job::shape_patterns_;
std::map<std::string, std::vector<Pattern>> Job::char_patterns_;

Job::Job(int zoom, std::array<std::array<int, 2>, 2> frame, int line_width,
    Gdk::RGBA color, Shape::Style style)
    : zoom_{zoom}
    , frame_{frame}
    , line_width_{line_width}
    , color_{color}
    , style_{style}
{
}

Job::~Job()
{
}

void Job::reset_outer_frame()
{
    outer_frame_ = frame_;
}

const std::array<std::array<int, 2>, 2>& Job::get_frame() const
{
    return frame_;
}

const int& Job::get_zoom() const
{
    return zoom_;
}

bool Job::match_style(const Shape* shape) const
{
    return shape->get_line_width() == line_width_ &&
        shape->get_color() == color_ && shape->get_style() == style_;
}

void Job::inflate(const std::array<std::array<int, 2UL>, 2UL> frame)
{
    extend_frame(outer_frame_, frame[0]);
    extend_frame(outer_frame_, frame[1]);
}

void Job::set_sketches(std::vector<Sketch>&& sketches)
{
    sketches_ = std::move(sketches);
}

void Job::process()
{
    elements_.clear();
    for (auto& sketch: sketches_)
    {
        elements_.emplace_back(simplify(sketch));
    }
    choice_ = std::size_t(-1);
    match();
}

std::vector<Convex> Job::simplify(Sketch& sketch)
{
    auto& points = sketch.get_points();
    double tolerance = get_diameter(sketch.get_frame()) / 24.0;
    std::vector<std::tuple<double, double, std::size_t>> redondents;
    do
    {
        redondents.clear();
        for (std::size_t i = 2; i < points.size(); ++i)
        {
            double len1, len2;
            auto angle = get_angle(points[i - 2], points[i - 1], points[i],
                &len1, &len2);
            if (std::pow(std::cos(angle) + 1.0, 0.6) * std::min(len1, len2) < tolerance)
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
    return Convex::extract(points, outer_frame_);
}

void Job::match()
{
    pattern_ = nullptr;
    double min_difference = Convex::treshold_;
    for (const auto& [language, patterns]: char_patterns_)
    {
        for (const auto& pattern: patterns)
        {
            double difference = pattern.match(elements_);
            if (min_difference > difference)
            {
                min_difference = difference;
                pattern_ = &pattern;
            }
        }
    }
    for (const auto& pattern: shape_patterns_)
    {
        double difference = pattern.match(elements_);
        if (min_difference > difference)
        {
            min_difference = difference;
            pattern_ = &pattern;
        }
    }
}
