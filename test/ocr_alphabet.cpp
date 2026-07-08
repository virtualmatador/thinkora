#include <gtkmm.h>

#include <array>
#include <chrono>
#include <cmath>
#include <cstdlib>
#include <initializer_list>
#include <iostream>
#include <mutex>
#include <numbers>
#include <set>
#include <string>
#include <thread>
#include <vector>

#define private public
#include "text.h"
#include "window.h"
#undef private

namespace {
using Point2 = std::array<double, 2>;
using Stroke = std::vector<Point2>;

constexpr double left = 0.1;
constexpr double right = 0.9;
constexpr double top = 0.05;
constexpr double bottom = 0.95;
constexpr double middle = 0.5;
constexpr double center = 0.5;

Stroke line(double x1, double y1, double x2, double y2) {
  return {{x1, y1}, {x2, y2}};
}

Stroke poly(std::initializer_list<Point2> points) { return points; }

Stroke arc(double cx, double cy, double rx, double ry, double start, double end,
           int steps) {
  Stroke points;
  points.reserve(steps);
  for (int i = 0; i < steps; ++i) {
    double t =
        (start + (end - start) * i / (steps - 1)) * std::numbers::pi / 180.0;
    points.push_back({cx + rx * std::cos(t), cy + ry * std::sin(t)});
  }
  return points;
}

std::vector<Stroke> glyph(char ch) {
  switch (ch) {
  case 'A':
    return {
        line(0.25, bottom, 0.4, top),
        line(0.35, 0.55, 0.65, 0.55),
        line(0.6, top, 0.75, bottom),
    };
  case 'B':
    return {
        line(left, top, left, bottom),
        arc(left, 0.275, 0.58, 0.225, 270.0, 450.0, 7),
        arc(left, 0.725, 0.62, 0.225, 270.0, 450.0, 7),
    };
  case 'C':
    return {arc(0.53, middle, 0.43, 0.45, 310.0, 50.0, 9)};
  case 'D':
    return {
        line(left, top, left, bottom),
        arc(left, middle, 0.72, 0.45, 270.0, 450.0, 9),
    };
  case 'E':
    return {
        line(left, top, left, bottom),
        line(left, top, right, top),
        line(left, middle, 0.76, middle),
        line(left, bottom, right, bottom),
    };
  case 'F':
    return {
        line(left, top, left, bottom),
        line(left, top, right, top),
        line(left, middle, 0.76, middle),
    };
  case 'G':
    return {
        arc(0.53, middle, 0.43, 0.45, 310.0, 20.0, 8),
        line(0.88, 0.56, 0.56, 0.56),
    };
  case 'H':
    return {
        line(left, top, left, bottom),
        line(right, top, right, bottom),
        line(left, middle, right, middle),
    };
  case 'I':
    return {
        line(0.22, top, 0.78, top),
        line(center, top, center, bottom),
        line(0.22, bottom, 0.78, bottom),
    };
  case 'J':
    return {
        line(0.24, top, right, top),
        poly({{right, top},
              {right, 0.72},
              {0.78, bottom},
              {0.48, bottom},
              {0.32, 0.78}}),
    };
  case 'K':
    return {
        line(left, top, left, bottom),
        line(right, top, left, middle),
        line(left, middle, right, bottom),
    };
  case 'L':
    return {
        line(left, top, left, bottom),
        line(left, bottom, right, bottom),
    };
  case 'M':
    return {
        line(left, bottom, left, top),
        line(left, top, center, 0.58),
        line(center, 0.58, right, top),
        line(right, top, right, bottom),
    };
  case 'N':
    return {
        line(left, bottom, left, top),
        line(left, top, right, bottom),
        line(right, bottom, right, top),
    };
  case 'O':
    return {arc(center, middle, 0.42, 0.45, 270.0, 630.0, 13)};
  case 'P':
    return {
        line(left, top, left, bottom),
        arc(left, 0.29, 0.6, 0.24, 270.0, 450.0, 8),
    };
  case 'Q':
    return {
        arc(center, middle, 0.42, 0.45, 270.0, 630.0, 13),
        line(0.62, 0.68, right, bottom),
    };
  case 'R':
    return {
        line(left, top, left, bottom),
        arc(left, 0.29, 0.6, 0.24, 270.0, 450.0, 8),
        line(left, middle, right, bottom),
    };
  case 'S':
    return {
        poly({{0.82, 0.16},
              {0.62, top},
              {0.28, 0.12},
              {0.18, 0.34},
              {0.38, middle},
              {0.72, 0.58},
              {0.82, 0.8},
              {0.62, bottom},
              {0.24, 0.86}}),
    };
  case 'T':
    return {
        line(left, top, right, top),
        line(center, top, center, bottom),
    };
  case 'U':
    return {
        poly({{left, top},
              {left, 0.64},
              {0.22, bottom},
              {center, bottom},
              {0.78, bottom},
              {right, 0.64},
              {right, top}}),
    };
  case 'V':
    return {
        line(left, top, center, bottom),
        line(center, bottom, right, top),
    };
  case 'W':
    return {
        line(left, top, 0.3, bottom),
        line(0.3, bottom, center, 0.58),
        line(center, 0.58, 0.7, bottom),
        line(0.7, bottom, right, top),
    };
  case 'X':
    return {
        line(left, top, right, bottom),
        line(right, top, left, bottom),
    };
  case 'Y':
    return {
        line(left, top, center, middle),
        line(right, top, center, middle),
        line(center, middle, center, bottom),
    };
  case 'Z':
    return {
        line(left, top, right, top),
        line(right, top, left, bottom),
        line(left, bottom, right, bottom),
    };
  case '0':
    return {
        line(left, top, right, top),
        line(right, top, right, bottom),
        line(left, bottom, right, bottom),
        line(left, top, left, bottom),
    };
  case '1':
    return {
        line(center, top, center, bottom),
    };
  case '2':
    return {
        poly({{0.18, 0.18},
              {0.38, top},
              {0.72, 0.12},
              {0.86, 0.34},
              {0.66, middle},
              {left, bottom},
              {right, bottom}}),
    };
  case '3':
    return {
        poly({{0.22, 0.12},
              {0.78, 0.12},
              {0.58, middle},
              {0.82, 0.84},
              {0.22, 0.88}}),
    };
  case '4':
    return {
        line(0.72, top, 0.72, bottom),
        line(0.72, top, left, middle),
        line(left, middle, right, middle),
    };
  case '5':
    return {
        poly({{right, top},
              {0.22, top},
              {0.18, middle},
              {0.72, middle},
              {0.84, 0.76},
              {0.62, bottom},
              {0.24, 0.86}}),
    };
  case '6':
    return {
        poly({{0.78, 0.12},
              {0.34, 0.18},
              {0.16, middle},
              {0.3, 0.86},
              {0.64, bottom},
              {0.86, 0.74},
              {0.68, 0.52},
              {0.28, 0.52}}),
    };
  case '7':
    return {
        line(left, top, right, top),
        line(right, top, 0.36, bottom),
    };
  case '8':
    return {
        arc(center, 0.3, 0.28, 0.25, 270.0, 630.0, 9),
        arc(center, 0.7, 0.30, 0.25, 270.0, 630.0, 9),
    };
  case '9':
    return {
        poly({{0.72, 0.48},
              {0.32, 0.48},
              {0.14, 0.26},
              {0.36, top},
              {0.7, 0.14},
              {0.84, middle},
              {0.66, 0.82},
              {0.22, 0.88}}),
    };
  default:
    return {};
  }
}

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

void keep_window_alive(std::chrono::milliseconds duration) {
  auto deadline = std::chrono::steady_clock::now() + duration;
  while (std::chrono::steady_clock::now() < deadline) {
    drain_events();
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
  }
  drain_events();
}

Point2 to_screen(Board &board, Point2 point) {
  auto allocation = board.get_allocation();
  return {
      point[0] - board.center_[0] + allocation.get_width() / 2.0,
      point[1] - board.center_[1] + allocation.get_height() / 2.0,
  };
}

void mouse_press(Board &board, unsigned int button, Point2 point) {
  board.on_button_press(button, point[0], point[1]);
}

void mouse_motion(Board &board, Point2 point) {
  board.on_motion(point[0], point[1]);
}

void mouse_release(Board &board, unsigned int button, Point2 point) {
  board.on_button_release(button, point[0], point[1]);
}

void draw_stroke(Board &board, const Stroke &stroke, double x_offset) {
  constexpr double scale = 160.0;
  const Point2 origin{x_offset - 80.0, -80.0};
  auto first = to_screen(board, {origin[0] + stroke.front()[0] * scale,
                                 origin[1] + stroke.front()[1] * scale});
  mouse_press(board, 1, first);
  for (auto point : stroke) {
    auto screen = to_screen(
        board, {origin[0] + point[0] * scale, origin[1] + point[1] * scale});
    mouse_motion(board, screen);
  }
  auto last = to_screen(board, {origin[0] + stroke.back()[0] * scale,
                                origin[1] + stroke.back()[1] * scale});
  mouse_release(board, 1, last);
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

        bool has_guesses =
            !board.ocr_.guesses_.empty() || !board.ocr_.shape_sources_.empty();
        jobs_lock.unlock();
        if (has_guesses) {
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

bool check_ocr_result(Board &board, char ch, double center_x,
                      std::size_t expected_text_count) {
  auto shapes = get_shapes(board);
  std::size_t text_count = 0;
  std::size_t sketch_count = 0;
  bool found_current_text = false;
  for (const auto *shape : shapes) {
    if (shape->get_type() == Shape::Type::SKETCH) {
      ++sketch_count;
    } else if (shape->get_type() == Shape::Type::TEXT) {
      ++text_count;
      const auto *text = dynamic_cast<const Text *>(shape);
      const auto &frame = text->get_frame();
      double frame_center = (frame[0][0] + frame[1][0]) / 2.0;
      if (text->text_ == std::string(1, ch) &&
          std::abs(frame_center - center_x) < 100.0) {
        found_current_text = true;
      }
    }
  }
  if (sketch_count != 0) {
    std::cerr << "Expected OCR to replace sketches, found " << sketch_count
              << " sketch shapes after " << ch << ".\n";
    for (const auto *shape : shapes) {
      if (shape->get_type() == Shape::Type::TEXT) {
        const auto *text = dynamic_cast<const Text *>(shape);
        const auto &frame = text->get_frame();
        std::cerr << "  text: '" << text->text_ << "' frame=(" << frame[0][0]
                  << ", " << frame[0][1] << ") - (" << frame[1][0] << ", "
                  << frame[1][1] << ")\n";
      }
    }
    return false;
  }
  if (text_count != expected_text_count) {
    std::cerr << "Expected " << expected_text_count << " text shapes after "
              << ch << ", found " << text_count << ".\n";
    return false;
  }
  if (!found_current_text) {
    std::cerr << "Did not find text " << ch << " near x=" << center_x << ".\n";
    for (const auto *shape : shapes) {
      if (shape->get_type() == Shape::Type::TEXT) {
        const auto *text = dynamic_cast<const Text *>(shape);
        const auto &frame = text->get_frame();
        std::cerr << "  text: '" << text->text_ << "' frame=(" << frame[0][0]
                  << ", " << frame[0][1] << ") - (" << frame[1][0] << ", "
                  << frame[1][1] << ")\n";
      }
    }
    return false;
  }
  return true;
}

bool has_display() {
  return std::getenv("DISPLAY") || std::getenv("WAYLAND_DISPLAY");
}
} // namespace

int main(int argc, char **argv) {
  if (!has_display()) {
    std::cerr << "Skipping OCR alphabet test: no GUI display.\n";
    return 77;
  }

  auto app = Gtk::Application::create("com.shaidin.thinkora.ocr-alphabet-test");
  app->register_application();

  Window window;
  app->add_window(window);
  window.unfullscreen();
  window.set_default_size(1600, 900);
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

  const std::string samples = "ABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";
  double center_x = 0.0;
  for (std::size_t index = 0; index < samples.size(); ++index) {
    char ch = samples[index];
    std::cerr << "Drawing " << ch << '\n';

    for (const auto &stroke : glyph(ch)) {
      draw_stroke(board, stroke, center_x);
    }

    std::cerr << "Waiting for OCR " << ch << '\n';
    if (!flush_ocr(board)) {
      std::cerr << "OCR did not finish after drawing " << ch << ".\n";
      std::cerr.flush();
      std::_Exit(1);
    }
    if (!check_ocr_result(board, ch, center_x, index + 1)) {
      std::cerr.flush();
      std::_Exit(1);
    }

    center_x += 250.0;
    board.center_ = {center_x, 0.0};
    board.redraw(false);
    drain_events();

    std::this_thread::sleep_for(std::chrono::milliseconds(100));
  }

  keep_window_alive(std::chrono::milliseconds(3000));

  board.modified_ = false;
  std::cerr.flush();
}
