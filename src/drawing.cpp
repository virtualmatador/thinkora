#include "drawing.h"

Drawing::Drawing(Shape* shape, const int& zoom, const int& time)
    : shape_{shape}
    , zoom_{zoom}
    , time_{time}
{
}
