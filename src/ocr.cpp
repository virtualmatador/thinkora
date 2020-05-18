#include <cstddef>
#include <filesystem>
#include <fstream>
#include <memory>
#include <vector>

#include "json.h"

#include "board.h"
#include "convex.h"
#include "circle.h"
#include "line.h"
#include "point.h"
#include "text.h"
#include "toolbox.h"

#include "ocr.h"

Ocr::Ocr(Board* board)
    : run_{true}
    , board_{board}
{
    shape_patterns_ = read_patterns("pattern/shape/");
    for (auto& dir: std::filesystem::directory_iterator("pattern/character/"))
    {
        if (dir.is_directory())
        {
            char_patterns_[dir.path().filename()] = read_patterns(dir.path());
        }
    }
    thread_ = std::thread([this]()
    {
        while (run_)
        {
            if (get_sketch())
            {
                std::this_thread::sleep_for(std::chrono::operator""ms(100));
            }
        }
    });
}

std::vector<Pattern> Ocr::read_patterns(std::filesystem::path path)
{
    std::vector<Pattern> patterns;
    for (auto& json_file: std::filesystem::directory_iterator(path))
    {
        std::fstream json_reader(json_file.path());
        jsonio::json json_pattern;
        json_reader >> json_pattern;
        if (json_pattern.completed())
        {
            patterns.emplace_back(json_file.path().filename(), json_pattern);
        }
    }
    return patterns;
}

Ocr::~Ocr()
{
    run_ = false;
    thread_.join();
}

void Ocr::add(const Job& job)
{
    std::lock_guard<std::mutex> lock{jobs_lock_};
    jobs_.emplace_back(job);
}

bool Ocr::get_sketch()
{
    Job* job;
    {
        std::lock_guard<std::mutex> lock{jobs_lock_};
        if (jobs_.size() > 0)
        {
            job = &jobs_.front();
        }
        else
        {
            return true;
        }
    }
    auto time = std::chrono::steady_clock::now();
    std::array<std::array<int, 2>, 2> frame = job->frame_;
    auto sketches = board_->list_sketches(job, frame);
    std::chrono::steady_clock::time_point recent =
        std::chrono::steady_clock::time_point::min();
    for (const auto& sketch: sketches)
    {
        if (recent < sketch.get_birth())
        {
            recent = sketch.get_birth();
        }
    }
    if (recent != std::chrono::steady_clock::time_point::min() &&
        (recent == std::chrono::steady_clock::time_point::max() ||
        time - recent < std::chrono::operator""s(1)))
    {
        return true;
    }
    else if (sketches.size() == 0 || process(job, sketches, frame))
    {
        std::lock_guard<std::mutex> lock{jobs_lock_};
        jobs_.pop_front();
    }
    return false;
}

bool Ocr::process(const Job* job, std::vector<Sketch>& sketches,
    const std::array<std::array<int, 2>, 2>& frame)
{
    std::vector<std::vector<Convex>> elements;
    for (auto& sketch: sketches)
    {
        simplify(sketch, frame, elements);
    }
    std::vector<Shape*> shapes = match(job, frame, elements);
    // modify using sticky points
    // create sticky points
    return board_->replace_sketches(job, sketches, shapes);
}

void Ocr::simplify(Sketch& sketch, const std::array<std::array<int, 2>, 2>&
        frame, std::vector<std::vector<Convex>>& elements)
{
    auto& points = sketch.get_points();
    double tolerance = get_diameter(sketch.get_frame()) / 24.0;
    std::vector<std::tuple<double, double, std::size_t>> redondents;
    do
    {
        redondents.clear();
        for (std::size_t i = 2; i < points.size(); ++i)
        {
            double len1, len2;
            auto angle = get_angle(points[i - 2], points[i - 1], points[i],
                &len1, &len2);
            if (std::pow(std::cos(angle) + 1.0, 0.6) * std::min(len1, len2) < tolerance)
            {
                redondents.emplace_back(angle, len1 * len2, i - 1);
                ++i;
            }
        }
        std::sort(redondents.begin(), redondents.end(), [](auto& a, auto&b)
        {
            if (std::get<0>(a) == std::get<0>(b))
            {
                return std::get<1>(a) < std::get<1>(b);
            }
            return std::get<0>(a) > std::get<0>(b);
        });
        redondents.resize((redondents.size() + 1) / 2);
        std::sort(redondents.begin(), redondents.end(), [](auto& a, auto&b)
        {
            return std::get<2>(a) > std::get<2>(b);
        });
        for (const auto& redondent: redondents)
        {
            points.erase(points.begin() + std::get<2>(redondent));
        }
    } while (redondents.size() > 0);
    elements.emplace_back(Convex::extract(points, frame));
}

std::vector<Shape*> Ocr::match(const Job* job, const std::array<std::array
    <int, 2>, 2>& frame, std::vector<std::vector<Convex>>& elements)
{
    std::vector<Shape*> shapes;
    double min_difference = 0.5;
    const std::string* character = nullptr;
    for (const auto& [language, patterns]: char_patterns_)
    {
        for (const auto& pattern: patterns)
        {
            double difference = pattern.match(elements);
            if (min_difference > difference)
            {
                min_difference = difference;
                character = &pattern.get_character();
            }
        }
    }
    for (const auto& pattern: shape_patterns_)
    {
        double difference = pattern.match(elements);
        if (min_difference > difference)
        {
            min_difference = difference;
            character = &pattern.get_character();
        }
    }
    if (!character)
    {
        for (const auto& element: elements)
        {
            //Polyline* pl = new Polyline(element);
            //shapes.emplace_back(pl);
        }
    }
    else if (*character == "circle")
    {
        Circle* circle = new Circle(job->line_width_, job->color_, job->style_);
        circle->set_circle(get_center(frame),
            std::pow(std::pow(get_diameter(frame), 2.0) / 2.0, 0.5) / 2.0);
        shapes.emplace_back(circle);
    }
    else if (*character == "A")
    {
        Text* text = new Text(job->line_width_, job->color_, job->style_);
        auto region = Cairo::Region::create();
        auto dc = board_->get_window()->begin_draw_frame(region);
        text->set_text(dc->get_cairo_context(), frame[0],
            frame[1][1] - frame[0][1], "A");
        board_->get_window()->end_draw_frame(dc);
        shapes.emplace_back(text);
    }
    return shapes;
}
