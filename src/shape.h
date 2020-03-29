#ifndef SHAPE_H
#define SHAPE_H

class Board;

class Shape
{
public:
    Shape(int zoom, int x1, int x2, int y1, int y2);
    virtual ~Shape();
    virtual void draw(Gdk::Point view, int zoom) = 0;

private:
    int zoom_;
    int x1_;
    int x2_;
    int y1_;
    int y2_;

private:
    friend class Board;
};

#endif // SHAPE_H
