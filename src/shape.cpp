#include <limits>

#include "toolbox.h"

#include "shape.h"

Shape::Shape()
    : color_{0.0, 0.0, 0.0, 0.0}
{
    set_frame();
}

Shape::Shape(std::vector<std::array<int, 2>>&& points,
    const std::array<double, 4>& color)
    : points_{std::move(points)}
    , color_{color}
{
    set_frame();
}

Shape::~Shape()
{
}

const std::array<std::array<int, 2>, 2>& Shape::get_frame() const
{
    return frame_;
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

void Shape::set_frame()
{
    frame_[0] = {std::numeric_limits<int>::max(),
        std::numeric_limits<int>::max()};
    frame_[1] = {std::numeric_limits<int>::min(),
        std::numeric_limits<int>::min()};;
    for (const auto& point: points_)
    {
        frame_[0][0] = std::min(frame_[0][0], point[0]);
        frame_[0][1] = std::min(frame_[0][1], point[1]);
        frame_[1][0] = std::max(frame_[1][0], point[0]);
        frame_[1][1] = std::max(frame_[1][1], point[1]);
    }
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

void Shape::write(std::ostream& os) const
{
    for (const auto& color: color_)
    {
        os << color << ' ';
    }
    os << std::endl;
    os << points_.size();
    for (const auto& point: points_)
    {
        os << std::endl << point[0] << ' ' << point[1];
    }
}

void Shape::read(std::istream& is)
{
    for (auto& color: color_)
    {
        is >> color;
    }
    std::size_t size;
    is >> size;
    for (std::size_t i = 0; i < size; ++i)
    {
        int x, y;
        is >> x >> y;
        points_.push_back({x, y});
    }
    set_frame();
}

std::ostream& operator<<(std::ostream& os, const Shape& shape)
{
    shape.write(os);
    return os;
}

std::istream& operator>>(std::istream& is, Shape& shape)
{
    shape.read(is);
    return is;
}
