#ifndef LINE_H
#define LINE_H

#include "shape.h"

class Line final: public Shape
{
public:
    using Shape::Shape;
    void add_point(const std::array<int, 2>& point);

private:
    std::array<std::array<int, 2>, 2> draw_points(const Cairo::RefPtr<Cairo::Context>& cr,
        const std::vector<std::array<int, 2>>& points) const override;
    Type get_type() const override;
};

#endif // LINE_H
