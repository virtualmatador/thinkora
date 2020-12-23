#ifndef THINKORA_SRC_CHARACTER_H
#define THINKORA_SRC_CHARACTER_H

#include <string>
#include <vector>
#include <tuple>

#include <json.h>

#include "convex.h"
#include "toolbox.h"

class Character
{
public:
    Character(const std::string& name, const jsonio::json& character);
    ~Character();
    const std::string& get_name() const;
    const std::vector<std::pair<Convex, Rectangle>>& get_segments() const;

private:
    std::string name_;
    std::vector<std::pair<Convex, Rectangle>> segments_;
};
    
#endif // THINKORA_SRC_CHARACTER_H
