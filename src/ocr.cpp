#include <array>
#include <cstddef>
#include <chrono>
#include <filesystem>
#include <fstream>
#include <list>
#include <memory>
#include <vector>

#include "json.h"

#include "board.h"
#include "line.h"
#include "shape.h"
#include "text.h"
#include "toolbox.h"

#include "ocr.h"

Ocr::Ocr(Board& board)
    : characters_{ read_json<Character>("characters") }
    , patterns_{ link_patterns(read_json<Pattern>("segments"), characters_) }
    , run_{ true }
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
        //sources.emplace_back(sketch);
        auto points = sketch->simplify();
        auto pt_1 = points.front();
        std::array<Gdk::RGBA, 2> colors { Gdk::RGBA("#FF0000"), Gdk::RGBA("#00FF00") };
        for (auto it = std::next(points.begin()); it != points.end(); ++it)
        {
            auto ln = new Line(4.0, colors[std::distance(it, points.begin()) % colors.size()], Shape::Style::DOT_DOT);
            ln->set_line({pt_1, *it});
            results.emplace_back(ln);
            pt_1 = *it;
        }
        board_.apply_ocr(sources, zoom_, results);
        return;


        // TODO check for edge
        if (false)
        {
            // TODO add edge
            apply();
        }
        else
        {
            std::list<std::pair<const Pattern&, double>> patterns;
            for (const auto& pattern : patterns_)
            {
                auto diff = pattern.match(points, sketch->get_frame());
                if (diff < 0.4)
                {
                    patterns.emplace_back(pattern, diff);
                }
            }
            auto guesses = extend(sketch, patterns);
            bool all_done = true;
            for (auto guess : guesses_)
            {
                if (!guess->is_done())
                {
                    all_done = false;
                    break;
                }
            }
            if (all_done)
            {
                guesses.clear();
                apply();
                guesses = extend(sketch, patterns);
            }
            std::swap(guesses_, guesses);
        }
    }
    else
    {
        apply();
    }
}

std::list<std::shared_ptr<Guess>> Ocr::extend(const Sketch* sketch,
    const std::list<std::pair<const Pattern&, double>>& patterns)
{
    decltype(guesses_) guesses;
    for (auto guess : guesses_)
    {
        auto child_guesses = guess->extend(sketch, patterns);
        if (!child_guesses.empty())
        {
            guesses.merge(std::move(child_guesses));
        }
        else
        {
            guesses.emplace_back(guess);
        }                
    }
    return guesses;
}

void Ocr::apply()
{
    double best_diff = std::numeric_limits<double>::max();
    std::shared_ptr<Guess> best_guess;
    for (const auto& guess : guesses_)
    {
        if (best_diff > guess->get_diff())
        {
            best_diff = guess->get_diff();
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
            extend_frame(frame, best_guess->get_frame()[0]);
            extend_frame(frame, best_guess->get_frame()[1]);
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

template<class T>
std::vector<T> Ocr::read_json(const std::string& folder)
{
    auto path = std::filesystem::path("../language") / folder;
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

std::vector<Pattern> Ocr::link_patterns(std::vector<Pattern> patterns,
    const std::vector<Character>& characters)
{
    std::map<std::string, Pattern&> map_patterns;
    for (auto& pattern : patterns)
    {
        map_patterns.insert({ pattern.get_name(), pattern });
    }
    for (const auto& character : characters)
    {
        for (const auto& pt : character.get_patterns())
        {
            map_patterns.find(pt.first)->second.add_character(
                character, std::distance(character.get_patterns().data(), &pt));
        }
    }
    return patterns;
}