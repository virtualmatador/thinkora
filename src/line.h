#ifndef POLYLINE_H
#define POLYLINE_H

#include <array>
#include <vector>

#include "shape.h"

class Line final: public Shape
{
public:
    using Shape::Shape;
    void set_line(const std::array<std::array<int, 2>, 2>& points);

public:
    Type get_type() const override;

private:
    void draw_details(const Cairo::RefPtr<Cairo::Context>& cr,
        const int& zoom_delta, const std::array<int, 2>& pad) const;
    void write_dtails(std::ostream& os) const;
    void read_details(std::istream& is);

private:
    std::array<std::array<int, 2>, 2> points_;
};

#endif // POLYLINE_H
