#ifndef OCR_H
#define OCR_H

#include <atomic>
#include <list>
#include <mutex>
#include <thread>

#include "sketch.h"

//#include <tesseract/baseapi.h>

class Ocr
{
public:
    Ocr();
    ~Ocr();
    void push(std::pair<const Sketch*, int> sketch);

private:
    bool get_sketch();
    void process();
    void clear();

private:
    //tesseract::TessBaseAPI ocr_;
    std::thread thread_;
    std::atomic<bool> run_;
    std::list<std::pair<const Sketch*, int>> sketchs_;
    std::mutex lock_sketchs_;
};

#endif // OCR_H
