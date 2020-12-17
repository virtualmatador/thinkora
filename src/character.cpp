#include "character.h"

Character::Character(const std::string& name, const jsonio::json& character)
    : name_{ name }
{
    character_ = character["character"].get_string();
    for (const auto& pattern : character["patterns"].get_array())
    {
        patterns_.push_back({
            pattern["name"].get_string(),
            {
                pattern["left"].get_double(),
                pattern["top"].get_double(),
                pattern["right"].get_double(),
                pattern["bottom"].get_double(),
            }
        });
    }
}

Character::~Character()
{
}

std::size_t Character::get_size() const
{
    return patterns_.size();
}

const std::vector<std::pair<std::string, Rectangle>>& Character::get_patterns() const
{
    return patterns_;
}

const std::string& Character::get_character() const
{
    return character_;
}

