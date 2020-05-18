#ifndef THINKORA_SRC_POINT_H
#define THINKORA_SRC_POINT_H

#include <array>

#include "shape.h"

class Point: public Shape
{
public:
    using Shape::Shape;
    void set_point(const std::array<int, 2>& point);

public:
    Type get_type() const override;

private:
    void draw_details(const Cairo::RefPtr<Cairo::Context>& cr,
        const int& zoom_delta, const std::array<int, 2>& pad) const;
    void write_dtails(std::ostream& os) const;
    void read_details(std::istream& is);

private:
    std::array<int, 2> point_;
};

#endif // THINKORA_SRC_POINT_H
