#ifndef THINKORA_SRC_TEXT_H
#define THINKORA_SRC_TEXT_H

#include <array>
#include <string>

#include "shape.h"

class Text: public Shape
{
public:
    using Shape::Shape;
    void set_text(const Cairo::RefPtr<Cairo::Context>& cr,
        const Point& position, const double& height, const std::string& text);

public:
    Type get_type() const override;

private:
    void draw_details(const Cairo::RefPtr<Cairo::Context>& cr,
        const int& zoom_delta, const Point& pad) const;
    void write_dtails(std::ostream& os) const;
    void read_details(std::istream& is);

private:
    std::string text_;
    double height_;
};

#endif // THINKORA_SRC_TEXT_H
