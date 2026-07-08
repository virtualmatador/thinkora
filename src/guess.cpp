#include <algorithm>
#include <cmath>
#include <limits>
#include <numbers>
#include <numeric>
#include <vector>

#include "ocr.h"

#include "guess.h"

namespace
{
constexpr double minimum_dimension = 1e-6;
constexpr double match_threshold = 0.75;
constexpr double fault_threshold = 0.2;
constexpr double rotation_consistency_slack = std::numbers::pi / 36.0;
constexpr double rotation_consistency_unit = std::numbers::pi / 4.0;
constexpr double word_rotation_weight = 0.25;

struct FlowEntry
{
    const Character* character;
    Rectangle frame;
    double rotation;
};

double width(const Rectangle& frame)
{
    return frame[1][0] - frame[0][0];
}

double height(const Rectangle& frame)
{
    return frame[1][1] - frame[0][1];
}

Point center(const Rectangle& frame)
{
    return
    {
        (frame[0][0] + frame[1][0]) / 2.0,
        (frame[0][1] + frame[1][1]) / 2.0,
    };
}

double dot(const Point& first, const Point& second)
{
    return first[0] * second[0] + first[1] * second[1];
}

Point subtract(const Point& first, const Point& second)
{
    return { first[0] - second[0], first[1] - second[1] };
}

void include_frame(Rectangle& destination, const Rectangle& source)
{
    destination[0][0] = std::min(destination[0][0], source[0][0]);
    destination[0][1] = std::min(destination[0][1], source[0][1]);
    destination[1][0] = std::max(destination[1][0], source[1][0]);
    destination[1][1] = std::max(destination[1][1], source[1][1]);
}

Rectangle sources_frame(const std::list<const Sketch*>& sources,
    const Sketch* sketch)
{
    Rectangle frame = empty_frame();
    for (auto source : sources)
    {
        include_frame(frame, source->get_frame());
    }
    if (sketch)
    {
        include_frame(frame, sketch->get_frame());
    }
    return frame;
}

double average_rotation(const std::vector<double>& rotations)
{
    if (rotations.empty())
    {
        return 0.0;
    }

    double rotation_x = 0.0;
    double rotation_y = 0.0;
    for (double rotation : rotations)
    {
        rotation_x += std::cos(rotation);
        rotation_y += std::sin(rotation);
    }
    return std::atan2(rotation_y, rotation_x);
}

void collect_flow_entries(const Guess* guess, std::vector<FlowEntry>& entries)
{
    if (!guess)
    {
        return;
    }
    collect_flow_entries(guess->get_parent().get(), entries);
    if (guess->is_complete())
    {
        entries.emplace_back(guess->get_character_pattern(),
            guess->get_frame(), guess->get_rotation());
    }
}

double character_scale(const Character& character, const Rectangle& frame)
{
    return height(frame) / std::max(height(character.get_frame()), 0.05);
}

double flow_score(const Guess* guess, const Character& character,
    const Rectangle& candidate_frame, double candidate_rotation)
{
    std::vector<FlowEntry> entries;
    collect_flow_entries(guess, entries);
    if (entries.empty())
    {
        return 0.0;
    }

    std::vector<double> scales;
    scales.reserve(entries.size());
    for (const auto& entry : entries)
    {
        scales.emplace_back(character_scale(*entry.character, entry.frame));
    }

    double expected_scale = scales.back();
    if (scales.size() >= 2)
    {
        double growth = scales.back() - scales[scales.size() - 2];
        double growth_limit = std::max(expected_scale * 0.35, 1.0);
        expected_scale += std::clamp(growth, -growth_limit, growth_limit);
    }
    double scale = character_scale(character, candidate_frame);
    double size_score = std::abs(std::log(
        std::max(scale, minimum_dimension) /
        std::max(expected_scale, minimum_dimension)));

    double layout_score = 0.0;
    auto last_center = center(entries.back().frame);
    auto current_center = center(candidate_frame);
    Point advance { 1.0, 0.0 };
    if (entries.size() >= 2)
    {
        auto first_center = center(entries.front().frame);
        auto vector = subtract(last_center, first_center);
        auto length = get_distance(first_center, last_center);
        if (length > minimum_dimension)
        {
            advance = { vector[0] / length, vector[1] / length };
        }
    }
    double along = dot(subtract(current_center, last_center), advance);
    double expected_width = width(entries.back().frame);
    if (along < -expected_width * 0.2)
    {
        layout_score += std::abs(along) / std::max(expected_width, 1.0);
    }

    std::vector<double> word_rotations;
    word_rotations.reserve(entries.size());
    for (const auto& entry : entries)
    {
        word_rotations.emplace_back(entry.rotation);
    }
    double word_rotation = average_rotation(word_rotations);
    double rotation_score =
        std::max(0.0,
            std::abs(get_rotation(word_rotation, candidate_rotation)) -
                rotation_consistency_slack) /
        rotation_consistency_unit;
    return size_score * 0.35 + layout_score * 0.25 +
        rotation_score * word_rotation_weight;
}
}

