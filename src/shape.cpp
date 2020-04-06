#include <exception>
#include <limits>

#include "board.h"

#include "shape.h"

Shape::Shape()
    : line_width_{1}
    , color_{Gdk::RGBA("#000000")}
    , style_{Style::SOLID}
{
}

Shape::Shape(const int& thickness, const Gdk::RGBA& color, const Style& style)
    : line_width_{thickness}
    , color_{color}
    , style_{style}
{
}

const std::array<std::array<int, 2>, 2>& Shape::get_frame() const
{
    return frame_;
}

void Shape::draw(const Cairo::RefPtr<Cairo::Context>& cr,
    const int& zoom_delta, const std::array<int, 2>& pad) const
{
    cr->set_source_rgba(color_.get_red(), color_.get_green(),
        color_.get_blue(), color_.get_alpha());
    cr->set_line_width(line_width_);
    cr->set_dash(Board::dashes_[line_width_ - 1][int(style_)], 0.0);
    draw_details(cr, zoom_delta, pad);
    cr->stroke();
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
    write_dtails(os);
}

void Shape::read(std::istream& is)
{
    is >> line_width_ ;
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
    read_details(is);
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
