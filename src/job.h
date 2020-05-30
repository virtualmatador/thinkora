#ifndef THINKORA_SRC_JOB_H
#define THINKORA_SRC_JOB_H

#include <array>
#include <cstddef>
#include <mutex>
#include <vector>

#include <gtkmm.h>

#include "convex.h"
#include "pattern.h"
#include "shape.h"

class Sketch;

class Job
{
public:
    Job(const Sketch* sketch);
    ~Job();
    const std::array<std::array<int, 2>, 2>& get_frame() const;
    const int& get_zoom() const;
    void process();
    bool is_simple() const;
    Shape* get_result();

private:
    void simplify();
    void match();

private:
    const Sketch* sketch_;
    std::mutex sketch_lock_;
    std::vector<std::array<int, 2>> points_;
    std::vector<Convex> convexes_;
    const Pattern* pattern_;
    std::size_t choice_;

private:
    static std::vector<Pattern> shape_patterns_;
    static std::map<std::string, std::vector<Pattern>> char_patterns_;

private:
    friend class Ocr;
};

#endif // THINKORA_SRC_JOB_H
