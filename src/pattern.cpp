#include "pattern.h"

Pattern::Pattern(const jsonio::json& pattern)
{
    for (const auto& json_segment: pattern["in"].get_array())
    {
        std::vector<Convex> segment;
        for (const auto& json_convex: json_segment.get_array())
        {
            segment.emplace_back(json_convex.get_object());
        }
        segments_.emplace_back(segment);
    }
    for (const auto& json_result: pattern["out"].get_array())
    {
        results_.emplace_back(json_result.get_object());
    }
}

Pattern::~Pattern()
{
}

double Pattern::match(const std::vector<std::vector<Convex>>& elements) const
{
    if (segments_.size() != elements.size())
    {
        return 1.0;
    }
    for (std::size_t i = 0; i < segments_.size(); ++i)
    {
        if (segments_[i].size() != elements[i].size())
        {
            return 1.0;
        }
    }
    double total_difference = 0;
    std::size_t convex_count = 0;
    for (std::size_t i = 0; i < segments_.size(); ++i)
    {
        for (std::size_t j = 0; j < segments_[i].size(); ++j)
        {
            auto difference = segments_[i][j].compare(elements[i][j]);
            if (difference < Convex::treshold_)
            {
                total_difference += difference;
            }
            else
            {
                return 1.0;
            }
        }
        convex_count += segments_[i].size();
    }
    return total_difference / convex_count;
}

bool Pattern::is_simple() const
{
    return results_.size() == 1;
}

const std::string& Pattern::get_character(std::size_t choice) const
{
    return results_[choice].character_;
}
