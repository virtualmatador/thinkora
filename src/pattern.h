#ifndef PATTERN_H
#define PATTERN_H

#include <array>
#include <vector>

#include "jsonio/jsonio.hpp"

class Pattern
{
public:
    Pattern(const jsonio::json& pattern);
    ~Pattern();
    double Match(const std::vector<std::array<int, 2>>& points);

private:
    class Segment
    {
    private:
        class Convex
        {
        private:
            double b_x;
            double b_y;
            int b_a;
            double e_x;
            double e_y;
            int e_a;
            double d_x;
            double d_y;
            int d_a;
            int d_t;
            double f_x;
            double f_y;
        };
    
    private:
        std::vector<Convex> convexes_;
    };

private:
    wchar_t character_;
    std::vector<Segment> segments_;
};

#endif // PATTERN_H
