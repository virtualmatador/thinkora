#include "board.h"

#include "shape.h"

Shape::Shape(int zoom, int x1, int x2, int y1, int y2)
    : zoom_{zoom}
    , x1_{x1}
    , x2_{x2}
    , y1_{y1}
    , y2_{y2}
{
}

Shape::~Shape()
{
}
