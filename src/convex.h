#ifndef THINKORA_SRC_CONVEX_H
#define THINKORA_SRC_CONVEX_H

#include <array>
#include <vector>

#include "json.h"

#include "toolbox.h"

class Convex
{
public:
    Convex(const jsonio::json& json);
    Convex(const Point& point,
        const Rectangle& frame);
    Convex(const std::vector<Point>& points, const std::size_t& begin,
        const std::size_t& end, const double& d_r,
        const Rectangle& convex_frame, const Rectangle& frame);
    ~Convex();
    double compare(const Convex& convex) const;

public:
    static std::vector<Convex> get_convexes(const std::vector<Point>& points);

private:
    bool b_a_b_;
    double b_a_;
    double b_x_;
    double b_y_;
    bool e_a_b_;
    double e_a_;
    double e_x_;
    double e_y_;
    bool d_a_b_;
    double d_a_;
    double d_l_;
    double d_r_;
    double f_x_;
    double f_y_;
    int n_b_;
    int n_e_;
};

#endif // THINKORA_SRC_CONVEX_H
