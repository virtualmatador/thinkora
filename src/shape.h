#ifndef SHAPE_H
#define SHAPE_H

#include <array>
#include <vector>

#include <gtkmm.h>

class Shape
{
public:
    Shape(std::vector<std::array<int, 2>>&& points);
    virtual ~Shape();
    void set_color(const std::array<double, 4>& color);
    void draw(const Cairo::RefPtr<Cairo::Context>& cr,
        const int& zoom_delta, const std::array<int, 2>& pad) const;

private:
    virtual void draw_points(const Cairo::RefPtr<Cairo::Context>& cr,
        const std::vector<std::array<int, 2>>& points) const = 0;

private:
    std::vector<std::array<int, 2>> transform(const int& zoom_delta,
        const std::array<int, 2>& pad) const;

protected:
    std::vector<std::array<int, 2>> points_;
    std::array<double, 4> color_;
    std::array<std::array<int, 2>, 2> frame_;

private:
    friend class Board;
};

#endif // SHAPE_H
