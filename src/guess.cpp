#include "guess.h"

Guess::Guess(Guess* parent)
    : parent_{ parent }
    , score_{ 1.0 }
{
}

Guess::~Guess()
{
}

Guess* Guess::extend(const Pattern& pattern, const Sketch& sketch,
    double similarity)
{
    shared_from_this();
    // TODO extend branch
    return this;
}

double Guess::get_score() const
{
    return score_;
}

bool Guess::is_complete() const
{
    return result_index_ == result_->get_size() - 1;
}

Guess* Guess::start_node()
{
    return new Guess(nullptr);
}
