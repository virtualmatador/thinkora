#include <algorithm>
#include <cmath>
#include <istream>
#include <iterator>
#include <ostream>

#include "toolbox.h"

#include "text.h"

namespace
{
double zoom_factor(const int& zoom)
{
    if (zoom > 0)
    {
        return std::pow(2.0, zoom);
    }
    if (zoom < 0)
    {
        return 1.0 / std::pow(2.0, -zoom);
    }
    return 1.0;
}

void include_point(Rectangle& frame, const Point& point)
{
    frame[0][0] = std::min(frame[0][0], point[0]);
    frame[0][1] = std::min(frame[0][1], point[1]);
    frame[1][0] = std::max(frame[1][0], point[0]);
    frame[1][1] = std::max(frame[1][1], point[1]);
}
}

void Text::set_text(const std::string& text, const Rectangle& frame)
{
    set_text(text, frame, 0.0);
}

void Text::set_text(const std::string& text, const Rectangle& frame,
    double rotation)
{
    text_ = text;
    text_frame_ = frame;
    rotation_ = rotation;
    update_frame();
}

double Text::get_rotation() const
{
    return rotation_;
}

const Rectangle& Text::get_text_frame() const
{
    return text_frame_;
}

Shape::Type Text::get_type() const
{
    return Type::TEXT;
}

Point Text::text_point(const Point& point) const
{
    const double c = std::cos(rotation_);
    const double s = std::sin(rotation_);
    return
    {
        point[0] * c - point[1] * s,
        point[0] * s + point[1] * c,
    };
}

void Text::update_frame()
{
    frame_ = empty_frame();
    std::array<Point, 4> corners
    {{
        text_frame_[0],
        { text_frame_[1][0], text_frame_[0][1] },
        text_frame_[1],
        { text_frame_[0][0], text_frame_[1][1] },
    }};
    for (const auto& corner : corners)
    {
        include_point(frame_, text_point(corner));
    }
}

void Text::draw_details(const Cairo::RefPtr<Cairo::Context>& cr,
        const int& zoom_delta, const Point& pad) const
{
    if (text_.empty())
    {
        return;
    }

    const double frame_height = text_frame_[1][1] - text_frame_[0][1];
    if (frame_height <= 0.0)
    {
        return;
    }

    cr->save();
    cr->translate(-pad[0], -pad[1]);
    const double zoom = zoom_factor(zoom_delta);
    cr->scale(zoom, zoom);
    cr->rotate(rotation_);

    cr->set_font_size(frame_height);

    Cairo::TextExtents text_extents;
    cr->get_text_extents(text_, text_extents);
    if (text_extents.height > 0.0)
    {
        cr->set_font_size(
            frame_height * frame_height / text_extents.height);
        cr->get_text_extents(text_, text_extents);
        cr->move_to(
            text_frame_[0][0] - text_extents.x_bearing,
            text_frame_[0][1] - text_extents.y_bearing);
    }
    else
    {
        Cairo::FontExtents font_extents;
        cr->get_font_extents(font_extents);
        const double font_height =
            font_extents.ascent + font_extents.descent;
        if (font_height <= 0.0)
        {
            cr->restore();
            return;
        }

        cr->set_font_size(frame_height * frame_height / font_height);
        cr->get_font_extents(font_extents);
        cr->move_to(
            text_frame_[0][0], text_frame_[0][1] + font_extents.ascent);
    }

    cr->show_text(text_);
    cr->restore();
}

void Text::write_dtails(std::ostream& os) const
{
    os << text_.size();
    os.write(text_.c_str(), text_.size());
    os << std::endl <<
        'R' << ' ' <<
        rotation_ << ' ' <<
        text_frame_[0][0] << ' ' <<
        text_frame_[0][1] << ' ' <<
        text_frame_[1][0] << ' ' <<
        text_frame_[1][1];
}

void Text::read_details(std::istream& is)
{
    std::size_t size;
    is >> size;
    text_.resize(0);
    text_.reserve(size);
    std::copy_n(std::istreambuf_iterator(is), size, std::back_inserter(text_));
    is.ignore(1);
    text_frame_ = frame_;
    rotation_ = 0.0;
    if (is.peek() == 'R')
    {
        char marker;
        is >> marker >>
            rotation_ >>
            text_frame_[0][0] >>
            text_frame_[0][1] >>
            text_frame_[1][0] >>
            text_frame_[1][1];
    }
    update_frame();
}
