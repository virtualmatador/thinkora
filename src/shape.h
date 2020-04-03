#ifndef SHAPE_H
#define SHAPE_H

#include <array>
#include <fstream>
#include <string>
#include <vector>

#include <gtkmm.h>
#include <tesseract/baseapi.h>

class Shape
{
public:
    enum class Type
    {
        LINE,
        CIRCLE,
    };
    enum class Style
    {
        SOLID,
        DASH_DASH,
        DASH_DOT,
        DOT_DOT,
        SIZE
    };

public:
    Shape();
    Shape(std::vector<std::array<int, 2>>&& points, const int& thickness,
        const Gdk::RGBA& color, const Style& style);
    virtual ~Shape();
    void set_label(const std::string& label);
    void add_point(const std::array<int, 2>& point);
    void finalize();
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

public:
    static void fill_dashes(const int& thickness_limit);

private:
    std::string label_;
    std::vector<std::array<int, 2>> points_;
    int thickness_;
    Gdk::RGBA color_;
    Style style_;
    std::array<std::array<int, 2>, 2> frame_;

private:
    friend std::ostream& operator<<(std::ostream& os, const Shape& shape);
    friend std::istream& operator>>(std::istream& is, Shape& shape);
};

std::ostream& operator<<(std::ostream& os, const Shape& shape);
std::istream& operator>>(std::istream& is, Shape& shape);

#endif // SHAPE_H
