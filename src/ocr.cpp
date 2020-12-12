#include <cstddef>
#include <filesystem>
#include <fstream>
#include <list>
#include <memory>
#include <vector>

#include "json.h"

#include "board.h"
#include "fit.h"
#include "line.h"
#include "shape.h"
#include "toolbox.h"

#include "ocr.h"

Ocr::Ocr(Board& board)
    : run_{ true }
    , zoom_{ 0 }
    , board_{ board }
{
    for (auto& json_file: std::filesystem::directory_iterator(
        "../language/patterns"))
    {
        std::fstream json_reader(json_file.path());
        jsonio::json json_pattern;
        json_reader >> json_pattern;
        if (json_pattern.completed())
        {
            patterns_.emplace_back(json_file.path().stem(), json_pattern);
        }
    }
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
        if (zoom_ != sketch->get_zoom())
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
            //auto similarity = pattern.match(convexes);
            //if (similarity > 0.0)
            {
                match = true;
                for (auto& guess : guesses_)
                {
                    //guesses.emplace_back(guess->extend(
                    //    pattern.get_name(), sketch->get_frame(), similarity));
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

/*
bool Ocr::do_job()
{
    std::shared_ptr<Job> job;
    {
        std::lock_guard<std::mutex> lock{jobs_lock_};
        if (jobs_.size() > 0)
        {
            job = std::move(jobs_.front());
            jobs_.pop_front();
        }
    }
    if (job)
    {
        auto time = std::chrono::steady_clock::now();
        if (time - job->sketch_->get_birth() > std::chrono::operator""ms(delay_ms_))
        {
            job->process();
            std::list<std::shared_ptr<Job>> partial_jobs;
            if (job->is_simple())
            {
                job->choice_ = 0;
            }
            // else if (job->need_base_line())
            // {
            //     // TODO get base line from left or right
            //     // Choose between lowercase and uppercase
            // }
            else if (!combine(job, partial_jobs))
            {
                return false;
            }
            board_->apply_ocr(job, partial_jobs);
            if (job)
            {
                partial_jobs_.emplace_back(std::move(job));
            }
            for (auto& partial_job: partial_jobs)
            {
                partial_jobs_.emplace_back(std::move(partial_job));
            }
            return false;
        }
        // modify using sticky points
        // create sticky points
        return true;
    }
    else
    {
        // TODO check all partial_jobs can be deleted from board
        
        // while (!partial_jobs_.empty())
        // {
        //     job = &partial_jobs_.front();
        //     job->choice_ = std::size_t(-1);
        //     if (board_->replace_sketches(job, nullptr, job->get_result()))
        //     {
        //         partial_jobs_.pop_front();
        //     }
        //     else
        //     {
        //         std::lock_guard<std::mutex> lock{jobs_lock_};
        //         jobs_.splice(jobs_.end(), partial_jobs_, partial_jobs_.begin());
        //     }
        // }
        return true;
    }
}

bool Ocr::combine(std::shared_ptr<Job>& job,
    std::list<std::shared_ptr<Job>>& partial_jobs)
{
    // TODO mix jobs together
    // if ()
    {
        std::lock_guard<std::mutex> lock{jobs_lock_};
        jobs_.emplace_back(std::move(job));
        return false;
    }
    return true;
}
*/