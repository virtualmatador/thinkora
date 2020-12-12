#ifndef THINKORA_SRC_PATTERN_H
#define THINKORA_SRC_PATTERN_H

#include <array>
#include <string>
#include <vector>

#include "json.h"

#include "convex.h"
#include "fit.h"

class Sketch;

class Pattern
{
public:
    Pattern(const std::string& name, const jsonio::json& pattern);
    ~Pattern();
    double match(const std::vector<Convex>& convexes) const;

private:

private:
    std::string name_;
    std::vector<Convex> convexes_;
};

#endif // THINKORA_SRC_PATTERN_H
