#ifndef THINKORA_SRC_CONVEX_H
#define THINKORA_SRC_CONVEX_H

#include <array>
#include <vector>

#include "toolbox.h"

struct ConvexData {
  Rectangle frame;
  double b_a;
  double b_x;
  double b_y;
  double e_a;
  double e_x;
  double e_y;
  double d_a;
  int n_b;
  int n_e;
};

struct ConvexMatch {
  double diff;
  double rotation;
};

class Convex {
public:
  Convex(const ConvexData &data);
  Convex(const std::vector<Point> &points);
  ~Convex();
  void invert();
  double compare(const Convex &convex) const;
  double compare(const Convex &convex, double rotation) const;
  ConvexMatch compare_best_rotation(const Convex &convex) const;
  const Rectangle &get_frame() const;

public:
  static std::vector<Convex> get_convexes(const std::vector<Point> &points);

private:
  Rectangle frame_;
  double b_a_;
  double b_x_;
  double b_y_;
  double e_a_;
  double e_x_;
  double e_y_;
  double d_a_;
  int n_b_;
  int n_e_;
};

#endif // THINKORA_SRC_CONVEX_H
