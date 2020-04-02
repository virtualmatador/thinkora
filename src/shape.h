#ifndef SHAPE_H
#define SHAPE_H

#include <array>
#include <fstream>
#include <vector>

#include <gtkmm.h>

class Shape
{
public:
    enum class Type
    {
        CIRCLE,
    };

public:
    Shape();
    Shape(std::vector<std::array<int, 2>>&& points,
        const Gdk::RGBA& color);
    virtual ~Shape();
    const std::array<std::array<int, 2>, 2>& get_frame() const;
    void draw(const Cairo::RefPtr<Cairo::Context>& cr,
        const int& zoom_delta, const std::array<int, 2>& pad) const;

private:
    void set_frame();
    std::vector<std::array<int, 2>> transform(const int& zoom_delta,
        const std::array<int, 2>& pad) const;
    void write(std::ostream& os) const;
    void read(std::istream& is);

public:
    virtual Type get_type() const = 0;

private:
    virtual void draw_points(const Cairo::RefPtr<Cairo::Context>& cr,
        const std::vector<std::array<int, 2>>& points) const = 0;

private:
    std::vector<std::array<int, 2>> points_;
    Gdk::RGBA color_;

private:
    std::array<std::array<int, 2>, 2> frame_;

private:
    friend std::ostream& operator<<(std::ostream& os, const Shape& shape);
    friend std::istream& operator>>(std::istream& is, Shape& shape);
};

std::ostream& operator<<(std::ostream& os, const Shape& shape);
std::istream& operator>>(std::istream& is, Shape& shape);

#endif // SHAPE_H
