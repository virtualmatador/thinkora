#include "toolbox.h"

#include "shape.h"

Shape::Shape(std::vector<std::array<int, 2>>&& points)
    : points_{std::move(points)}
    , color_{0.0, 0.0, 0.0, 1.0}
{
    frame_[0] = points_[0];
    frame_[1] = points_[0];
    for (int i = 1; i < points_.size(); ++i)
    {
        frame_[0][0] = std::min(frame_[0][0], points_[i][0]);
        frame_[0][1] = std::min(frame_[0][1], points_[i][1]);
        frame_[1][0] = std::max(frame_[1][0], points_[i][0]);
        frame_[1][1] = std::max(frame_[1][1], points_[i][1]);
    }
}

Shape::~Shape()
{
}

void Shape::set_color(const std::array<double, 4>& color)
{
    color_ = color;
}

void Shape::draw(const Cairo::RefPtr<Cairo::Context>& cr,
    const int& zoom_delta, const std::array<int, 2>& pad) const
{
    draw_points(cr, transform(zoom_delta, pad));
}

std::vector<std::array<int, 2>> Shape::transform(const int& zoom_delta,
    const std::array<int, 2>& pad) const
{
    std::vector<std::array<int, 2>> transformed(points_.size());
    for (int i = 0; i < transformed.size(); ++i)
    {
        transformed[i] = zoom(points_[i], zoom_delta);
        transformed[i][0] -= pad[0];
        transformed[i][1] -= pad[1];
    }
    return transformed;
}
