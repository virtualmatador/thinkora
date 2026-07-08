#ifndef THINKORA_SRC_TEXT_H
#define THINKORA_SRC_TEXT_H

#include <array>
#include <string>

#include "shape.h"

class Text: public Shape
{
public:
    using Shape::Shape;
    void set_text(const std::string& text, const Rectangle& frame);
    void set_text(const std::string& text, const Rectangle& frame,
        double rotation);
    double get_rotation() const;
    const Rectangle& get_text_frame() const;

public:
    Type get_type() const override;

private:
    Point text_point(const Point& point) const;
    void update_frame();
    void draw_details(const Cairo::RefPtr<Cairo::Context>& cr,
        const int& zoom_delta, const Point& pad) const override;
    void write_dtails(std::ostream& os) const override;
    void read_details(std::istream& is) override;

private:
    std::string text_;
    Rectangle text_frame_ {{{ 0.0, 0.0 }, { 0.0, 0.0 }}};
    double rotation_ = 0.0;
};

#endif // THINKORA_SRC_TEXT_H
