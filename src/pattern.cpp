#include "pattern.h"

Pattern::Pattern(const std::string& name, const jsonio::json& pattern)
    : name_{ name }
{
    for (const auto& json_convex: pattern.get_array())
    {
        convexes_.emplace_back(json_convex.get_object());
    }
}

Pattern::~Pattern()
{
}

double Pattern::match(const std::vector<Convex>& convexes) const
{
    if (convexes_.size() != convexes.size())
    {
        return 1.0;
    }
    double total_similarity = 0;
    for (std::size_t i = 0; i < convexes_.size(); ++i)
    {
        auto similarity = convexes_[i].compare(convexes[i]);
        if (similarity > 0.6)
        {
            total_similarity += similarity;
        }
        else
        {
            return 0.0;
        }
    }
    return total_similarity / convexes_.size();
}
