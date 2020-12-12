#ifndef THINKORA_SRC_GUESS_H
#define THINKORA_SRC_GUESS_H

#include <memory>
#include <list>

#include "pattern.h"
#include "result.h"
#include "sketch.h"

class Guess : std::enable_shared_from_this<Guess>
{
public:
    Guess(Guess* parent);
    ~Guess();
    Guess* extend(const Pattern& pattern, const Sketch& sketch,
        double similarity);
    double get_score() const;
    bool is_complete() const;

public:
    static Guess* start_node();

private:
    std::shared_ptr<Guess> parent_;
    Result* result_;
    std::size_t result_index_;
    int top_min_;
    int top_max_;
    int bottom_min_;
    int bottom_max_;
    double score_;
    std::list<Sketch*> noises_;
};

#endif // THINKORA_SRC_GUESS_H
