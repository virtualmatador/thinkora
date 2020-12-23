#include <array>
#include <cstddef>
#include <chrono>
#include <filesystem>
#include <fstream>
#include <list>
#include <memory>
#include <numbers>
#include <vector>

#include "json.h"

#include "board.h"
#include "circle.h"
#include "dot.h"
#include "shape.h"
#include "text.h"
#include "toolbox.h"
#include "wire.h"

#include "ocr.h"

std::vector<Character> Ocr::characters_;

Ocr::Ocr(Board& board)
    : run_{ true }
    , zoom_{ 0 }
    , width_{ 0.0 }
    , color_{ Gdk::RGBA("#000000") }
    , style_{ Shape::Style::SIZE }
    , board_{ board }
{
    guesses_.emplace_back(Guess::head());
    thread_ = std::thread([this]()
    {
        while (run_)
        {
            std::chrono::steady_clock::time_point last_run{
                std::chrono::steady_clock::now() };
            std::unique_lock<std::mutex> jobs_wait_lock{ jobs_lock_ };
            jobs_condition_.wait_for(jobs_wait_lock, std::chrono::seconds(2),
                [this, &last_run]()
            {
                return !run_ || !jobs_.empty() ||
                    std::chrono::steady_clock::now() - last_run > std::chrono::seconds(2);
            });
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
        width_ = sketch->get_width();
        color_ = sketch->get_color();
        style_ = sketch->get_style();


        std::list<Shape*> results;
        std::list<const Sketch*> sources;
        sources.emplace_back(sketch);
        auto points = sketch->simplify();
        // TODO check for edge
        if (false)
        {
            // TODO add edge
            apply();
        }
        else
        {
            auto convexes = Convex::get_convexes(points);
            for (const auto convex : convexes)
            {
                auto guesses = extend(sketch, convex);
                if (check_apply(guesses))
                {
                    guesses.clear();
                    apply();
                    guesses = extend(sketch, convex);
                    if (check_apply(guesses))
                    {
                        std::swap(guesses_, guesses);
                        guesses.clear();
                        apply();
                    }
                    else
                    {
                        std::swap(guesses_, guesses);
                    }
                }
                else
                {
                    std::swap(guesses_, guesses);
                }
            }
        }
    }
    else
    {
        apply();
    }
}

bool Ocr::check_apply(const std::list<std::shared_ptr<Guess>>& guesses)
{
    for (auto guess : guesses)
    {
        if (!guess->is_done())
        {
            return false;
        }
    }
    return true;
}

std::list<std::shared_ptr<Guess>> Ocr::extend(const Sketch* sketch,
    const Convex& convex)
{
    decltype(guesses_) guesses;
    for (auto guess : guesses_)
    {
        guesses.merge(guess->extend(sketch, convex));
    }
    return guesses;
}

void Ocr::apply()
{
    double best_value = std::numeric_limits<double>::min();
    std::shared_ptr<const Guess> best_guess;
    for (const auto& guess : guesses_)
    {
        if (best_value < guess->get_value())
        {
            best_value = guess->get_value();
            best_guess = guess;
        }
    }
    std::list<Shape*> results;
    std::list<const Sketch*> sources;
    std::string text;
    Rectangle frame = empty_frame();
    while (best_guess->get_parent())
    {
        // TODO post process (symetry, size, position) and draw unmatch sketches
        if (best_guess->is_complete())
        {
            // TODO check for shapes
            text += best_guess->get_character();
            // TODO extend frame based on guess location
        }
        best_guess = best_guess->get_parent();
    }
    if (!text.empty())
    {
        text.reserve();
        // TODO Check for matching text (width color style and size) in neighborhood, line or paragraph, then combine them
        // TODO Check for shapes around to link their name
        // TODO Adjust top and bottom
        Text* txt = new Text(width_, color_, style_);
        txt->set_text(text, frame);
        results.emplace_back(txt);
    }
    guesses_.clear();
    guesses_.emplace_back(Guess::head());
    board_.apply_ocr(sources, zoom_, results);
}

void Ocr::read_characters()
{
    auto path = std::filesystem::path("../language/characters");
    for (auto& json_file: std::filesystem::directory_iterator(path))
    {
        std::fstream json_reader(json_file.path());
        jsonio::json json_data;
        json_reader >> json_data;
        if (json_data.completed())
        {
            characters_.emplace_back(json_file.path().stem(), json_data);
        }
    }
}
