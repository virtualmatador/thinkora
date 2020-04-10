#ifndef OCR_H
#define OCR_H

#include <array>
#include <atomic>
#include <chrono>
#include <list>
#include <mutex>
#include <thread>

#include "sketch.h"

//#include <tesseract/baseapi.h>

class Board;

class Ocr
{
public:
    Ocr(Board* board);
    ~Ocr();
    void add(const int& zoom, const std::array<std::array<int, 2>, 2>& frame);

private:
    bool get_sketch();
    bool process(const int& zoom, const std::array<std::array<int, 2>, 2>&
        frame, std::vector<Sketch>& sketches);

private:
    //tesseract::TessBaseAPI ocr_;
    std::thread thread_;
    std::atomic<bool> run_;
    std::list<std::pair<int, std::array<std::array<int, 2>, 2>>> jobs_;
    std::mutex jobs_lock_;
    Board* board_;
};

#endif // OCR_H
