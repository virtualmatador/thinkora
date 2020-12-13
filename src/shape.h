#ifndef THINKORA_SRC_SHAPE_H
#define THINKORA_SRC_SHAPE_H

#include <array>
#include <fstream>
#include <string>
#include <vector>

#include <gtkmm.h>

#include "toolbox.h"

class Shape
{
public:
    enum class Type
    {
        SKETCH,
        DOT,
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
    Shape(const Shape* shape);
    Shape(const double& width, const Gdk::RGBA& color, const Style& style);
    virtual ~Shape();
    const double& get_width() const;
    const Gdk::RGBA& get_color() const;
    const Style& get_style() const;
    const Rectangle& get_frame() const;
    void draw(const Cairo::RefPtr<Cairo::Context>& cr, const int& zoom_delta,
        const Point& pad) const;

private:
    void write(std::ostream& os) const;
    void read(std::istream& is);

public:
    virtual Type get_type() const = 0;

private:
    virtual void draw_details(const Cairo::RefPtr<Cairo::Context>& cr,
        const int& zoom_delta, const Point& pad) const = 0;
    virtual void write_dtails(std::ostream& os) const = 0;
    virtual void read_details(std::istream& is) = 0;

public:
    static void fill_dashes(const int& thickness_limit);

private:
    double width_;
    Gdk::RGBA color_;
    Style style_;

protected:
    Rectangle frame_;

private:
    friend std::ostream& operator<<(std::ostream& os, const Shape& shape);
    friend std::istream& operator>>(std::istream& is, Shape& shape);
};

std::ostream& operator<<(std::ostream& os, const Shape& shape);
std::istream& operator>>(std::istream& is, Shape& shape);

#endif // THINKORA_SRC_SHAPE_H
