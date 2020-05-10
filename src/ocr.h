#ifndef OCR_H
#define OCR_H

#include <array>
#include <atomic>
#include <chrono>
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
    Ocr(Board* board);
    ~Ocr();
    void add(const Job& job);

private:
    bool get_sketch();
    bool process(const Job* job, std::vector<Sketch>& sketches);
    void simplify(Sketch& sketch, std::vector<Shape*>& elements);
    std::vector<Shape*> combine(std::vector<Shape*>& elements);

private:
    std::thread thread_;
    std::atomic<bool> run_;
    std::list<Job> jobs_;
    std::mutex jobs_lock_;
    Board* board_;
    std::vector<Pattern> shape_patterns_;
    std::map<std::string, std::vector<Pattern>> char_patterns_;
};

#endif // OCR_H
