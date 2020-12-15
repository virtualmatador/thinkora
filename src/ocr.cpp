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
#include "text.h"
#include "toolbox.h"

#include "ocr.h"

Ocr::Ocr(Board& board)
    : characters_{ read_json<Character>("characters") }
    , patterns_{ link_patterns(read_json<Pattern>("segments"), characters_) }
    , run_{ true }
    , head_guess_{ std::make_shared<Guess>(nullptr, nullptr,
        std::size_t(-1), Rectangle({0.0, 0.0, 0.0, 0.0}), 0.0) }
    , zoom_{ 0 }
    , width_{ 0.0 }
    , color_{ Gdk::RGBA("#000000") }
    , style_{ Shape::Style::SIZE }
    , board_{ board }
{
    guesses_.emplace_back(head_guess_);
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
        width_ = sketch->get_width();
        color_ = sketch->get_color();
        style_ = sketch->get_style();
        sources_.emplace_back(sketch);
        auto points = sketch->simplify();
        if (false) //edge)
        {
            // add edge
            apply();
        }
        else
        {
            std::list<std::pair<const Pattern&, double>> patterns;
            for (const auto& pattern : patterns_)
            {
                auto diff = pattern.match(points, sketch->get_frame());
                if (diff < 0.2)
                {
                    patterns.emplace_back(pattern, diff);
                }
            }
            bool match = false;
            decltype(guesses_) guesses;
            for (auto guess : guesses_)
            {
                auto child_guesses = guess->extend(*sketch, patterns);
                if (!child_guesses.empty())
                {
                    match = true;
                    guesses.merge(std::move(child_guesses));
                }
                else
                {
                    guesses.emplace_back(guess);
                }                
            }
            std::swap(guesses_, guesses);
            if (!match)
            {
                // TODO check with shapes
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
    }
    else
    {
        apply();
    }
    
    /*
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
    double best_diff = std::numeric_limits<double>::max();
    bool is_complete = false;
    std::shared_ptr<Guess> best_guess;
    for (const auto& guess : guesses_)
    {
        if (!is_complete || guess->is_complete())
        {
            if (best_diff > guess->get_diff())
            {
                best_diff = guess->get_diff();
                is_complete = guess->is_complete();
                best_guess = guess;
            }
        }
    }
    std::string text;
    Rectangle frame;
    while(best_guess)
    {
        if (best_guess->is_complete() && best_guess->get_parent())
        {
            text += best_guess->get_character();
            frame = best_guess->get_frame();
        }
        best_guess = best_guess->get_parent();
    }
    if (!text.empty())
    {
        text.reserve();
        Text* txt = new Text(width_, color_, style_);
        // TODO calc size and frame
        txt->set_text(text, 1, frame);
        results_.emplace_back(txt);
    }
    guesses_.clear();
    guesses_.emplace_back(head_guess_);
    board_.apply_ocr(sources_, zoom_, results_);
    sources_.clear();
    results_.clear();
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