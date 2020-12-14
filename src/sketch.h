#ifndef THINKORA_SRC_SKETCH_H
#define THINKORA_SRC_SKETCH_H

#include <array>
#include <chrono>
#include <cstddef>
#include <vector>

#include "pattern.h"
#include "shape.h"

class Sketch: public Shape
{
public:
    using Shape::Shape;
    void set_sketch(int zoom);
    void add_point(const Point& point);
    void set_birth(const std::chrono::steady_clock::time_point& birth);
    const std::vector<Point>& get_points() const;
    const std::chrono::steady_clock::time_point& get_birth() const;
    const int& get_zoom() const;
    std::vector<Point> simplify() const;

public:
    Type get_type() const override;

private:
    void draw_details(const Cairo::RefPtr<Cairo::Context>& cr,
        const int& zoom_delta, const Point& pad) const override;
    void write_dtails(std::ostream& os) const override;
    void read_details(std::istream& is) override;

private:
    std::vector<Point> points_;
    std::chrono::steady_clock::time_point birth_;
    int zoom_;
};

#endif // THINKORA_SRC_SKETCH_H
