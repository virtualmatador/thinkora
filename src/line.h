#ifndef THINKORA_SRC_LINE_H
#define THINKORA_SRC_LINE_H

#include <array>
#include <vector>

#include "shape.h"

class Line: public Shape
{
public:
    using Shape::Shape;
    void set_line(const Rectangle& points);

public:
    Type get_type() const override;

private:
    void draw_details(const Cairo::RefPtr<Cairo::Context>& cr,
        const int& zoom_delta, const Point& pad) const;
    void write_dtails(std::ostream& os) const;
    void read_details(std::istream& is);

private:
    Rectangle points_;
};

#endif // THINKORA_SRC_LINE_H
