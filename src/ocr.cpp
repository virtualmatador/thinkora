#include <cstddef>
#include <filesystem>
#include <fstream>
#include <list>
#include <memory>
#include <vector>

#include "json.h"

#include "board.h"
#include "line.h"
#include "shape.h"
#include "toolbox.h"

#include "ocr.h"

Ocr::Ocr(Board& board)
    : run_{ true }
    , zoom_{ 0 }
    , width_{ 0.0 }
    , color_{ Gdk::RGBA("#000000FF") }
    , style_{ Shape::Style::SIZE }
    , patterns_{ read_json<Pattern>("../language/patterns") }
    , characters_{ read_json<Character>("../language/characters") }
    , board_{ board }
{
    thread_ = std::thread([this]()
    {
        while (run_)
        {
            std::unique_lock<std::mutex> jobs_wait_lock{ jobs_lock_ };
            jobs_condition_.wait_for(jobs_wait_lock, std::chrono::seconds(2));
            if (run_)
            {
                working_lock_.lock();
                jobs_wait_lock.unlock();
                run();
                working_lock_.unlock();
            }
        }
    });
}

Ocr::~Ocr()
{
    run_ = false;
    jobs_condition_.notify_all();
    thread_.join();
}

void Ocr::add(const Sketch* sketch)
{
    jobs_lock_.lock();
    jobs_.emplace_back(sketch);
    jobs_lock_.unlock();
    jobs_condition_.notify_one();
}

void Ocr::cancel()
{
    jobs_lock_.lock();
    working_lock_.lock();
    jobs_.clear();
    working_lock_.unlock();
    jobs_lock_.unlock();
}

void Ocr::run()
{
    jobs_lock_.lock();
    const Sketch* sketch;
    if (jobs_.empty())
    {
        sketch = nullptr;
    }
    else
    {
        sketch = jobs_.front();
        jobs_.pop_front();
    }
    jobs_lock_.unlock();
    if (sketch)
    {
        if (zoom_ != sketch->get_zoom() ||
            width_ != sketch->get_width() ||
            color_ != sketch->get_color() ||
            style_ != sketch->get_style())
        {
            apply();
        }
        zoom_ = sketch->get_zoom();
        sources_.emplace_back(sketch);
        // Check for edge
        decltype(guesses_) guesses;
        auto points = sketch->simplify();
        bool match = false;
        for (const auto& pattern : patterns_)
        {
            auto diff = pattern.match(points, sketch->get_frame());
            if (diff < 0.4)
            {
                match = true;
                for (auto& guess : guesses_)
                {
                    guesses.emplace_back(guess->extend(
                        pattern.get_name(), sketch, diff));
                }
            }
        }
        if (match)
        {
            std::swap(guesses_, guesses);
        }
        else
        {
            auto start = points.front();
            for (auto it = std::next(points.begin()); it != points.end(); ++it)
            {
                auto ln = new Line(sketch);
                ln->set_line({start[0], start[1], (*it)[0], (*it)[1]});
                results_.emplace_back(ln);
                start = *it;
            }
            apply();
        }
    }
    else
    {
        apply();
    }
    
    /*
    bool incomplete = false;
    double best_score = 0;
    const Guess* best_guess = nullptr;
    for (const auto& guess : guesses_)
    {
        if (guess.is_complete())
        {
            if (best_score < guess.get_score())
            {
                best_score = guess.get_score();
                best_guess = &guess;
            }
        }
        else
        {
            incomplete = true;
            break;
        }

    }
    if (!incomplete)
    {
        apply(*best_guess);
        guesses_.clear();
        guesses_.emplace_back(Guess::start_node());
    }
    */
}

void Ocr::apply()
{
    // TODO apply to results
    guesses_.clear();
    guesses_.emplace_back(Guess::start_node());
    board_.apply_ocr(sources_, zoom_, results_);
    sources_.clear();
    results_.clear();
}

template<class T>
std::vector<T> Ocr::read_json(const std::filesystem::path& path)
{
    std::vector<T> results;
    for (auto& json_file: std::filesystem::directory_iterator(path))
    {
        std::fstream json_reader(json_file.path());
        jsonio::json json_data;
        json_reader >> json_data;
        if (json_data.completed())
        {
            results.emplace_back(json_file.path().stem(), json_data);
        }
    }
    return results;
}
