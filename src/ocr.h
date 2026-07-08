#ifndef THINKORA_SRC_OCR_H
#define THINKORA_SRC_OCR_H

#include <atomic>
#include <condition_variable>
#include <list>
#include <memory>
#include <mutex>
#include <thread>
#include <vector>

#include "character.h"
#include "guess.h"
#include "shape.h"
#include "sketch.h"

class Board;

class Ocr
{
public:
    static void read_characters();
    static void read_shapes();

public:
    Ocr(Board& board);
    ~Ocr();
    void add(const Sketch* sketch);
    void finish();
    void apply();

private:
    void run();
    void apply(bool include_shapes);
    bool check_apply(const std::list<std::shared_ptr<const Guess>>& guesses);
    std::list<std::shared_ptr<const Guess>> extend(const Sketch* sketch,
        const std::vector<Convex>& convexes);
    void add_shape_source(const Sketch* sketch,
        const std::vector<Convex>& convexes);
    void clear_shape_sources();

public:
    static std::vector<Character> characters_;
    static std::vector<Character> shapes_;

private:
    std::thread thread_;
    std::atomic<bool> run_;
    bool force_apply_;
    std::condition_variable jobs_condition_;
    std::mutex jobs_lock_;
    std::list<const Sketch*> jobs_;
    std::mutex work_lock_;
    std::list<std::shared_ptr<const Guess>> guesses_;
    std::list<const Sketch*> shape_sources_;
    std::vector<Convex> shape_convexes_;
    std::shared_ptr<const Guess> head_;
    int zoom_;
    double width_;
    Gdk::RGBA color_;
    Shape::Style style_;
    Board& board_;
};

#endif // THINKORA_SRC_OCR_H
