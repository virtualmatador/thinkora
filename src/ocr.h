#ifndef THINKORA_SRC_OCR_H
#define THINKORA_SRC_OCR_H

#include <atomic>
#include <condition_variable>
#include <filesystem>
#include <list>
#include <memory>
#include <mutex>
#include <thread>

#include "character.h"
#include "guess.h"
#include "shape.h"
#include "sketch.h"

class Board;

class Ocr
{
public:
    static void read_characters();

public:
    Ocr(Board& board);
    ~Ocr();
    void add(const Sketch* sketch);
    void cancel();
    void apply();

private:
    void run();
    bool check_apply(const std::list<std::shared_ptr<Guess>>& guesses);
    std::list<std::shared_ptr<Guess>> extend(const Sketch* sketch,
        const Convex& convex);

public:
    static std::vector<Character> characters_;

private:
    std::thread thread_;
    std::atomic<bool> run_;
    std::condition_variable jobs_condition_;
    std::mutex jobs_lock_;
    std::list<const Sketch*> jobs_;
    std::mutex working_lock_;
    std::list<std::shared_ptr<Guess>> guesses_;
    int zoom_;
    double width_;
    Gdk::RGBA color_;
    Shape::Style style_;
    Board& board_;
};

#endif // THINKORA_SRC_OCR_H
