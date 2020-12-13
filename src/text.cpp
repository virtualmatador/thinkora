#include "toolbox.h"

#include "text.h"

void Text::set_text(const Cairo::RefPtr<Cairo::Context>& cr,
    const Point& position, const double& height, const std::string& text)
{
    text_ = text;
    double font_size = height * 1.25;
    Cairo::TextExtents extents;
    cr->set_font_size(font_size);
    cr->get_text_extents(text_, extents);
    height_ = font_size / extents.height;
    frame_ =
    {
        position,
        position[0] + extents.width, position[1] + extents.height
    };
}

Shape::Type Text::get_type() const
{
    return Type::TEXT;
}

void Text::draw_details(const Cairo::RefPtr<Cairo::Context>& cr,
        const int& zoom_delta, const Point& pad) const
{
    Rectangle frame =
    {
        transform(frame_[0], zoom_delta, pad),
        transform(frame_[1], zoom_delta, pad),
    };
    cr->set_font_size(height_ * (frame[1][1] - frame[0][1]));
    cr->move_to(frame[0][0], frame[1][1]);
    cr->show_text(text_);
}

void Text::write_dtails(std::ostream& os) const
{
    os << text_.size();
    os.write(text_.c_str(), text_.size());
    os << ' ' << height_ << std::endl;
}

void Text::read_details(std::istream& is)
{
    std::size_t size;
    is >> size;
    text_.resize(0);
    text_.reserve(size);
    std::copy_n(std::istreambuf_iterator(is), size, std::back_inserter(text_));
    is.ignore(1);
    is >> height_;
}
