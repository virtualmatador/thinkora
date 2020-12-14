#ifndef THINKORA_SRC_DOT_H
#define THINKORA_SRC_DOT_H

#include <array>

#include "shape.h"

class Dot: public Shape
{
public:
    using Shape::Shape;
    void set_dot(const Point& point);

public:
    Type get_type() const override;

private:
    void draw_details(const Cairo::RefPtr<Cairo::Context>& cr,
        const int& zoom_delta, const Point& pad) const override;
    void write_dtails(std::ostream& os) const override;
    void read_details(std::istream& is) override;

private:
    Point point_;
};

#endif // THINKORA_SRC_DOT_H
