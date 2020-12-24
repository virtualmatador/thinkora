#include <numbers>

#include "toolbox.h"

#include "convex.h"

Convex::Convex(const jsonio::json& json)
{
    auto frame = json["frame"];
    frame_ =
    {
        frame["left"].get_double(),
        frame["top"].get_double(),
        frame["right"].get_double(),
        frame["bottom"].get_double(),
    };
    auto convex = json["convex"];
    const jsonio::json* value;
    value = convex.get_value("b_a");
    if (value)
    {
        b_a_b_ = true;
        b_a_ = convex["b_a"].get_double() * std::numbers::pi / 180.0;
        b_x_ = convex["b_x"].get_double();
        b_y_ = convex["b_y"].get_double();
    }
    else
    {
        b_a_b_ = false;
    }
    value = convex.get_value("e_a");
    if (value)
    {
        e_a_b_ = true;
        e_a_ = convex["e_a"].get_double() * std::numbers::pi / 180.0;
        e_x_ = convex["e_x"].get_double();
        e_y_ = convex["e_y"].get_double();
    }
    else
    {
        e_a_b_ = false;
    }
    value = convex.get_value("d_a");
    if (value)
    {
        d_a_b_ = true;
        d_a_ = convex["d_a"].get_double() * std::numbers::pi / 180.0;
    }
    else
    {
        d_a_b_ = false;
    }
    d_l_ = convex["d_l"].get_double();
    d_r_ = convex["d_r"].get_double() * std::numbers::pi / 180.0;
    n_b_ = convex["n_b"].get_long();
    n_e_ = convex["n_e"].get_long();
}

Convex::Convex(const std::vector<Point>& points, double rotation)
: d_r_{ rotation }
{
    frame_ = empty_frame();
    extend_frame(frame_, points.front());
    if (points.size() == 1)
    {
        b_a_b_ = false;
        e_a_b_ = false;
        d_a_b_ = false;
        d_l_ = 0.0;
        n_b_ = n_e_ = 1;
    }
    else
    {
        for (auto it = points.begin() + 1; it != points.end(); ++ it)
        {
            extend_frame(frame_, *it);
        }
        auto diameter = get_distance(frame_[0], frame_[1]);
        b_a_b_ = true;
        b_a_ = get_angle(
        {
            points[1][0] - points[0][0],
            points[1][1] - points[0][1]
        });
        b_x_ = (points[0][0] - frame_[0][0]) / diameter;
        b_y_ = (points[0][1] - frame_[0][1]) / diameter;
        e_a_b_ = true;
        e_a_ = get_angle(
        {
            points[points.size() - 2][0] - points[points.size() - 1][0],
            points[points.size() - 2][1] - points[points.size() - 1][1]
        });
        e_x_ = (points[points.size() - 1][0] - frame_[0][0]) / diameter;
        e_y_ = (points[points.size() - 1][1] - frame_[0][1]) / diameter;
        d_a_b_ = true;
        d_a_ = get_angle(
        {
            points[points.size() - 1][0] - points[0][0],
            points[points.size() - 1][1] - points[0][1]
        });
        d_l_ = get_distance(points[points.size() - 1], points[0]) / diameter;
        n_b_ = n_e_ = points.size();
    }
}

Convex::~Convex()
{
}

void Convex::invert()
{
    std::swap(b_a_b_, e_a_b_);
    std::swap(b_a_, e_a_);
    std::swap(b_x_, e_x_);
    std::swap(b_y_, e_y_);
    if (d_a_ > 0.0)
    {
        d_a_ -= 2.0 * std::numbers::pi;
    }
    else
    {
        d_a_ += 2.0 * std::numbers::pi;
    }
    d_r_ = -d_r_;
}

double Convex::compare(const Convex& convex) const
{
    double diff;
    if (n_b_ <= convex.n_b_ && n_e_ >= convex.n_e_)
    {
        diff = 0.0;
        int count = 0;
        double angle_unit = std::numbers::pi * 0.25;
        if (b_a_b_)
        {
            diff += std::abs(get_rotation(b_a_, convex.b_a_)) / angle_unit;
            diff += std::abs(b_x_ - convex.b_x_);
            diff += std::abs(b_y_ - convex.b_y_);
            count += 3;
        }
        if (e_a_b_)
        {
            diff += std::abs(get_rotation(e_a_, convex.e_a_)) / angle_unit;
            diff += std::abs(e_x_ - convex.e_x_);
            diff += std::abs(e_y_ - convex.e_y_);
            count += 3;
        }
        if (d_a_b_)
        {
            diff += std::abs(get_rotation(d_a_, convex.d_a_)) / angle_unit;
            count += 1;
        }
        diff += std::abs(d_r_ - convex.d_r_) / angle_unit;
        diff += std::abs(d_l_ - convex.d_l_);
        count += 2;
        diff /= count;
    }
    else
    {
        diff = 1.0;
    }
    return diff;
}

const Rectangle& Convex::get_frame() const
{
    return frame_;
}

std::vector<Convex> Convex::get_convexes(const std::vector<Point>& points)
{
    std::vector<Convex> convexes;
    double last_angle;
    double rotation = 0.0;
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
                else if (convex_points.size() > 2 &&
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
            convexes.emplace_back(convex_points, rotation);
            rotation = 0.0;
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
                rotation += get_rotation(last_angle, new_angle);
            }
            last_angle = new_angle;
            convex_points.emplace_back(points[end]);
        }
    }
    return convexes;
}
