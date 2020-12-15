#include "guess.h"

Guess::Guess(std::shared_ptr<Guess> parent, const Character* character,
        std::size_t index, const Rectangle& frame, double diff)
    : parent_{ parent }
    , character_{ character }
    , index_{ index }
    , frame_{ frame }
    , diff_{ diff }
{
}

Guess::~Guess()
{
}

std::list<std::shared_ptr<Guess>> Guess::extend(const Sketch& sketch,
    const std::list<std::pair<const Pattern&, double>>& patterns)
{
    std::list<std::shared_ptr<Guess>> guesses;
    for(auto[pattern, diff] : patterns)
    {
        for (auto& [character, index] : pattern.get_characters())
        {
            if (index == index_ + 1)
            {
                if (is_complete() || &character == character_)
                {
                    // TODO check position apply to diff
                    auto frame = sketch.get_frame();
                    auto normalized_index = index;
                    if (normalized_index == character.get_size() - 1)
                    {
                        normalized_index = std::size_t(-1);
                    }
                    guesses.emplace_back(
                        std::make_shared<Guess>(shared_from_this(),
                        &character, normalized_index, frame, diff_ + diff));
                }
            }
        }
    }
    if (guesses.empty())
    {
        diff_ += 1.0;
    }
    return guesses;
}

std::shared_ptr<Guess> Guess::get_parent() const
{
    return parent_;
}

const Rectangle& Guess::get_frame() const
{
    return frame_;
}

double Guess::get_diff() const
{
    return diff_;
}

bool Guess::is_complete() const
{
    return index_ == std::size_t(-1);
}

char Guess::get_character() const
{
    return character_->get_character();
}
