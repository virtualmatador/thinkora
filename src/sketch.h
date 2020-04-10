#ifndef SKETCH_H
#define SKETCH_H

#include <array>
#include <chrono>
#include <cstddef>
#include <vector>

#include "shape.h"

class Sketch final: public Shape
{
public:
    using Shape::Shape;
    void add_point(const std::array<int, 2>& point);
    void set_birth();
    const std::chrono::steady_clock::time_point& get_birth();

public:
    Type get_type() const override;
    void set_frame() override;

private:
    void draw_details(const Cairo::RefPtr<Cairo::Context>& cr,
        const int& zoom_delta, const std::array<int, 2>& pad) const;
    void write_dtails(std::ostream& os) const;
    void read_details(std::istream& is);

private:
    std::vector<std::array<int, 2>> points_;
    std::chrono::steady_clock::time_point birth_;
};

#endif // SKETCH_H
