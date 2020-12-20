#ifndef THINKORA_SRC_WIRE_H
#define THINKORA_SRC_WIRE_H

#include <array>
#include <vector>

#include "shape.h"

class Wire: public Shape
{
public:
    using Shape::Shape;
    void set_wire(const Rectangle& points);

public:
    Type get_type() const override;

private:
    void draw_details(const Cairo::RefPtr<Cairo::Context>& cr,
        const int& zoom_delta, const Point& pad) const override;
    void write_dtails(std::ostream& os) const override;
    void read_details(std::istream& is) override;

private:
    Rectangle points_;
};

#endif // THINKORA_SRC_WIRE_H
