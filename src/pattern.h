#ifndef THINKORA_SRC_PATTERN_H
#define THINKORA_SRC_PATTERN_H

#include <array>
#include <vector>

#include "json.h"

#include "result.h"

#include "convex.h"

class Pattern
{
public:
    Pattern(const jsonio::json& pattern);
    ~Pattern();
    double match(const std::vector<std::vector<Convex>>& elements) const;
    bool is_simple() const;
    const std::string& get_character(std::size_t choice) const;

private:
    std::vector<std::vector<Convex>> segments_;
    std::vector<Result> results_;
};

#endif // THINKORA_SRC_PATTERN_H
