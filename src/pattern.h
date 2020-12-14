#ifndef THINKORA_SRC_PATTERN_H
#define THINKORA_SRC_PATTERN_H

#include <string>
#include <vector>

#include "json.h"

#include "toolbox.h"

class Sketch;

class Pattern
{
public:
    Pattern(const std::string& name, const jsonio::json& pattern);
    ~Pattern();
    double match(
        const std::vector<Point>& points, const Rectangle& frame) const;
    const std::string& get_name() const;

private:
    double compare(const std::vector<Point>& points_a,
        const std::vector<Point>& points_b) const;

private:
    std::string name_;
    std::vector<Point> points_;
    Rectangle frame_;
};

#endif // THINKORA_SRC_PATTERN_H
