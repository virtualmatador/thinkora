#include "sketch.h"

#include "pattern.h"

Pattern::Pattern(const std::string& name, const jsonio::json& pattern)
    : name_{ name }
    , frame_{ empty_frame() }
{
    for (const auto& json_point: pattern.get_array())
    {
        points_.push_back(
        {
            json_point[0].get_double(),
            json_point[1].get_double(),
        });
        extend_frame(frame_, points_.back());
    }
}

Pattern::~Pattern()
{
}

double Pattern::match(
    const std::vector<Point>& points, const Rectangle& frame) const
{
    double ratio = 
        (frame_[1][0] - frame_[0][0]) /
        (frame_[1][1] - frame_[0][1]) *
        (frame[1][1] - frame[0][1]) /
        (frame[1][0] - frame[0][0]);
    if (ratio < 2.5 && ratio > 0.4)
    {
        std::vector<Point> points;
        for (const auto& point : points)
        {
            points.push_back(
            {
                (point[0] - frame[0][0]) * (frame_[1][0] - frame_[0][0]) /
                    (frame[1][0] - frame[0][0]) + frame_[0][0],
                (point[1] - frame[0][1]) * (frame_[1][1] - frame_[0][1]) /
                    (frame[1][1] - frame[0][1]) + frame_[0][1],
            });
        }
        return (compare(points, points_) + compare(points_, points)) /
            get_distance_point(frame_[0], frame_[1]);
    }
    else
    {
        return 0.0;
    }
}

const std::string& Pattern::get_name() const
{
    return name_;
}

double Pattern::compare(const std::vector<Point>& points_a,
    const std::vector<Point>& points_b) const
{
    double total_dist = 0;
    for (const auto& pt_b : points_b)
    {
        auto dist_min = std::numeric_limits<double>::max();
        auto start = points_a.begin();
        for (auto it = std::next(start); it != points_a.end(); start = it++)
        {
            auto dist = get_distance_line(pt_b, {*start, *it});
            if (dist_min > dist)
            {
                dist_min = dist;
            }
        }
        total_dist += dist_min;
    }
    return total_dist / points_b.size();
}
