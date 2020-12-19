#ifndef THINKORA_SRC_PATTERN_H
#define THINKORA_SRC_PATTERN_H

#include <string>
#include <vector>

#include "json.h"

#include "character.h"
#include "toolbox.h"

class Sketch;

class Pattern
{
public:
    Pattern(const std::string& name, const jsonio::json& pattern);
    ~Pattern();
    void add_character(const Character& character, std::size_t index);
    const std::string& get_name() const;
    const std::vector<std::pair<const Character&, std::size_t>>&
        get_characters() const;
    double match(
        const std::vector<Point>& points, const Rectangle& frame) const;

private:
    std::string name_;
    std::vector<Point> points_;
    Rectangle frame_;
    std::vector<std::pair<const Character&, std::size_t>> characters_;
};

#endif // THINKORA_SRC_PATTERN_H
