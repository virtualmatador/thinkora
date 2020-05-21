#ifndef THINKORA_SRC_OCR_H
#define THINKORA_SRC_OCR_H

#include <array>
#include <atomic>
#include <chrono>
#include <filesystem>
#include <list>
#include <map>
#include <mutex>
#include <vector>
#include <thread>

#include "shape.h"
#include "sketch.h"
#include "pattern.h"

class Board;

class Job
{
public:
    int zoom_;
    std::array<std::array<int, 2>, 2> frame_;
    int line_width_;
    Gdk::RGBA color_;
    Shape::Style style_;
};

class Ocr
{
public:
    static const int delay_ms_ = 1500;

public:
    Ocr(Board* board);
    ~Ocr();
    std::vector<Pattern> read_patterns(std::filesystem::path path);
    void add(const Job& job);

private:
    bool get_sketch();
    bool process(const Job* job, std::vector<Sketch>& sketches,
        const std::array<std::array<int, 2>, 2>& frame);
    void simplify(Sketch& sketch, const std::array<std::array<int, 2>, 2>&
        frame, std::vector<std::vector<Convex>>& elements);
    std::vector<Shape*> match(const Job* job, const std::array<std::array
        <int, 2>, 2>& frame, std::vector<std::vector<Convex>>& elements);

private:
    std::thread thread_;
    std::atomic<bool> run_;
    std::list<Job> jobs_;
    std::mutex jobs_lock_;
    Board* board_;
    std::vector<Pattern> shape_patterns_;
    std::map<std::string, std::vector<Pattern>> char_patterns_;
};

#endif // THINKORA_SRC_OCR_H
