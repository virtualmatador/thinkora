#include <numeric>

#include "ocr.h"

#include "guess.h"

Guess::Guess(std::shared_ptr<const Guess> parent, const Character* character,
    const Sketch* sketch, std::list<std::size_t>&& deficients, double diff)
    : parent_{ parent }
    , character_{ character }
    , extra_{ sketch }
    , deficients_ { std::move(deficients) }
    , diff_{ diff }
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
        auto child_guess = extend(
            character_, std::list<std::size_t>(deficients_), sketch, convexes);
        if (child_guess)
        {
            guesses.emplace_back(std::move(child_guess));
        }
        else
        {
            guesses.emplace_back(std::make_shared<Guess>(shared_from_this(),
                character_, sketch, std::list<std::size_t>(deficients_),
                diff_ + 1.0));
        }   
    }
    else
    {
        for (auto& character : Ocr::characters_)
        {
            std::list<std::size_t> deficients(character.get_convexes().size());
            std::iota(deficients.begin(), deficients.end(), std::size_t(0));
            auto child_guess = extend(
                &character, std::move(deficients), sketch, convexes);
            if (child_guess)
            {
                guesses.emplace_back(std::move(child_guess));
            }
        }
        if (guesses.empty())
        {
            guesses.emplace_back(
                std::make_shared<Guess>(shared_from_this(), nullptr, sketch,
                std::list<std::size_t>(), diff_ + 1.0));
        }
    }
    return guesses;
}

std::shared_ptr<const Guess> Guess::extend(
    const Character* character, std::list<std::size_t>&& deficients,
    const Sketch* sketch, const std::vector<Convex>& convexes) const
{
    double diffs = 0.0;
    for (const auto& convex : convexes)
    {
        Convex inverted_convex = convex;
        inverted_convex.invert();
        bool found = false;
        for (auto it = deficients.begin(); it != deficients.end();)
        {
            // TODO check if sizes are close &&
            // TODO check position is on cursor, apply to diff or discard
            auto diff = character->get_convexes()
                [*it].compare(convex);
            if (diff > 0.4)
            {
                diff = character->get_convexes()
                    [*it].compare(inverted_convex);
            }
            if (diff <= 0.4)
            {
                diffs += diff;
                it = deficients.erase(it);
                found = true;
                break;
            }
            else
            {
                ++it;
            }
        }
        if (!found)
        {
            auto fault =
                get_distance(convex.get_frame()[0], convex.get_frame()[1]) /
                get_distance(sketch->get_frame()[0], sketch->get_frame()[1]);
            if (fault < 0.2)
            {
                diffs += fault;
            }
            else
            {
                return nullptr;
            }
        }
    }
    return std::make_shared<Guess>(shared_from_this(), character, nullptr,
        std::move(deficients), diff_ + diffs / convexes.size());
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

std::shared_ptr<Guess> Guess::head()
{
    return std::make_shared<Guess>(nullptr, nullptr, nullptr,
        std::list<std::size_t>(), 0.0);
}
