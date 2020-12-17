#include "guess.h"

Guess::Guess(std::shared_ptr<Guess> parent, const Character* character,
        std::size_t index, const Rectangle& frame, double diff)
    : parent_{ parent }
    , character_{ character }
    , index_{ index }
    , frame_{ frame }
    , diff_{ diff }
    , done_{ false }
{
}

Guess::~Guess()
{
}

std::list<std::shared_ptr<Guess>> Guess::extend(const Sketch& sketch,
    const std::list<std::pair<const Pattern&, double>>& patterns)
{
    std::list<std::shared_ptr<Guess>> guesses;
    done_ = parent_.get();
    for(auto[pattern, diff] : patterns)
    {
        for (auto& [character, index] : pattern.get_characters())
        {
            if (index == index_ + 1)
            {
                if (is_complete() || &character == character_)
                {
                    if (!is_complete() || !character_ || (
                        // TODO check if sizes are close &&
                        character_->get_character().size() == 1 &&
                        character.get_character().size() == 1))
                    {
                        done_ = false;
                        // TODO check position is on cursor, apply to diff or discard
                    }
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
        unmatchs_.emplace_back(&sketch);
        diff_ += 1.0;
    }
    return guesses;
}

bool Guess::is_done()
{
    return done_;
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

const std::string& Guess::get_character() const
{
    return character_->get_character();
}

std::shared_ptr<Guess> Guess::head()
{
    return std::make_shared<Guess>(nullptr, nullptr,
        std::size_t(-1), Rectangle({0.0, 0.0, 0.0, 0.0}), 0.0);
}