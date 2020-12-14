#include "guess.h"

Guess::Guess(Guess* parent, const Character* character, std::size_t index)
    : parent_{ parent }
    , character_{ character }
    , index_{ index }
    , diff_{ 0.0 }
{
}

Guess::~Guess()
{
}

Guess* Guess::extend(const std::string& pattern, const Sketch& sketch,
    double diff)
{
    //shared_from_this();
    // TODO extend branch
    return this;
}

double Guess::get_diff() const
{
    return diff_;
}

bool Guess::is_complete() const
{
    return !character_ || index_ == character_->get_size() - 1;
}

Guess* Guess::start_node()
{
    return new Guess(nullptr, nullptr, 0);
}
