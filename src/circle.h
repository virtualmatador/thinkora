#ifndef THINKORA_SRC_CIRCLE_H
#define THINKORA_SRC_CIRCLE_H

#include <array>

#include "shape.h"

class Circle: public Shape
{
public:
    using Shape::Shape;
    void set_circle(const std::array<int, 2>& center, const int& radius);

public:
    Type get_type() const override;

private:
    void draw_details(const Cairo::RefPtr<Cairo::Context>& cr,
        const int& zoom_delta, const std::array<int, 2>& pad) const;
    void write_dtails(std::ostream& os) const;
    void read_details(std::istream& is);

private:
    std::array<std::array<int, 2>, 2> circle_;
};

#endif // THINKORA_SRC_CIRCLE_H
