#ifndef THINKORA_SRC_OCR_H
#define THINKORA_SRC_OCR_H

#include <atomic>
#include <condition_variable>
#include <filesystem>
#include <list>
#include <mutex>
#include <thread>

#include "character.h"
#include "guess.h"
#include "pattern.h"
#include "shape.h"
#include "sketch.h"

class Board;

class Ocr
{
public:
    Ocr(Board& board);
    ~Ocr();
    void add(const Sketch* sketch);
    void cancel();
    void apply();

private:
    template<class T>
    static std::vector<T> read_json(const std::filesystem::path& path);
    void run();

private:
    const std::vector<Pattern> patterns_;
    const std::vector<Character> characters_;
    std::thread thread_;
    std::atomic<bool> run_;
    std::condition_variable jobs_condition_;
    std::mutex jobs_lock_;
    std::list<const Sketch*> jobs_;
    std::mutex working_lock_;
    std::list<Guess*> guesses_;
    std::list<const Sketch*> sources_;
    std::list<Shape*> results_;
    int zoom_;
    double width_;
    Gdk::RGBA color_;
    Shape::Style style_;
    Board& board_;
};

#endif // THINKORA_SRC_OCR_H
