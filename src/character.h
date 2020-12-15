#ifndef THINKORA_SRC_CHARACTER_H
#define THINKORA_SRC_CHARACTER_H

#include <string>
#include <vector>
#include <tuple>

#include <json.h>

#include "toolbox.h"

class Character
{
public:
    Character(const std::string& name, const jsonio::json& character);
    ~Character();
    std::size_t get_size() const;
    const std::vector<std::pair<std::string, Rectangle>>& get_patterns() const;
    char get_character() const;

private:
    std::string name_;
    std::vector<std::pair<std::string, Rectangle>> patterns_;
    char character_;
};
    
#endif // THINKORA_SRC_CHARACTER_H
