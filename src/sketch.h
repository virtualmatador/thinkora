#ifndef THINKORA_SRC_SKETCH_H
#define THINKORA_SRC_SKETCH_H

#include <array>
#include <chrono>
#include <cstddef>
#include <memory>
#include <vector>

#include "job.h"
#include "shape.h"

class Sketch: public Shape
{
public:
    using Shape::Shape;
    ~Sketch();
    void set_sketch();
    void add_point(const std::array<int, 2>& point);
    void set_birth(const std::chrono::steady_clock::time_point& birth);
    std::vector<std::array<int, 2>>& get_points();
    const std::chrono::steady_clock::time_point& get_birth() const;

public:
    Type get_type() const override;

private:
    void draw_details(const Cairo::RefPtr<Cairo::Context>& cr,
        const int& zoom_delta, const std::array<int, 2>& pad) const;
    void write_dtails(std::ostream& os) const;
    void read_details(std::istream& is);

private:
    std::vector<std::array<int, 2>> points_;
    std::chrono::steady_clock::time_point birth_;
    std::shared_ptr<Job> job_;
};

#endif // THINKORA_SRC_SKETCH_H
