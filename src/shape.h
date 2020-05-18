#ifndef THINKORA_SRC_SHAPE_H
#define THINKORA_SRC_SHAPE_H

#include <array>
#include <fstream>
#include <string>
#include <vector>

#include <gtkmm.h>

class Shape
{
public:
    enum class Type
    {
        SKETCH,
        POINT,
        LINE,
        CIRCLE,
        TEXT,
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
    static Shape* create_shape(const Shape::Type& type);

public:
    Shape();
    Shape(const int& line_width, const Gdk::RGBA& color, const Style& style);
    const int& get_line_width() const;
    const Gdk::RGBA& get_color() const;
    const Style& get_style() const;
    const std::array<std::array<int, 2>, 2>& get_frame() const;
    void draw(const Cairo::RefPtr<Cairo::Context>& cr,
        const int& zoom_delta, const std::array<int, 2>& pad) const;

private:
    void write(std::ostream& os) const;
    void read(std::istream& is);

public:
    virtual Type get_type() const = 0;

private:
    virtual void draw_details(const Cairo::RefPtr<Cairo::Context>& cr,
        const int& zoom_delta, const std::array<int, 2>& pad) const = 0;
    virtual void write_dtails(std::ostream& os) const = 0;
    virtual void read_details(std::istream& is) = 0;

public:
    static void fill_dashes(const int& thickness_limit);

private:
    int line_width_;
    Gdk::RGBA color_;
    Style style_;

protected:
    std::array<std::array<int, 2>, 2> frame_;

private:
    friend std::ostream& operator<<(std::ostream& os, const Shape& shape);
    friend std::istream& operator>>(std::istream& is, Shape& shape);
};

std::ostream& operator<<(std::ostream& os, const Shape& shape);
std::istream& operator>>(std::istream& is, Shape& shape);

#endif // THINKORA_SRC_SHAPE_H
