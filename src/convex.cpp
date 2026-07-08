#include <algorithm>
#include <iterator>
#include <limits>
#include <numbers>
#include <vector>

#include "toolbox.h"

#include "convex.h"

namespace
{
constexpr double closed_path_threshold = 0.2;

double delta_angle(const std::vector<Point>& points, double diameter)
{
    std::vector<double> angles;
    for (auto it = std::next(points.begin()); it != points.end(); ++it)
    {
        const auto& previous = *std::prev(it);
        const auto& point = *it;
        if (get_distance(previous, point) >
            std::numeric_limits<double>::epsilon())
        {
            angles.emplace_back(get_angle(
            {
                point[0] - previous[0],
                point[1] - previous[1],
            }));
        }
    }

    double delta = 0.0;
    for (auto it = std::next(angles.begin()); it != angles.end(); ++it)
    {
        delta += get_rotation(*std::prev(it), *it);
    }
    if (angles.size() > 1 &&
        get_distance(points.front(), points.back()) <
            diameter * closed_path_threshold)
    {
        delta += get_rotation(angles.back(), angles.front());
    }
    return delta;
}

}

Convex::Convex(const ConvexData& data)
    : frame_{ data.frame }
    , b_a_{ data.b_a }
    , b_x_{ data.b_x }
    , b_y_{ data.b_y }
    , e_a_{ data.e_a }
    , e_x_{ data.e_x }
    , e_y_{ data.e_y }
    , d_a_{ data.d_a }
    , n_b_{ data.n_b }
    , n_e_{ data.n_e }
{
}

Convex::Convex(const std::vector<Point>& points)
{
    frame_ = empty_frame();
    extend_frame(frame_, points.front());
    if (points.size() == 1)
    {
        b_a_ = 0.0;
        b_x_ = 0.0;
        b_y_ = 0.0;
        e_a_ = 0.0;
        e_x_ = 0.0;
        e_y_ = 0.0;
        d_a_ = 0.0;
        n_b_ = n_e_ = 1;
    }
    else
    {
        for (auto it = points.begin() + 1; it != points.end(); ++ it)
        {
            extend_frame(frame_, *it);
        }
        auto diameter = get_distance(frame_[0], frame_[1]);
        b_a_ = get_angle(
        {
            points[1][0] - points[0][0],
            points[1][1] - points[0][1]
        });
        auto scale = std::max(diameter, std::numeric_limits<double>::epsilon());
        b_x_ = (points[0][0] - frame_[0][0]) / scale;
        b_y_ = (points[0][1] - frame_[0][1]) / scale;
        e_a_ = get_angle(
        {
            points[points.size() - 2][0] - points[points.size() - 1][0],
            points[points.size() - 2][1] - points[points.size() - 1][1]
        });
        e_x_ = (points[points.size() - 1][0] - frame_[0][0]) / scale;
        e_y_ = (points[points.size() - 1][1] - frame_[0][1]) / scale;
        d_a_ = delta_angle(points, diameter);
        n_b_ = n_e_ = points.size();
    }
}

Convex::~Convex()
{
}

void Convex::invert()
{
    std::swap(b_a_, e_a_);
    std::swap(b_x_, e_x_);
    std::swap(b_y_, e_y_);
    d_a_ = -d_a_;
}

double Convex::compare(const Convex& convex) const
{
    return compare(convex, 0.0);
}

double Convex::compare(const Convex& convex, double rotation) const
{
    double diff;
    diff = 0.0;
    int count = 0;
    double angle_unit = std::numbers::pi * 0.25;
    diff += std::abs(get_rotation(b_a_, convex.b_a_ - rotation)) /
        angle_unit;
    diff += std::abs(b_x_ - convex.b_x_);
    diff += std::abs(b_y_ - convex.b_y_);
    diff += std::abs(get_rotation(e_a_, convex.e_a_ - rotation)) /
        angle_unit;
    diff += std::abs(e_x_ - convex.e_x_);
    diff += std::abs(e_y_ - convex.e_y_);
    count += 6;
    diff += std::abs(d_a_ - convex.d_a_) / angle_unit;
    count += 1;
    if (convex.n_b_ < n_b_)
    {
        diff += double(n_b_ - convex.n_b_) / std::max(1, n_b_);
        ++count;
    }
    if (convex.n_e_ > n_e_)
    {
        diff += double(convex.n_e_ - n_e_) / std::max(1, n_e_);
        ++count;
    }
    diff /= count;
    return diff;
}

ConvexMatch Convex::compare_best_rotation(const Convex& convex) const
{
    std::vector<double> rotations { 0.0 };
    rotations.emplace_back(get_rotation(b_a_, convex.b_a_));
    rotations.emplace_back(get_rotation(e_a_, convex.e_a_));
    double best_diff = std::numeric_limits<double>::max();
    double best = 0.0;
    for (double rotation : rotations)
    {
        auto diff = compare(convex, rotation);
        if (diff < best_diff)
        {
            best_diff = diff;
            best = rotation;
        }
    }
    return { best_diff, best };
}

const Rectangle& Convex::get_frame() const
{
    return frame_;
}

std::vector<Convex> Convex::get_convexes(const std::vector<Point>& points)
{
    std::vector<Convex> convexes;
    double last_angle = 0.0;
    bool has_last_angle = false;
    std::vector<Point> convex_points{ points[0] };
    bool mid_start = false;
    Point mid_point;
    for (std::size_t end = 1;; ++end)
    {
        bool cut = false;
        double new_angle;
        if (end != points.size())
        {
            if (convex_points.size() > 1)
            {
                new_angle = get_angle(
                    points[end], points[end - 1], points[end - 2]);
                if (abs(new_angle) <
                    std::numbers::pi / 3.0)
                {
                    cut = true;
                    end -= 1;
                }
                else if (convex_points.size() > 2 && has_last_angle &&
                    new_angle * last_angle < 0.0)
                {
                    mid_point =
                    {
                        (points[end - 2][0] + points[end - 1][0]) / 2.0,
                        (points[end - 2][1] + points[end - 1][1]) / 2.0,
                    };
                    convex_points.back() = mid_point;
                    mid_start = true;
                    cut = true;
                    end -= 2;
                }
            }
        }
        else
        {
            cut = true;
        }
        if (cut)
        {
            convexes.emplace_back(convex_points);
            has_last_angle = false;
            if (end != points.size())
            {
                if (!mid_start)
                {
                    convex_points = { points[end] };
                }
                else
                {
                    mid_start = false;
                    convex_points = { mid_point };
                }
            }
            else
            {
                break;
            }            
        }
        else
        {
            if (convex_points.size() > 1)
            {
                has_last_angle = true;
                last_angle = new_angle;
            }
            convex_points.emplace_back(points[end]);
        }
    }
    return convexes;
}
