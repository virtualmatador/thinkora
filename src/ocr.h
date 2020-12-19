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
    void run();
    std::list<std::shared_ptr<Guess>> extend(const Sketch* sketch,
        const std::list<std::pair<const Pattern&, double>>& patterns);
    template<class T>
    static std::vector<T> read_json(const std::string& folder);
    static std::vector<Pattern> link_patterns(std::vector<Pattern> patterns,
        const std::vector<Character>& characters);

private:
    const std::vector<Character> characters_;
    const std::vector<Pattern> patterns_;
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
