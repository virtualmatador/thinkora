#ifndef PATTERN_H
#define PATTERN_H

#include <array>
#include <vector>

#include "json.h"

#include "convex.h"

class Pattern
{
public:
    Pattern(const std::string& character, const jsonio::json& pattern);
    ~Pattern();
    double match(const std::vector<std::vector<Convex>>& elements) const;
    const std::string& get_character() const;

private:

private:
    std::string character_;
    std::vector<std::vector<Convex>> segments_;
};

#endif // PATTERN_H
