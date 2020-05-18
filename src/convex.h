#ifndef THINKORA_SRC_CONVEX_H
#define THINKORA_SRC_CONVEX_H

#include <array>
#include <vector>

#include "json.h"

class Convex
{
public:
    static std::vector<Convex> extract(const std::vector<std::array<int, 2>>&
        points, const std::array<std::array<int, 2>, 2>& frame);

public:
    Convex(const jsonio::json& json);
    Convex(const std::array<int, 2>& point,
        const std::array<std::array<int, 2>, 2>& frame);
    Convex(const std::vector<std::array<int, 2>>& points,
        const std::size_t& begin, const std::size_t& end, const int& d_r,
        const std::array<std::array<int, 2>, 2>& convex_frame,
        const std::array<std::array<int, 2>, 2>& frame);
    ~Convex();
    double compare(const Convex& convex) const;

private:
    bool b_a_b_;
    int b_a_;
    double b_x_;
    double b_y_;
    bool e_a_b_;
    int e_a_;
    double e_x_;
    double e_y_;
    bool d_a_b_;
    int d_a_;
    double d_x_;
    double d_y_;
    int d_r_;
    double f_x_;
    double f_y_;
    int n_b_;
    int n_e_;
};

#endif // THINKORA_SRC_CONVEX_H
