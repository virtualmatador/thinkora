#include <algorithm>
#include <chrono>
#include <cmath>
#include <cstddef>
#include <iterator>
#include <limits>
#include <list>
#include <memory>
#include <numbers>
#include <numeric>
#include <vector>

#include "board.h"
#include "circle.h"
#include "dot.h"
#include "render.h"
#include "shape.h"
#include "shapes.h"
#include "text.h"
#include "toolbox.h"
#include "wire.h"

#include "characters.h"
#include "ocr.h"

namespace {
constexpr auto commit_delay = std::chrono::milliseconds(1500);
constexpr double minimum_dimension = 1e-6;
constexpr double match_threshold = 0.75;
constexpr double shape_fault_threshold = 0.15;
constexpr double snap_rotation_unit = std::numbers::pi / 4.0;
constexpr double snap_rotation_threshold = std::numbers::pi / 36.0;
constexpr double snap_aspect_threshold = 0.05;
constexpr double missing_part_commit_penalty = 0.75;

struct ShapeCandidate {
  bool valid = false;
  const Character *pattern = nullptr;
  double value = std::numeric_limits<double>::max();
  Point center{0.0, 0.0};
  double cx = 0.0;
  double cy = 0.0;
  double rotation = 0.0;
};

struct ProjectedBounds {
  double left = std::numeric_limits<double>::max();
  double top = std::numeric_limits<double>::max();
  double right = -std::numeric_limits<double>::max();
  double bottom = -std::numeric_limits<double>::max();
};

struct CleanupStroke {
  const Sketch *source = nullptr;
  std::vector<Point> points;
};

struct EndpointMatch {
  bool valid = false;
  std::size_t first = 0;
  std::size_t second = 0;
  bool first_front = false;
  bool second_front = false;
  double distance = std::numeric_limits<double>::max();
};

struct CircleCandidate {
  bool valid = false;
  Point center{0.0, 0.0};
  double radius = 0.0;
  double start_angle = 0.0;
  double end_angle = 0.0;
};

void include_frame(Rectangle &destination, const Rectangle &source) {
  destination[0][0] = std::min(destination[0][0], source[0][0]);
  destination[0][1] = std::min(destination[0][1], source[0][1]);
  destination[1][0] = std::max(destination[1][0], source[1][0]);
  destination[1][1] = std::max(destination[1][1], source[1][1]);
}

double width(const Rectangle &frame) { return frame[1][0] - frame[0][0]; }

double height(const Rectangle &frame) { return frame[1][1] - frame[0][1]; }

Point center(const Rectangle &frame) {
  return {
      (frame[0][0] + frame[1][0]) / 2.0,
      (frame[0][1] + frame[1][1]) / 2.0,
  };
}

double dot(const Point &first, const Point &second) {
  return first[0] * second[0] + first[1] * second[1];
}

Rectangle expanded(Rectangle frame, double ratio) {
  double pad_x = width(frame) * ratio;
  double pad_y = height(frame) * ratio;
  frame[0][0] -= pad_x;
  frame[0][1] -= pad_y;
  frame[1][0] += pad_x;
  frame[1][1] += pad_y;
  return frame;
}

bool contains(const Rectangle &frame, const Point &point) {
  return point[0] >= frame[0][0] && point[0] <= frame[1][0] &&
         point[1] >= frame[0][1] && point[1] <= frame[1][1];
}

Point middle(const Point &first, const Point &second) {
  return {
      (first[0] + second[0]) / 2.0,
      (first[1] + second[1]) / 2.0,
  };
}

void add_unique(std::list<const Sketch *> &sources, const Sketch *source) {
  if (std::find(sources.begin(), sources.end(), source) == sources.end()) {
    sources.emplace_back(source);
  }
}

double pending_penalty(std::shared_ptr<const Guess> guess) {
  double penalty = 0.0;
  std::list<const Sketch *> accounted;
  for (; guess; guess = guess->get_parent()) {
    if (guess->is_complete()) {
      for (auto source : guess->get_sources()) {
        add_unique(accounted, source);
      }
    } else {
      bool has_unaccounted_source = false;
      for (auto source : guess->get_sources()) {
        if (std::find(accounted.begin(), accounted.end(), source) ==
            accounted.end()) {
          has_unaccounted_source = true;
        }
      }
      if (has_unaccounted_source) {
        penalty += 0.65;
        penalty +=
            missing_part_commit_penalty * guess->get_missing_part_ratio();
        for (auto source : guess->get_sources()) {
          add_unique(accounted, source);
        }
      }
      if (guess->get_extra()) {
        penalty += 0.65;
      }
    }
  }
  return penalty;
}

Rectangle sources_frame(const std::list<const Sketch *> &sources) {
  Rectangle frame = empty_frame();
  for (auto source : sources) {
    include_frame(frame, source->get_frame());
  }
  return frame;
}

double snap_rotation(double rotation) {
  double snapped =
      std::round(rotation / snap_rotation_unit) * snap_rotation_unit;
  if (std::abs(get_rotation(snapped, rotation)) < snap_rotation_threshold) {
    return snapped;
  }
  return rotation;
}

double average_rotation(const std::vector<double> &rotations) {
  if (rotations.empty()) {
    return 0.0;
  }

  double rotation_x = 0.0;
  double rotation_y = 0.0;
  for (double rotation : rotations) {
    rotation_x += std::cos(rotation);
    rotation_y += std::sin(rotation);
  }
  return std::atan2(rotation_y, rotation_x);
}

void include_projected(ProjectedBounds &bounds, const Point &point,
                       double rotation) {
  const Point advance{std::cos(rotation), std::sin(rotation)};
  const Point height_axis{-std::sin(rotation), std::cos(rotation)};
  double x = dot(point, advance);
  double y = dot(point, height_axis);
  bounds.left = std::min(bounds.left, x);
  bounds.top = std::min(bounds.top, y);
  bounds.right = std::max(bounds.right, x);
  bounds.bottom = std::max(bounds.bottom, y);
}

ProjectedBounds project_sources(const std::list<const Sketch *> &sources,
                                double rotation) {
  ProjectedBounds bounds;
  for (auto source : sources) {
    for (const auto &point : source->get_points()) {
      include_projected(bounds, point, rotation);
    }
  }
  return bounds;
}

Rectangle projected_frame(const ProjectedBounds &bounds) {
  return {{
      {bounds.left, bounds.top},
      {bounds.right, bounds.bottom},
  }};
}

Point unproject(double x, double y, double rotation) {
  const Point advance{std::cos(rotation), std::sin(rotation)};
  const Point height_axis{-std::sin(rotation), std::cos(rotation)};
  return {
      advance[0] * x + height_axis[0] * y,
      advance[1] * x + height_axis[1] * y,
  };
}

void set_shape_frame(ShapeCandidate &candidate,
                     const std::list<const Sketch *> &sources) {
  auto bounds = project_sources(sources, candidate.rotation);
  const auto &pattern_frame = candidate.pattern->get_frame();
  double pattern_width = width(pattern_frame);
  double pattern_height = height(pattern_frame);
  double observed_width = std::max(bounds.right - bounds.left, 0.0);
  double observed_height = std::max(bounds.bottom - bounds.top, 0.0);
  double object_width = observed_width;
  double object_height = observed_height;
  if (pattern_width > minimum_dimension) {
    object_width /= pattern_width;
  }
  if (pattern_height > minimum_dimension) {
    object_height /= pattern_height;
  }

  double pattern_center_x = (pattern_frame[0][0] + pattern_frame[1][0]) / 2.0;
  double pattern_center_y = (pattern_frame[0][1] + pattern_frame[1][1]) / 2.0;
  double observed_center_x = (bounds.left + bounds.right) / 2.0;
  double observed_center_y = (bounds.top + bounds.bottom) / 2.0;
  double object_center_x =
      observed_center_x - (pattern_center_x - 0.5) * object_width;
  double object_center_y =
      observed_center_y - (pattern_center_y - 0.5) * object_height;

  candidate.center =
      unproject(object_center_x, object_center_y, candidate.rotation);
  candidate.cx = object_width / 2.0;
  candidate.cy = object_height / 2.0;
  if (candidate.cx > minimum_dimension && candidate.cy > minimum_dimension) {
    double ratio = candidate.cx / candidate.cy;
    if (std::abs(std::log(ratio)) < snap_aspect_threshold) {
      double c = (candidate.cx + candidate.cy) / 2.0;
      candidate.cx = c;
      candidate.cy = c;
    }
  }
}

ShapeCandidate match_shape(const Character &pattern,
                           const std::list<const Sketch *> &sources,
                           const std::vector<Convex> &convexes) {
  ShapeCandidate candidate;
  if (sources.empty() || convexes.empty() || pattern.get_convexes().empty()) {
    return candidate;
  }

  std::list<std::size_t> deficients(pattern.get_convexes().size());
  std::iota(deficients.begin(), deficients.end(), std::size_t(0));
  auto candidate_frame = sources_frame(sources);
  auto source_diameter = get_distance(candidate_frame[0], candidate_frame[1]);
  auto matches =
      pattern.match(deficients, {}, convexes, candidate_frame, match_threshold,
                    shape_fault_threshold, source_diameter);
  auto match = std::find_if(
      matches.begin(), matches.end(),
      [](const CharacterMatch &match) { return match.deficients.empty(); });
  if (match == matches.end()) {
    return candidate;
  }

  candidate.valid = true;
  candidate.pattern = &pattern;
  candidate.value = match->diff;
  candidate.rotation = match->rotation;
  candidate.rotation = snap_rotation(candidate.rotation);
  set_shape_frame(candidate, sources);
  return candidate;
}

ShapeCandidate best_shape(const std::list<const Sketch *> &sources,
                          const std::vector<Convex> &convexes) {
  ShapeCandidate best;
  for (const auto &pattern : Ocr::shapes_) {
    auto candidate = match_shape(pattern, sources, convexes);
    if (candidate.valid && candidate.value < best.value) {
      best = candidate;
    }
  }
  return best;
}

bool prefer_shape(const ShapeCandidate &shape, bool has_text,
                  double text_value) {
  if (!shape.valid || shape.value > 0.65) {
    return false;
  }
  if (!has_text) {
    return true;
  }
  if (shape.pattern->get_name() == "line" && shape.value < 0.15) {
    return true;
  }
  return false;
}

std::vector<Point> oriented_points(std::vector<Point> points,
                                   bool endpoint_front,
                                   bool endpoint_should_be_front) {
  if (endpoint_front != endpoint_should_be_front) {
    std::reverse(points.begin(), points.end());
  }
  return points;
}

void merge_strokes(std::vector<CleanupStroke> &strokes,
                   const EndpointMatch &match) {
  auto left =
      oriented_points(strokes[match.first].points, match.first_front, false);
  auto right =
      oriented_points(strokes[match.second].points, match.second_front, true);
  auto connection = middle(left.back(), right.front());
  left.back() = connection;
  right.front() = connection;
  left.insert(left.end(), std::next(right.begin()), right.end());

  strokes[match.first].points = std::move(left);
  strokes.erase(strokes.begin() + match.second);
}

void consider_endpoint_match(EndpointMatch &best, std::size_t first,
                             std::size_t second, bool first_front,
                             bool second_front, const Point &first_point,
                             const Point &second_point) {
  double distance = get_distance(first_point, second_point);
  if (distance < best.distance) {
    best = {
        true, first, second, first_front, second_front, distance,
    };
  }
}

EndpointMatch closest_endpoints(const std::vector<CleanupStroke> &strokes) {
  EndpointMatch best;
  for (std::size_t first = 0; first < strokes.size(); ++first) {
    for (std::size_t second = first + 1; second < strokes.size(); ++second) {
      const auto &first_points = strokes[first].points;
      const auto &second_points = strokes[second].points;
      consider_endpoint_match(best, first, second, true, true,
                              first_points.front(), second_points.front());
      consider_endpoint_match(best, first, second, true, false,
                              first_points.front(), second_points.back());
      consider_endpoint_match(best, first, second, false, true,
                              first_points.back(), second_points.front());
      consider_endpoint_match(best, first, second, false, false,
                              first_points.back(), second_points.back());
    }
  }
  return best;
}

double stroke_path_length(const std::vector<Point> &points) {
  double length = 0.0;
  for (auto it = std::next(points.begin()); it != points.end(); ++it) {
    length += get_distance(*std::prev(it), *it);
  }
  return length;
}

bool smooth_distances(const std::vector<double> &distances) {
  auto total = std::accumulate(distances.begin(), distances.end(), 0.0);
  auto average = total / distances.size();
  if (average <= std::numeric_limits<double>::epsilon()) {
    return false;
  }

  for (std::size_t i = 0; i < distances.size(); ++i) {
    if (std::abs(distances[i] - average) > average * 0.75) {
      return false;
    }
    if (i > 0) {
      auto shorter = std::min(distances[i - 1], distances[i]);
      auto longer = std::max(distances[i - 1], distances[i]);
      if (shorter <= std::numeric_limits<double>::epsilon() ||
          longer / shorter > 2.5) {
        return false;
      }
    }
  }
  return true;
}

bool smooth_headings(const std::vector<double> &headings) {
  std::vector<double> turns;
  for (auto it = std::next(headings.begin()); it != headings.end(); ++it) {
    auto turn = get_rotation(*std::prev(it), *it);
    if (std::abs(turn) > std::numbers::pi / 3.0) {
      return false;
    }
    turns.emplace_back(turn);
  }

  double signed_turn = std::accumulate(turns.begin(), turns.end(), 0.0);
  double absolute_turn = 0.0;
  for (auto turn : turns) {
    absolute_turn += std::abs(turn);
  }
  if (absolute_turn < std::numbers::pi / 18.0) {
    return false;
  }
  if (std::abs(signed_turn) < absolute_turn * 0.7) {
    return false;
  }

  for (auto it = std::next(turns.begin()); it != turns.end(); ++it) {
    if (std::abs(*it - *std::prev(it)) > std::numbers::pi / 3.0) {
      return false;
    }
  }
  return true;
}

double signed_heading_turn(const std::vector<double> &headings) {
  double turn = 0.0;
  for (auto it = std::next(headings.begin()); it != headings.end(); ++it) {
    turn += get_rotation(*std::prev(it), *it);
  }
  return turn;
}

CircleCandidate circle_through_points(const Point &first, const Point &middle,
                                      const Point &last) {
  auto a = middle[0] - first[0];
  auto b = middle[1] - first[1];
  auto c = last[0] - first[0];
  auto d = last[1] - first[1];
  auto e = a * (first[0] + middle[0]) + b * (first[1] + middle[1]);
  auto f = c * (first[0] + last[0]) + d * (first[1] + last[1]);
  auto determinant =
      2.0 * (a * (last[1] - middle[1]) - b * (last[0] - middle[0]));
  if (std::abs(determinant) <= std::numeric_limits<double>::epsilon()) {
    return {};
  }

  Point center{
      (d * e - b * f) / determinant,
      (a * f - c * e) / determinant,
  };
  auto radius = get_distance(center, first);
  if (!std::isfinite(radius) ||
      radius <= std::numeric_limits<double>::epsilon()) {
    return {};
  }
  return {true, center, radius};
}

CircleCandidate smooth_circle_candidate(const std::vector<Point> &points) {
  if (points.size() < 4) {
    return {};
  }

  std::vector<double> distances;
  std::vector<double> headings;
  for (auto it = std::next(points.begin()); it != points.end(); ++it) {
    const auto &previous = *std::prev(it);
    auto distance = get_distance(previous, *it);
    if (distance <= std::numeric_limits<double>::epsilon()) {
      return {};
    }
    distances.emplace_back(distance);
    headings.emplace_back(get_angle({
        (*it)[0] - previous[0],
        (*it)[1] - previous[1],
    }));
  }

  if (!smooth_distances(distances) || !smooth_headings(headings)) {
    return {};
  }

  auto circle = circle_through_points(points.front(), points[points.size() / 2],
                                      points.back());
  if (!circle.valid) {
    return {};
  }

  auto length = stroke_path_length(points);
  if (circle.radius > length * 10.0) {
    return {};
  }

  const auto &begin = points.front();
  const auto &end = points.back();
  circle.start_angle = get_angle({
      begin[0] - circle.center[0],
      begin[1] - circle.center[1],
  });
  circle.end_angle = get_angle({
      end[0] - circle.center[0],
      end[1] - circle.center[1],
  });
  auto sweep = get_rotation(circle.start_angle, circle.end_angle);
  if (signed_heading_turn(headings) >= 0.0) {
    if (sweep < 0.0) {
      sweep += 2.0 * std::numbers::pi;
    }
  } else if (sweep > 0.0) {
    sweep -= 2.0 * std::numbers::pi;
  }
  circle.end_angle = circle.start_angle + sweep;
  return circle;
}

double sketch_cleanup_threshold(const std::list<const Sketch *> &sketches) {
  Rectangle frame = empty_frame();
  double max_width = 0.0;
  for (auto sketch : sketches) {
    include_frame(frame, sketch->get_frame());
    max_width = std::max(max_width, sketch->get_width());
  }

  double diameter = get_distance(frame[0], frame[1]);
  double frame_threshold = std::min(diameter * 0.08, 24.0);
  return std::max(max_width * 4.0, frame_threshold);
}

std::vector<CleanupStroke>
clean_sketch_strokes(const std::list<const Sketch *> &sketches) {
  std::vector<CleanupStroke> strokes;
  for (auto sketch : sketches) {
    auto points = sketch->simplify();
    if (!points.empty()) {
      strokes.emplace_back(CleanupStroke{sketch, std::move(points)});
    }
  }

  auto threshold = sketch_cleanup_threshold(sketches);
  for (;;) {
    auto match = closest_endpoints(strokes);
    if (!match.valid || match.distance > threshold) {
      break;
    }
    merge_strokes(strokes, match);
  }
  return strokes;
}

Shape *create_cleaned_shape(const CleanupStroke &stroke) {
  if (stroke.points.size() == 1) {
    auto shape = new Dot(stroke.source->get_width(), stroke.source->get_color(),
                         stroke.source->get_style());
    shape->set_dot(stroke.points.front());
    return shape;
  }

  if (auto circle = smooth_circle_candidate(stroke.points); circle.valid) {
    auto shape =
        new Circle(stroke.source->get_width(), stroke.source->get_color(),
                   stroke.source->get_style());
    shape->set_circle(circle.center, circle.radius, circle.start_angle,
                      circle.end_angle);
    return shape;
  }

  auto sketch =
      new Sketch(stroke.source->get_width(), stroke.source->get_color(),
                 stroke.source->get_style());
  sketch->set_sketch(stroke.source->get_zoom());
  for (const auto &point : stroke.points) {
    sketch->add_point(point);
  }
  return sketch;
}

void add_cleaned_unmatched_sketches(
    const std::list<const Sketch *> &shape_sources,
    std::list<const Sketch *> &sources, std::list<Shape *> &results) {
  std::list<const Sketch *> unmatched;
  for (auto source : shape_sources) {
    if (std::find(sources.begin(), sources.end(), source) == sources.end()) {
      unmatched.emplace_back(source);
    }
  }
  if (unmatched.empty()) {
    return;
  }

  auto strokes = clean_sketch_strokes(unmatched);
  for (auto source : unmatched) {
    add_unique(sources, source);
  }
  for (const auto &stroke : strokes) {
    results.emplace_back(create_cleaned_shape(stroke));
  }
}
} // namespace

