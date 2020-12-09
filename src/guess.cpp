#include "guess.h"

Guess::Guess(Guess* parent)
    : parent_{ parent }
    , score_{ 1.0 }
{
}

Guess::~Guess()
{
}

void Guess::match(const std::vector<Fit>& fits, std::list<Guess>& results)
{
    // TODO extend branch
}

double Guess::get_score() const
{
    return score_;
}

bool Guess::is_complete() const
{
    return result_->is_complete_;
}

Guess Guess::start_node()
{
    return Guess(nullptr);
}
