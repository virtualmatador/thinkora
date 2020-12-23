#ifndef THINKORA_SRC_GUESS_H
#define THINKORA_SRC_GUESS_H

#include <memory>
#include <list>

#include "toolbox.h"

#include "character.h"
#include "convex.h"
#include "sketch.h"

class Guess : public std::enable_shared_from_this<Guess>
{
public:
    Guess(std::shared_ptr<const Guess> parent, const Character* character,
        const Sketch* sketch, std::vector<std::size_t>&& deficients,
        double value);
    ~Guess();
    std::list<std::shared_ptr<Guess>> extend(const Sketch* sketch,
        const Convex& convex) const;
    bool is_done();
    std::shared_ptr<const Guess> get_parent() const;
    double get_value() const;
    bool is_complete() const;
    const std::string& get_character() const;

private:
    std::list<std::shared_ptr<Guess>> extend(
        const Character* character, const std::vector<std::size_t>& deficients,
        const Sketch* sketch, const Convex& convex) const;

public:
    static std::shared_ptr<Guess> head();

private:
    std::shared_ptr<const Guess> parent_;
    const Character* character_;
    std::vector<std::size_t> deficients_;
    const Sketch* extra_;
    Point tl_min_;
    Point tl_max_;
    Point br_min_;
    Point br_max_;
    double value_;
};

#endif // THINKORA_SRC_GUESS_H