std::vector<Character> Ocr::characters_;
std::vector<Character> Ocr::shapes_;

Ocr::Ocr(Board &board)
    : run_{true}, force_apply_{false}, head_{Guess::head()}, zoom_{0},
      width_{0.0}, color_{Gdk::RGBA("#000000")}, style_{Shape::Style::SIZE},
      board_{board} {
  guesses_.emplace_back(Guess::head());
  thread_ = std::thread([this]() {
    while (run_) {
      std::chrono::steady_clock::time_point last_run{
          std::chrono::steady_clock::now()};
      std::unique_lock<std::mutex> jobs_wait_lock{jobs_lock_};
      jobs_condition_.wait_for(
          jobs_wait_lock,
          commit_delay - (std::chrono::steady_clock::now() - last_run),
          [this, &last_run]() {
            if (!run_) {
              return true;
            }
            if (!jobs_.empty() || force_apply_) {
              return true;
            }
            if (!board_.is_drawing() &&
                std::chrono::steady_clock::now() - last_run > commit_delay) {
              return true;
            }
            return false;
          });
      if (run_) {
        force_apply_ = false;
        work_lock_.lock();
        run();
        work_lock_.unlock();
      }
    }
  });
}

Ocr::~Ocr() {
  run_ = false;
  jobs_condition_.notify_all();
  thread_.join();
}

