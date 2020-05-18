#include <exception>
#include <limits>

#include "board.h"
#include "circle.h"
#include "line.h"
#include "point.h"
#include "sketch.h"
#include "text.h"

#include "shape.h"

Shape* Shape::create_shape(const Shape::Type& type)
{
    Shape* shape;
    switch (type)
    {
    case Shape::Type::SKETCH:
        shape = new Sketch;
        break;
    case Shape::Type::POINT:
        shape = new Point;
        break;
    case Shape::Type::LINE:
        shape = new Line;
        break;
    case Shape::Type::CIRCLE:
        shape = new Circle;
        break;
    case Shape::Type::TEXT:
        shape = new Text;
        break;
    default:
        shape = nullptr;
        break;
    }
    return shape;
}

Shape::Shape()
    : line_width_{1}
    , color_{Gdk::RGBA("#000000")}
    , style_{Style::SOLID}
{
}

Shape::Shape(const int& line_width, const Gdk::RGBA& color, const Style& style)
    : line_width_{line_width}
    , color_{color}
    , style_{style}
{
}

const int& Shape::get_line_width() const
{
    return line_width_;
}

const Gdk::RGBA& Shape::get_color() const
{
    return color_;
}

const Shape::Style& Shape::get_style() const
{
    return style_;
}

const std::array<std::array<int, 2>, 2>& Shape::get_frame() const
{
    return frame_;
}

void Shape::draw(const Cairo::RefPtr<Cairo::Context>& cr,
    const int& zoom_delta, const std::array<int, 2>& pad) const
{
    cr->begin_new_path();
    cr->set_source_rgba(color_.get_red(), color_.get_green(),
        color_.get_blue(), color_.get_alpha());
    cr->set_line_width(line_width_);
    cr->set_dash(Board::dashes_[line_width_ - 1][int(style_)], 0.0);
    cr->set_line_cap(Cairo::LineCap::LINE_CAP_BUTT);
    draw_details(cr, zoom_delta, pad);
}

void Shape::write(std::ostream& os) const
{
    os << line_width_ << std::endl;
    os <<
        color_.get_red() << ' ' <<
        color_.get_green() << ' ' <<
        color_.get_blue() << ' ' <<
        color_.get_alpha() << std::endl;
    os << int(style_) << std::endl;
    os <<
        frame_[0][0] << ' ' <<
        frame_[0][1] << ' ' <<
        frame_[1][0] << ' ' <<
        frame_[1][1] << std::endl;
    write_dtails(os);
}

void Shape::read(std::istream& is)
{
    is >> line_width_;
    double color;
    is >> color;
    color_.set_red(color);
    is >> color;
    color_.set_green(color);
    is >> color;
    color_.set_blue(color);
        is >> color;
    color_.set_alpha(color);
    is >> (int&)style_;
    is >>
        frame_[0][0] >>
        frame_[0][1] >>
        frame_[1][0] >>
        frame_[1][1];
    read_details(is);
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
