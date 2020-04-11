#ifndef OCR_H
#define OCR_H

#include <array>
#include <atomic>
#include <chrono>
#include <list>
#include <mutex>
#include <vector>
#include <thread>

#include "shape.h"
#include "sketch.h"

//#include <tesseract/baseapi.h>

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
    void simplify(const Job* job, std::vector<Shape*>& elements,
        const std::vector<std::array<int, 2>>& points,
        std::size_t begin, std::size_t count);
    std::vector<Shape*> combine(std::vector<Shape*>& elements);

private:
    //tesseract::TessBaseAPI ocr_;
    std::thread thread_;
    std::atomic<bool> run_;
    std::list<Job> jobs_;
    std::mutex jobs_lock_;
    Board* board_;
};

#endif // OCR_H