void Ocr::add(const Sketch *sketch) {
  jobs_lock_.lock();
  jobs_.emplace_back(sketch);
  jobs_lock_.unlock();
  jobs_condition_.notify_one();
}

void Ocr::finish() {
  for (;;) {
    std::lock_guard<std::mutex> jobs_guard{jobs_lock_};
    std::lock_guard<std::mutex> work_guard{work_lock_};
    if (jobs_.empty() && guesses_.empty() && shape_sources_.empty()) {
      break;
    }
    force_apply_ = true;
    jobs_condition_.notify_one();
  }
}

void Ocr::run() {
  const Sketch *sketch;
  if (jobs_.empty()) {
    sketch = nullptr;
  } else {
    sketch = jobs_.front();
    jobs_.pop_front();
  }
  jobs_lock_.unlock();
  if (sketch) {
    if (zoom_ != sketch->get_zoom() || width_ != sketch->get_width() ||
        color_ != sketch->get_color() || style_ != sketch->get_style()) {
      apply();
    }
    zoom_ = sketch->get_zoom();
    width_ = sketch->get_width();
    color_ = sketch->get_color();
    style_ = sketch->get_style();
    auto points = sketch->simplify();
    // TODO check for edge
    if (false) {
      // TODO add edge
      apply();
    } else {
      auto convexes = Convex::get_convexes(points);
      auto guesses = extend(sketch, convexes);
      if (!guesses_.empty() && check_apply(guesses)) {
        guesses.clear();
        apply(false);
        add_shape_source(sketch, convexes);
        guesses = extend(sketch, convexes);
        if (check_apply(guesses)) {
          std::swap(guesses_, guesses);
          guesses.clear();
          apply(false);
        } else {
          std::swap(guesses_, guesses);
        }
      } else {
        add_shape_source(sketch, convexes);
        std::swap(guesses_, guesses);
      }
    }
  } else if (!guesses_.empty() || !shape_sources_.empty()) {
    apply();
  }
}

