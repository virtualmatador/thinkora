#include <numeric>

#include "ocr.h"

#include "guess.h"

Guess::Guess(std::shared_ptr<const Guess> parent, const Character* character,
    const Sketch* sketch, std::vector<std::size_t>&& deficients, double value)
    : parent_{ parent }
    , character_{ character }
    , extra_{ sketch }
    , deficients_ { std::move(deficients) }
    , value_{ value }
{
}

Guess::~Guess()
{
}

std::list<std::shared_ptr<Guess>> Guess::extend(const Sketch* sketch,
    const Convex& convex) const
{
    if (character_ && !deficients_.empty())
    {
        return extend(character_, deficients_, sketch, convex);
    }
    std::list<std::shared_ptr<Guess>> guesses;
    for (auto& character : Ocr::characters_)
    {
        std::vector<std::size_t> deficients(character.get_segments().size());
        std::iota(deficients.begin(), deficients.end(), std::size_t(0));
        guesses.merge(extend(&character, deficients, sketch, convex));
    }
    if (guesses.empty())
    {
        guesses.emplace_back(
            std::make_shared<Guess>(shared_from_this(), nullptr, sketch,
            std::vector<std::size_t>(), value_ - 1.0));
    }
    return guesses;
}

std::list<std::shared_ptr<Guess>> Guess::extend(
    const Character* character, const std::vector<std::size_t>& deficients,
    const Sketch* sketch, const Convex& convex) const
{
    std::list<std::shared_ptr<Guess>> guesses;
    Convex inverted_convex = convex;
    inverted_convex.invert();
    for (const auto& deficient : deficients)
    {
        // TODO check if sizes are close &&
        // TODO check position is on cursor, apply to diff or discard
        auto similarity = character->get_segments()
            [deficient].first.compare(convex);
        if (similarity < 0.6)
        {
            similarity = character->get_segments()
                [deficient].first.compare(inverted_convex);
        }
        if (similarity >= 0.6)
        {
            std::vector<std::size_t> new_deficients;
            for (const auto& new_deficient : deficients)
            {
                if (new_deficient != deficient)
                {
                    new_deficients.push_back(new_deficient);
                }
            }
            guesses.emplace_back(
                std::make_shared<Guess>(shared_from_this(), character,
                nullptr, std::move(new_deficients), value_ + similarity));
        }
    }
    return guesses;
}

bool Guess::is_done()
{
    return extra_ && parent_.get();
}

std::shared_ptr<const Guess> Guess::get_parent() const
{
    return parent_;
}

double Guess::get_value() const
{
    return value_;
}

bool Guess::is_complete() const
{
    return deficients_.empty();
}

const std::string& Guess::get_character() const
{
    return character_->get_name();
}

std::shared_ptr<Guess> Guess::head()
{
    return std::make_shared<Guess>(nullptr, nullptr, nullptr,
        std::vector<std::size_t>(), 0.0);
}
