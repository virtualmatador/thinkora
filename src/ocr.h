#ifndef THINKORA_SRC_OCR_H
#define THINKORA_SRC_OCR_H

#include <atomic>
#include <condition_variable>
#include <filesystem>
#include <list>
#include <mutex>
#include <thread>

#include "guess.h"
#include "pattern.h"
#include "sketch.h"

class Board;

class Ocr
{
public:
    Ocr(Board& board);
    ~Ocr();
    void add(const Sketch* sketch);
    void cancel();

private:
    std::vector<Pattern> read_patterns(std::filesystem::path path);
    void run();
    void apply(const Guess& guess);

private:
    std::vector<Pattern> patterns_;
    std::thread thread_;
    std::atomic<bool> run_;
    std::list<const Sketch*> jobs_;
    decltype(jobs_)::iterator progress_;
    std::list<Guess> guesses_;
    std::condition_variable jobs_condition_;
    std::mutex jobs_lock_;
    std::mutex working_lock_;
    Board& board_;
};

#endif // THINKORA_SRC_OCR_H
