#include <gtkmm.h>

#include <array>
#include <chrono>
#include <cstdlib>
#include <iostream>
#include <mutex>
#include <set>
#include <thread>
#include <vector>

#define private public
#include "render.h"
#include "window.h"
#undef private

namespace {
using Point2 = std::array<double, 2>;
using Stroke = std::vector<Point2>;

void drain_events() {
  auto context = Glib::MainContext::get_default();
  while (context->pending()) {
    context->iteration(false);
  }
}

template <typename Predicate>
bool wait_for(Predicate predicate, std::chrono::milliseconds timeout) {
  auto deadline = std::chrono::steady_clock::now() + timeout;
  while (std::chrono::steady_clock::now() < deadline) {
    drain_events();
    if (predicate()) {
      return true;
    }
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
  }
  drain_events();
  return predicate();
}

Point2 to_screen(Board &board, Point2 point) {
  auto allocation = board.get_allocation();
  return {
      point[0] - board.center_[0] + allocation.get_width() / 2.0,
      point[1] - board.center_[1] + allocation.get_height() / 2.0,
  };
}

void draw_stroke_at(Board &board, const Stroke &stroke, Point2 origin) {
  constexpr double scale = 160.0;
  auto first = to_screen(board, {origin[0] + stroke.front()[0] * scale,
                                 origin[1] + stroke.front()[1] * scale});
  board.on_button_press(1, first[0], first[1]);
  for (auto point : stroke) {
    auto screen = to_screen(
        board, {origin[0] + point[0] * scale, origin[1] + point[1] * scale});
    board.on_motion(screen[0], screen[1]);
  }
  auto last = to_screen(board, {origin[0] + stroke.back()[0] * scale,
                                origin[1] + stroke.back()[1] * scale});
  board.on_button_release(1, last[0], last[1]);
}

void draw_stroke(Board &board, const Stroke &stroke) {
  draw_stroke_at(board, stroke, {-80.0, -80.0});
}

void add_unmatched_sketch(Board &board, const Stroke &stroke, Point2 origin) {
  constexpr double scale = 160.0;
  auto sketch = new Sketch{1.0, Gdk::RGBA("#FFFFFF"), Shape::Style::SOLID};
  sketch->set_sketch(board.zoom_);
  for (auto point : stroke) {
    sketch->add_point({origin[0] + point[0] * scale,
                       origin[1] + point[1] * scale});
  }
  std::lock_guard<std::mutex> shapes_lock{board.shapes_lock_};
  board.add_reference(board.zoom_, sketch);
  board.ocr_.shape_sources_.emplace_back(sketch);
}

bool flush_ocr(Board &board) {
  return wait_for(
      [&] {
        std::unique_lock<std::mutex> work_lock{board.ocr_.work_lock_,
                                               std::try_to_lock};
        if (!work_lock) {
          return false;
        }

        std::unique_lock<std::mutex> jobs_lock{board.ocr_.jobs_lock_,
                                               std::try_to_lock};
        if (!jobs_lock) {
          return false;
        }

        if (!board.ocr_.jobs_.empty()) {
          return false;
        }

        bool has_work =
            !board.ocr_.guesses_.empty() || !board.ocr_.shape_sources_.empty();
        jobs_lock.unlock();
        if (has_work) {
          board.ocr_.apply();
        }
        return true;
      },
      std::chrono::seconds(5));
}

std::set<const Shape *> get_shapes(Board &board) {
  std::set<const Shape *> shapes;
  for (const auto &[zoom, layer] : board.shapes_) {
    for (const auto &[position, region] : layer) {
      shapes.insert(region.begin(), region.end());
    }
  }
  return shapes;
}

bool has_display() {
  return std::getenv("DISPLAY") || std::getenv("WAYLAND_DISPLAY");
}
} // namespace

