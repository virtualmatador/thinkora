#include "pattern.h"

Pattern::Pattern(const std::string& character, const jsonio::json& pattern)
{
    auto length = character.find('-');
    if (length != std::string::npos)
    {
        character_ = {character.begin(), character.begin() + length};
        for (const auto& json_segment: pattern.get_array())
        {
            std::vector<Convex> segment;
            for (const auto& json_convex: json_segment.get_array())
            {
                segment.emplace_back(json_convex.get_object());
            }
            segments_.emplace_back(segment);
        }
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
    double difference = 0;
    std::size_t convex_count = 0;
    for (std::size_t i = 0; i < segments_.size(); ++i)
    {
        for (std::size_t j = 0; j < segments_[i].size(); ++j)
        {
            difference += segments_[i][j].compare(elements[i][j]);
        }
        convex_count += segments_[i].size();
    }
    return difference / convex_count;
}

const std::string& Pattern::get_character() const
{
    return character_;
}
