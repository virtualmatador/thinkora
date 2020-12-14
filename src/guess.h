#ifndef THINKORA_SRC_GUESS_H
#define THINKORA_SRC_GUESS_H

#include <memory>
#include <list>

#include "character.h"
#include "sketch.h"

class Guess : std::enable_shared_from_this<Guess>
{
public:
    Guess(Guess* parent, const Character* character, std::size_t index);
    ~Guess();
    Guess* extend(const std::string& pattern, const Sketch& sketch,
        double diff);
    double get_diff() const;
    bool is_complete() const;

public:
    static Guess* start_node();

private:
    std::shared_ptr<Guess> parent_;
    const Character* character_;
    std::size_t index_;
    std::list<const Sketch*> noises_;
    int top_min_;
    int top_max_;
    int bottom_min_;
    int bottom_max_;
    double diff_;
};

#endif // THINKORA_SRC_GUESS_H