bool Ocr::check_apply(const std::list<std::shared_ptr<const Guess>> &guesses) {
  for (auto guess : guesses) {
    if (!guess->is_done()) {
      return false;
    }
  }
  return true;
}

std::list<std::shared_ptr<const Guess>>
Ocr::extend(const Sketch *sketch, const std::vector<Convex> &convexes) {
  decltype(guesses_) guesses;
  if (guesses_.empty()) {
    guesses.merge(head_->extend(sketch, convexes));
  } else {
    for (auto guess : guesses_) {
      guesses.merge(guess->extend(sketch, convexes));
    }
  }
  return guesses;
}

void Ocr::add_shape_source(const Sketch *sketch,
                           const std::vector<Convex> &convexes) {
  shape_sources_.emplace_back(sketch);
  shape_convexes_.insert(shape_convexes_.end(), convexes.begin(),
                         convexes.end());
}

void Ocr::clear_shape_sources() {
  shape_sources_.clear();
  shape_convexes_.clear();
}

void Ocr::apply() { apply(true); }

void Ocr::apply(bool include_shapes) {
  double best_value = std::numeric_limits<double>::max();
  std::shared_ptr<const Guess> best_guess;
  for (const auto &guess : guesses_) {
    auto value = guess->get_diff() + pending_penalty(guess);
    if (best_value > value) {
      best_value = value;
      best_guess = guess;
    }
  }
  std::list<Shape *> results;
  std::list<const Sketch *> sources;
  std::list<const Sketch *> extras;
  std::vector<std::shared_ptr<const Guess>> characters;
  std::vector<double> rotations;
  std::string text;
  Rectangle frame = empty_frame();
  while (best_guess) {
    if (best_guess->is_complete()) {
      characters.emplace_back(best_guess);
    } else if (auto extra = best_guess->get_extra()) {
      extras.emplace_back(extra);
    }
    best_guess = best_guess->get_parent();
  }
  std::reverse(characters.begin(), characters.end());
  Rectangle previous_frame = empty_frame();
  bool has_previous_frame = false;
  for (const auto &character : characters) {
    auto character_frame = character->get_frame();
    if (has_previous_frame) {
      double gap = character_frame[0][0] - previous_frame[1][0];
      double average_width =
          (width(character_frame) + width(previous_frame)) / 2.0;
      if (gap > average_width * 0.9) {
        text += ' ';
      }
    }
    text += character->get_character();
    rotations.emplace_back(character->get_rotation());
    for (auto source : character->get_sources()) {
      add_unique(sources, source);
      include_frame(frame, source->get_frame());
    }
    previous_frame = character_frame;
    has_previous_frame = true;
  }
  auto shape = include_shapes ? best_shape(shape_sources_, shape_convexes_)
                              : ShapeCandidate{};
  if (prefer_shape(shape, !text.empty() && !sources.empty(), best_value)) {
    sources.clear();
    for (auto source : shape_sources_) {
      add_unique(sources, source);
    }
    Render *render = new Render(width_, color_, style_);
    render->set_render(shape.pattern->get_name(), shape.center, shape.cx,
                       shape.cy, shape.rotation);
    results.emplace_back(render);
  } else if (!text.empty() && !sources.empty()) {
    // TODO Check for matching text (width color style and size) in
    // neighborhood, line or paragraph, then combine them
    // TODO Check for shapes around to link their name
    // TODO Adjust top and bottom
    auto cleanup_frame = expanded(frame, 0.2);
    for (auto extra : extras) {
      if (contains(cleanup_frame, center(extra->get_frame()))) {
        add_unique(sources, extra);
      }
    }
    Text *txt = new Text(width_, color_, style_);
    double rotation = snap_rotation(average_rotation(rotations));
    txt->set_text(text, projected_frame(project_sources(sources, rotation)),
                  rotation);
    results.emplace_back(txt);
  } else {
    sources.clear();
  }
  if (include_shapes) {
    add_cleaned_unmatched_sketches(shape_sources_, sources, results);
  }
  guesses_.clear();
  if (include_shapes || !sources.empty()) {
    clear_shape_sources();
  }
  board_.apply_ocr(sources, zoom_, results);
}

void Ocr::read_characters() {
  auto characters = get_characters();
  characters_.assign(characters.begin(), characters.end());
}

void Ocr::read_shapes() {
  auto shapes = get_shapes();
  shapes_.assign(shapes.begin(), shapes.end());
}
