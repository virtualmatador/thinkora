#include "toolbox.h"

#include "convex.h"

Convex::Convex(const jsonio::json& json)
{
    const jsonio::json* value;
    value = json.get_value("b_a");
    if (value)
    {
        b_a_b_ = true;
        b_a_ = json["b_a"].get_long();
        b_x_ = json["b_x"].get_double();
        b_y_ = json["b_y"].get_double();
    }
    else
    {
        b_a_b_ = false;
    }
    value = json.get_value("e_a");
    if (value)
    {
        e_a_b_ = true;
        e_a_ = json["e_a"].get_long();
        e_x_ = json["e_x"].get_double();
        e_y_ = json["e_y"].get_double();
    }
    else
    {
        e_a_b_ = false;
    }
    value = json.get_value("d_a");
    if (value)
    {
        d_a_b_ = true;
        d_a_ = json["d_a"].get_long();
    }
    else
    {
        d_a_b_ = false;
    }
    d_l_ = json["d_l"].get_double();
    d_r_ = json["d_r"].get_long();
    f_x_ = json["f_x"].get_double();
    f_y_ = json["f_y"].get_double();
    n_b_ = json["n_b"].get_long();
    n_e_ = json["n_e"].get_long();
}

Convex::Convex(const std::array<int, 2>& point,
    const std::array<std::array<int, 2>, 2>& frame)
{
    b_a_b_ = false;
    e_a_b_ = false;
    d_a_b_ = false;
    f_x_ = double(point[0] - frame[0][0] + 1) /
        double(frame[1][0] - frame[0][0]);
    f_y_ = double(point[1] - frame[0][1] + 1) /
        double(frame[1][1] - frame[0][1]);
    n_b_ = n_e_ = 1;
}

Convex::Convex(const std::vector<std::array<int, 2>>& points,
    const std::size_t& begin, const std::size_t& end, const int& d_r,
    const std::array<std::array<int, 2>, 2>& convex_frame,
    const std::array<std::array<int, 2>, 2>& frame)
{
    auto diameter = get_diameter(frame);
    b_a_b_ = true;
    b_a_ = get_angle(
    {
        points[begin + 1][0] - points[begin][0],
        points[begin + 1][1] - points[begin][1]
    });
    b_x_ = double(points[begin][0] - frame[0][0]) / diameter;
    b_y_ = double(points[begin][1] - frame[0][1]) / diameter;
    e_a_b_ = true;
    e_a_ = get_angle(
    {
        points[end - 2][0] - points[end - 1][0],
        points[end - 2][1] - points[end - 1][1]
    });
    e_x_ = double(points[end - 1][0] - frame[0][0]) / diameter;
    e_y_ = double(points[end - 1][1] - frame[0][1]) / diameter;
    d_a_b_ = true;
    d_a_ = get_angle(
    {
        points[end - 1][0] - points[begin][0],
        points[end - 1][1] - points[begin][1]
    });
    d_l_ = get_distance(points[end - 1], points[begin]) / diameter;
    d_r_ = d_r;
    f_x_ = double(convex_frame[1][0] - convex_frame[0][0] + 1) /
        double(frame[1][0] - frame[0][0] + 1);
    f_y_ = double(convex_frame[1][1] - convex_frame[0][1] + 1) /
        double(frame[1][1] - frame[0][1] + 1);
    n_b_ = n_e_ = end - begin;
}

Convex::~Convex()
{
}

double Convex::compare(const Convex& convex) const
{
    double similarity;
    if (n_b_ <= convex.n_b_ && n_e_ >= convex.n_e_)
    {
        similarity = 1.0;
        int count = 0;
        if (b_a_b_)
        {
            similarity -= double(std::abs(get_rotation(b_a_, convex.b_a_))) /
                45.0;
            similarity -= std::abs(b_x_ - convex.b_x_);
            similarity -= std::abs(b_y_ - convex.b_y_);
            count += 3;
        }
        if (e_a_b_)
        {
            similarity -= double(std::abs(get_rotation(e_a_, convex.e_a_))) /
                45.0;
            similarity -= std::abs(e_x_ - convex.e_x_);
            similarity -= std::abs(e_y_ - convex.e_y_);
            count += 3;
        }
        if (d_a_b_)
        {
            similarity -= double(std::abs(get_rotation(d_a_, convex.d_a_))) /
                45.0;
            count += 1;
        }
        similarity -= double(std::abs(d_r_ - convex.d_r_)) / 45.0;
        similarity -= std::abs(d_l_ - convex.d_l_);
        count += 2;
        similarity -= std::abs(f_x_ - convex.f_x_);
        similarity -= std::abs(f_y_ - convex.f_y_);
        count += 2;
        similarity /= count;
    }
    else
    {
        similarity = 0.0;
    }
    return similarity;
}
