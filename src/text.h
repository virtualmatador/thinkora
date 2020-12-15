#ifndef THINKORA_SRC_TEXT_H
#define THINKORA_SRC_TEXT_H

#include <array>
#include <string>

#include "shape.h"

class Text: public Shape
{
public:
    using Shape::Shape;
    void set_text(const std::string& text, const double& height,
        const Rectangle& frame);

public:
    Type get_type() const override;

private:
    void draw_details(const Cairo::RefPtr<Cairo::Context>& cr,
        const int& zoom_delta, const Point& pad) const override;
    void write_dtails(std::ostream& os) const override;
    void read_details(std::istream& is) override;

private:
    std::string text_;
    double height_;
};

#endif // THINKORA_SRC_TEXT_H
