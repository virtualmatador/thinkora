#ifndef THINKORA_SRC_CHARACTER_H
#define THINKORA_SRC_CHARACTER_H

#include <cstddef>
#include <list>
#include <string>
#include <vector>

#include "convex.h"
#include "toolbox.h"

struct CharacterPartMatch {
  bool matched;
  std::size_t index;
  Convex observed;
  double shape_diff;
  double rotation;
  double fault_diff;
};

struct CharacterMatch {
  double diff;
  double score_sum;
  std::size_t score_count;
  double rotation;
  std::list<std::size_t> deficients;
  std::vector<double> rotations;
  std::vector<CharacterPartMatch> parts;
};

class Character {
public:
  Character(std::string name, std::vector<Convex> convexes);
  ~Character();
  const std::string &get_name() const;
  const std::vector<Convex> &get_convexes() const;
  const Rectangle &get_frame() const;
  std::vector<CharacterMatch>
  match(const std::list<std::size_t> &deficients,
        const std::vector<double> &rotations,
        const std::vector<Convex> &observed,
        const Rectangle &candidate_frame, double match_threshold,
        double fault_threshold, double fault_diameter,
        std::vector<CharacterPartMatch> initial_parts = {}) const;

private:
  std::string name_;
  std::vector<Convex> convexes_;
  Rectangle frame_;
};

#endif // THINKORA_SRC_CHARACTER_H
