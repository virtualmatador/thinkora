#include <exception>
#include <limits>

#include "board.h"
#include "circle.h"
#include "dot.h"
#include "line.h"
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
    case Shape::Type::DOT:
        shape = new Dot;
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
    : width_{ 1.0 }
    , color_{ Gdk::RGBA("#FFFFFF") }
    , style_{ Style::SOLID }
{
}

Shape::Shape(const Shape* shape)
    : width_{ shape->width_ }
    , color_{ shape->color_ }
    , style_{ shape->style_ }
{
}

Shape::Shape(const double& width, const Gdk::RGBA& color, const Style& style)
    : width_{ width }
    , color_{ color }
    , style_{ style }
{
}

Shape::~Shape()
{
}

const double& Shape::get_width() const
{
    return width_;
}

const Gdk::RGBA& Shape::get_color() const
{
    return color_;
}

const Shape::Style& Shape::get_style() const
{
    return style_;
}

const Rectangle& Shape::get_frame() const
{
    return frame_;
}

void Shape::draw(const Cairo::RefPtr<Cairo::Context>& cr,
    const int& zoom_delta, const Point& pad) const
{
    cr->begin_new_path();
    cr->set_source_rgba(color_.get_red(), color_.get_green(),
        color_.get_blue(), color_.get_alpha());
    cr->set_line_width(width_);
    cr->set_dash(Board::dashes_[width_ - 1][int(style_)], 0.0);
    cr->set_line_cap(Cairo::LineCap::LINE_CAP_BUTT);
    draw_details(cr, zoom_delta, pad);
}

void Shape::write(std::ostream& os) const
{
    os << width_ << std::endl;
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
    is >> width_;
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
