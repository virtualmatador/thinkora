#ifndef CIRCLE_H
#define CIRCLE_H

#include "shape.h"

class Circle final: public Shape
{
public:
    using Shape::Shape;

private:
    void draw_points(const Cairo::RefPtr<Cairo::Context>& cr,
        const std::vector<std::array<int, 2>>& points) const override;
    Type get_type() const override;
};

#endif // CIRCLE_H
