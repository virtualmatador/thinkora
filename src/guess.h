#ifndef THINKORA_SRC_GUESS_H
#define THINKORA_SRC_GUESS_H

#include <cstddef>
#include <memory>
#include <list>
#include <vector>

#include "toolbox.h"

#include "character.h"
#include "convex.h"
#include "sketch.h"

class Guess : public std::enable_shared_from_this<Guess>
{
public:
    Guess(std::shared_ptr<const Guess> parent, const Character* character,
        std::list<const Sketch*>&& sources, const Sketch* extra,
        std::list<std::size_t>&& deficients, double diff,
        std::vector<double>&& rotations, double character_base_diff = 0.0,
        std::vector<CharacterPartMatch>&& character_parts = {});
    ~Guess();
    std::list<std::shared_ptr<const Guess>> extend(const Sketch* sketch,
        const std::vector<Convex>& convexes) const;
    bool is_done() const;
    std::shared_ptr<const Guess> get_parent() const;
    double get_diff() const;
    bool is_complete() const;
    const std::string& get_character() const;
    const Character* get_character_pattern() const;
    const std::list<const Sketch*>& get_sources() const;
    const Sketch* get_extra() const;
    double get_rotation() const;
    double get_missing_part_ratio() const;
    Rectangle get_frame() const;

private:
    std::list<std::shared_ptr<const Guess>> extend(
        const Character* character, std::list<std::size_t>&& deficients,
        const Sketch* sketch, const std::vector<Convex>& convexes,
        bool append_to_current) const;

public:
    static std::shared_ptr<Guess> head();

private:
    std::shared_ptr<const Guess> parent_;
    const Character* character_;
    std::list<const Sketch*> sources_;
    std::list<std::size_t> deficients_;
    const Sketch* extra_;
    Point tl_min_;
    Point tl_max_;
    Point br_min_;
    Point br_max_;
    double diff_;
    std::vector<double> rotations_;
    double rotation_;
    double character_base_diff_;
    std::vector<CharacterPartMatch> character_parts_;
};

#endif // THINKORA_SRC_GUESS_H
