#include "toolbox.h"

#include "text.h"

void Text::set_text(const std::string& text, const Rectangle& frame)
{
    text_ = text;
    frame_ = frame;
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
    // TODO calculate font size more accurately
    cr->set_font_size(frame[1][1] - frame[0][1]);
    cr->move_to(frame[0][0], frame[1][1]);
    cr->show_text(text_);
}

void Text::write_dtails(std::ostream& os) const
{
    os << text_.size();
    os.write(text_.c_str(), text_.size());
}

void Text::read_details(std::istream& is)
{
    std::size_t size;
    is >> size;
    text_.resize(0);
    text_.reserve(size);
    std::copy_n(std::istreambuf_iterator(is), size, std::back_inserter(text_));
    is.ignore(1);
}
