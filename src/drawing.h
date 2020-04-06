#ifndef DRAWING_H
#define DRAWING_H

#include "shape.h"

class Drawing final
{
public:
    Drawing(Shape* shape, const int& zoom, const int& time);

private:
    Shape* shape_;
    int zoom_;
    int time_;

private:
    friend class Board;
};

#endif // DRAWING_H