Guess::Guess(std::shared_ptr<const Guess> parent, const Character* character,
    std::list<const Sketch*>&& sources, const Sketch* extra,
    std::list<std::size_t>&& deficients, double diff,
    std::vector<double>&& rotations, double character_base_diff,
    std::vector<CharacterPartMatch>&& character_parts)
    : parent_{ parent }
    , character_{ character }
    , sources_{ std::move(sources) }
    , deficients_ { std::move(deficients) }
    , extra_{ extra }
    , diff_{ diff }
    , rotations_{ std::move(rotations) }
    , rotation_{ average_rotation(rotations_) }
    , character_base_diff_{ character_base_diff }
    , character_parts_{ std::move(character_parts) }
{
}

Guess::~Guess()
{
}

std::list<std::shared_ptr<const Guess>> Guess::extend(const Sketch* sketch,
    const std::vector<Convex>& convexes) const
{
    std::list<std::shared_ptr<const Guess>> guesses;
    if (character_ && !deficients_.empty())
    {
        auto child_guesses = extend(
            character_, std::list<std::size_t>(deficients_), sketch, convexes,
            true);
        if (!child_guesses.empty())
        {
            guesses.splice(guesses.end(), child_guesses);
        }
        else
        {
            guesses.emplace_back(std::make_shared<Guess>(shared_from_this(),
                character_, std::list<const Sketch*>(sources_), sketch,
                std::list<std::size_t>(deficients_), diff_ + 1.0,
                std::vector<double>(rotations_), character_base_diff_ + 1.0,
                std::vector<CharacterPartMatch>(character_parts_)));
        }   
    }
    else
    {
        for (auto& character : Ocr::characters_)
        {
            std::list<std::size_t> deficients(character.get_convexes().size());
            std::iota(deficients.begin(), deficients.end(), std::size_t(0));
            auto child_guesses = extend(
                &character, std::move(deficients), sketch, convexes, false);
            if (!child_guesses.empty())
            {
                guesses.splice(guesses.end(), child_guesses);
            }
        }
        if (guesses.empty())
        {
            guesses.emplace_back(
                std::make_shared<Guess>(shared_from_this(), nullptr,
                std::list<const Sketch*>(), sketch, std::list<std::size_t>(),
                diff_ + 1.0, std::vector<double>()));
        }
    }
    return guesses;
}

std::list<std::shared_ptr<const Guess>> Guess::extend(
    const Character* character, std::list<std::size_t>&& deficients,
    const Sketch* sketch, const std::vector<Convex>& convexes,
    bool append_to_current) const
{
    std::list<std::shared_ptr<const Guess>> guesses;
    std::list<const Sketch*> sources;
    if (append_to_current)
    {
        sources = sources_;
    }
    sources.emplace_back(sketch);
    std::vector<double> rotations;
    if (append_to_current)
    {
        rotations = rotations_;
    }
    const double character_base_diff =
        append_to_current ? character_base_diff_ : diff_;
    std::vector<CharacterPartMatch> initial_parts;
    if (append_to_current)
    {
        initial_parts = character_parts_;
    }
    auto candidate_frame = sources_frame(sources, nullptr);
    auto sketch_diameter =
        get_distance(sketch->get_frame()[0], sketch->get_frame()[1]);
    auto matches = character->match(deficients, rotations, convexes,
        candidate_frame, match_threshold, fault_threshold, sketch_diameter,
        std::move(initial_parts));
    for (auto& match : matches)
    {
        double diff = match.diff;
        if (match.deficients.empty())
        {
            diff += flow_score(append_to_current ? parent_.get() : this,
                *character, candidate_frame, match.rotation);
        }
        guesses.emplace_back(std::make_shared<Guess>(shared_from_this(),
            character, std::list<const Sketch*>(sources), nullptr,
            std::move(match.deficients), character_base_diff + diff,
            std::move(match.rotations), character_base_diff,
            std::move(match.parts)));
    }
    return guesses;
}

bool Guess::is_done() const
{
    return extra_;
}

std::shared_ptr<const Guess> Guess::get_parent() const
{
    return parent_;
}

double Guess::get_diff() const
{
    return diff_;
}

bool Guess::is_complete() const
{
    return character_ && deficients_.empty();
}

const std::string& Guess::get_character() const
{
    return character_->get_name();
}

const Character* Guess::get_character_pattern() const
{
    return character_;
}

const std::list<const Sketch*>& Guess::get_sources() const
{
    return sources_;
}

const Sketch* Guess::get_extra() const
{
    return extra_;
}

double Guess::get_rotation() const
{
    return rotation_;
}

double Guess::get_missing_part_ratio() const
{
    if (!character_ || character_->get_convexes().empty())
    {
        return 0.0;
    }
    return double(deficients_.size()) / character_->get_convexes().size();
}

Rectangle Guess::get_frame() const
{
    return sources_frame(sources_, nullptr);
}

std::shared_ptr<Guess> Guess::head()
{
    return std::make_shared<Guess>(nullptr, nullptr,
        std::list<const Sketch*>(), nullptr, std::list<std::size_t>(), 0.0,
        std::vector<double>());
}
