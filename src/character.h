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
    const std::vector<Convex>& get_convexes() const;

private:
    std::string name_;
    std::vector<Convex> convexes_;
};
    
#endif // THINKORA_SRC_CHARACTER_H
