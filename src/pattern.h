#ifndef PATTERN_H
#define PATTERN_H

#include <array>
#include <vector>

#include "json.h"

class Pattern
{
public:
    Pattern(const jsonio::json& pattern);
    ~Pattern();
    double Match(const std::vector<std::array<int, 2>>& points);

private:
    class Convex
    {
    public:
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
        int d_t_;
        double f_x_;
        double f_y_;
        int n_b_;
        int n_e_;
    };

private:
    std::string character_;
    std::vector<std::vector<Convex>> segments_;
};

#endif // PATTERN_H
