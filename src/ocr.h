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

#include "job.h"
#include "sketch.h"
#include "pattern.h"

class Board;

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

private:
    std::thread thread_;
    std::atomic<bool> run_;
    std::list<Job> jobs_;
    std::mutex jobs_lock_;
    Board* board_;
};

#endif // THINKORA_SRC_OCR_H
