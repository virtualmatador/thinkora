#ifndef THINKORA_SRC_JOB_H
#define THINKORA_SRC_JOB_H

#include <array>
#include <cstddef>
#include <vector>

#include <gtkmm.h>

#include "convex.h"
#include "pattern.h"
#include "shape.h"
#include "sketch.h"

class Job
{
public:
    Job(int zoom, std::array<std::array<int, 2>, 2> frame, int line_width,
        Gdk::RGBA color, Shape::Style style);
    ~Job();
    void reset_outer_frame();
    const std::array<std::array<int, 2>, 2>& get_frame() const;
    const int& get_zoom() const;
    bool match_style(const Shape* shape) const;
    void inflate(const std::array<std::array<int, 2UL>, 2UL> frame);
    void set_sketches(std::vector<Sketch>&& sketches);
    void process();

private:
    std::vector<Convex> simplify(Sketch& sketch);
    void match();

private:
    int zoom_;
    std::array<std::array<int, 2>, 2> frame_;
    int line_width_;
    Gdk::RGBA color_;
    Shape::Style style_;
    std::vector<Sketch> sketches_;
    std::array<std::array<int, 2>, 2> outer_frame_;
    std::vector<std::vector<Convex>> elements_;
    const Pattern* pattern_;
    std::size_t choice_;

private:
    static std::vector<Pattern> shape_patterns_;
    static std::map<std::string, std::vector<Pattern>> char_patterns_;

private:
    friend class Ocr;
};

#endif // THINKORA_SRC_JOB_H
