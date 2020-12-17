#ifndef THINKORA_SRC_GUESS_H
#define THINKORA_SRC_GUESS_H

#include <memory>
#include <list>

#include "toolbox.h"

#include "character.h"
#include "sketch.h"

class Guess : public std::enable_shared_from_this<Guess>
{
public:
    Guess(std::shared_ptr<Guess> parent, const Character* character,
        std::size_t index, const Rectangle& frame, double diff);
    ~Guess();
    std::list<std::shared_ptr<Guess>> extend(const Sketch& sketch,
        const std::list<std::pair<const Pattern&, double>>& patterns);
    bool is_done();
    std::shared_ptr<Guess> get_parent() const;
    const Rectangle& get_frame() const;
    double get_diff() const;
    bool is_complete() const;
    const std::string& get_character() const;

public:
    static std::shared_ptr<Guess> head();

private:
    std::shared_ptr<Guess> parent_;
    const Character* character_;
    std::size_t index_;
    Rectangle frame_;
    int top_min_;
    int top_max_;
    int bottom_min_;
    int bottom_max_;
    std::list<const Sketch*> unmatchs_;
    double diff_;
    bool done_;
};

#endif // THINKORA_SRC_GUESS_H