int main(int argc, char **argv) {
  if (!has_display()) {
    std::cerr << "Skipping OCR shapes test: no GUI display.\n";
    return 77;
  }

  auto app = Gtk::Application::create("com.shaidin.thinkora.ocr-shapes-test");
  app->register_application();

  Window window;
  app->add_window(window);
  window.unfullscreen();
  window.set_default_size(800, 600);
  window.present();

  Board &board = window.board_;
  if (!wait_for(
          [&] {
            auto allocation = board.get_allocation();
            return allocation.get_width() > 100 &&
                   allocation.get_height() > 100;
          },
          std::chrono::seconds(5))) {
    std::cerr << "Board was not allocated.\n";
    std::cerr.flush();
    std::_Exit(1);
  }

  draw_stroke(board, {{0.1, 0.5}, {0.9, 0.5}});
  if (!flush_ocr(board)) {
    std::cerr << "OCR did not finish after drawing line.\n";
    std::cerr.flush();
    std::_Exit(1);
  }

  std::size_t render_count = 0;
  std::size_t sketch_count = 0;
  bool found_line = false;
  for (const auto *shape : get_shapes(board)) {
    if (shape->get_type() == Shape::Type::SKETCH) {
      ++sketch_count;
    } else if (shape->get_type() == Shape::Type::RENDER) {
      ++render_count;
      const auto *render = dynamic_cast<const Render *>(shape);
      found_line = found_line || render->get_name() == "line";
    }
  }

  if (sketch_count != 0 || render_count != 1 || !found_line) {
    std::cerr << "Expected one rendered line and no sketches. sketches="
              << sketch_count << " renders=" << render_count << '\n';
    std::cerr.flush();
    std::_Exit(1);
  }

  const Point2 cleanup_origin{160.0, -80.0};
  {
    std::lock_guard<std::mutex> work_lock{board.ocr_.work_lock_};
    add_unmatched_sketch(board,
                         {{0.02, 0.04}, {0.24, 0.2}, {0.08, 0.38}},
                         cleanup_origin);
    add_unmatched_sketch(board,
                         {{0.1, 0.4}, {0.33, 0.16}, {0.42, 0.44}},
                         cleanup_origin);
    board.ocr_.apply();
  }
  drain_events();

  render_count = 0;
  sketch_count = 0;
  std::size_t text_count = 0;
  std::size_t cleaned_point_count = 0;
  found_line = false;
  for (const auto *shape : get_shapes(board)) {
    if (shape->get_type() == Shape::Type::SKETCH) {
      ++sketch_count;
      const auto *sketch = dynamic_cast<const Sketch *>(shape);
      cleaned_point_count = sketch->get_points().size();
    } else if (shape->get_type() == Shape::Type::RENDER) {
      ++render_count;
      const auto *render = dynamic_cast<const Render *>(shape);
      found_line = found_line || render->get_name() == "line";
    } else if (shape->get_type() == Shape::Type::TEXT) {
      ++text_count;
    }
  }

  if (sketch_count != 1 || cleaned_point_count < 4 || render_count != 1 ||
      !found_line || text_count != 0) {
    std::cerr << "Expected unmatched strokes to be cleaned into one sketch. "
              << "sketches=" << sketch_count
              << " points=" << cleaned_point_count
              << " renders=" << render_count << " text=" << text_count
              << '\n';
    std::cerr.flush();
    std::_Exit(1);
  }

  const Point2 circle_origin{400.0, -80.0};
  {
    std::lock_guard<std::mutex> work_lock{board.ocr_.work_lock_};
    add_unmatched_sketch(board,
                         {{0.1, 0.8},
                          {0.2, 0.55},
                          {0.4, 0.35},
                          {0.65, 0.25},
                          {0.9, 0.3}},
                         circle_origin);
    board.ocr_.apply();
  }
  drain_events();

  render_count = 0;
  sketch_count = 0;
  std::size_t circle_count = 0;
  text_count = 0;
  found_line = false;
  for (const auto *shape : get_shapes(board)) {
    if (shape->get_type() == Shape::Type::SKETCH) {
      ++sketch_count;
    } else if (shape->get_type() == Shape::Type::CIRCLE) {
      ++circle_count;
    } else if (shape->get_type() == Shape::Type::RENDER) {
      ++render_count;
      const auto *render = dynamic_cast<const Render *>(shape);
      found_line = found_line || render->get_name() == "line";
    } else if (shape->get_type() == Shape::Type::TEXT) {
      ++text_count;
    }
  }

  if (sketch_count != 1 || circle_count != 1 || render_count != 1 ||
      !found_line || text_count != 0) {
    std::cerr << "Expected smooth unmatched stroke to become one circle arc. "
              << "sketches=" << sketch_count << " circles=" << circle_count
              << " renders=" << render_count << " text=" << text_count
              << '\n';
    std::cerr.flush();
    std::_Exit(1);
  }

  const Point2 dot_origin{640.0, -80.0};
  {
    std::lock_guard<std::mutex> work_lock{board.ocr_.work_lock_};
    add_unmatched_sketch(board, {{0.5, 0.5}}, dot_origin);
    board.ocr_.apply();
  }
  drain_events();

  render_count = 0;
  sketch_count = 0;
  circle_count = 0;
  std::size_t dot_count = 0;
  text_count = 0;
  found_line = false;
  for (const auto *shape : get_shapes(board)) {
    if (shape->get_type() == Shape::Type::SKETCH) {
      ++sketch_count;
    } else if (shape->get_type() == Shape::Type::CIRCLE) {
      ++circle_count;
    } else if (shape->get_type() == Shape::Type::DOT) {
      ++dot_count;
    } else if (shape->get_type() == Shape::Type::RENDER) {
      ++render_count;
      const auto *render = dynamic_cast<const Render *>(shape);
      found_line = found_line || render->get_name() == "line";
    } else if (shape->get_type() == Shape::Type::TEXT) {
      ++text_count;
    }
  }

  if (sketch_count != 1 || circle_count != 1 || dot_count != 1 ||
      render_count != 1 || !found_line || text_count != 0) {
    std::cerr << "Expected unmatched point sketch to become one dot. "
              << "sketches=" << sketch_count << " circles=" << circle_count
              << " dots=" << dot_count << " renders=" << render_count
              << " text=" << text_count << '\n';
    std::cerr.flush();
    std::_Exit(1);
  }

  board.modified_ = false;
  std::cerr.flush();
}
