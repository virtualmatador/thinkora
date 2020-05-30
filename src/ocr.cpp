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
    Job::shape_patterns_ = read_patterns("pattern/shape/");
    for (auto& dir: std::filesystem::directory_iterator("pattern/character/"))
    {
        if (dir.is_directory())
        {
            Job::char_patterns_[dir.path().filename()] =
                read_patterns(dir.path());
        }
    }
    thread_ = std::thread([this]()
    {
        while (run_)
        {
            if (get_sketch())
            {
                std::this_thread::sleep_for(std::chrono::operator""ms(delay_ms_ / 4));
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
            patterns.emplace_back(json_pattern);
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
    board_->list_sketches(job);
    std::chrono::steady_clock::time_point recent =
        std::chrono::steady_clock::time_point::min();
    for (const auto& sketch: job->sketches_)
    {
        if (recent < sketch.get_birth())
        {
            recent = sketch.get_birth();
        }
    }
    if (recent == std::chrono::steady_clock::time_point::min() ||
        (recent != std::chrono::steady_clock::time_point::max() &&
        time - recent > std::chrono::operator""ms(delay_ms_)))
    {
        job->process();
        std::lock_guard<std::mutex> lock{jobs_lock_};
        jobs_.pop_front();
        return false;
    }
    //board_->replace_sketches(job, sketches, shapes)

    // TODO Combine with text in left if size and position and style match
    // Choose between lowercase and uppercase
    // Combine . "
    // modify using sticky points
    // create sticky points
    return true;
}

/*
    if (!character)
    {
        for (const auto& element: elements)
        {
            // TODO Polyline* pl = new Polyline(element);
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
    else
    {
        Text* text = new Text(job->line_width_, job->color_, job->style_);
        auto region = Cairo::Region::create();
        auto dc = board_->get_window()->begin_draw_frame(region);
        text->set_text(dc->get_cairo_context(), frame[0],
            frame[1][1] - frame[0][1], *character);
        board_->get_window()->end_draw_frame(dc);
        shapes.emplace_back(text);
    }

*/