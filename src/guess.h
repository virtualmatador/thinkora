#ifndef THINKORA_SRC_GUESS_H
#define THINKORA_SRC_GUESS_H

#include <list>

#include "pattern.h"
#include "result.h"
#include "sketch.h"

class Guess
{
public:
    Guess(Guess* parent);
    ~Guess();
    void match(const std::vector<Fit>& fits, std::list<Guess>& results);
    double get_score() const;
    bool is_complete() const;

public:
    static Guess start_node();

private:
    Guess* parent_;
    const Sketch* sketch_;
    const Result* result_;
    double score_;
};

#endif // THINKORA_SRC_GUESS_H
