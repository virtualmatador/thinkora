#ifndef THINKORA_SRC_OCR_H
#define THINKORA_SRC_OCR_H

#include <array>
#include <atomic>
#include <chrono>
#include <filesystem>
#include <list>
#include <map>
#include <memory>
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
    void add(Sketch* sketch, const int& zoom);
    void cancel();

private:
    bool do_job();
    bool combine(std::shared_ptr<Job>& job,
        std::list<std::shared_ptr<Job>>& partial_jobs);

private:
    std::thread thread_;
    std::atomic<bool> run_;
    std::list<std::shared_ptr<Job>> jobs_;
    std::list<std::shared_ptr<Job>> partial_jobs_;
    std::mutex jobs_lock_;
    Board* board_;
};

#endif // THINKORA_SRC_OCR_H
