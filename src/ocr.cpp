#include <cstddef>
#include <filesystem>
#include <fstream>
#include <memory>
#include <vector>

#include "json.h"

#include "board.h"
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
    // TODO jobs_.emplace_back(job);
}

bool Ocr::get_sketch()
{/*
    Job* job;
    {
        std::lock_guard<std::mutex> lock{jobs_lock_};
        if (jobs_.size() > 0)
        {
            job = &jobs_.front();
        }
        else
        {
            // TODO check all partial_jobs can be deleted from board
            while (!partial_jobs_.empty())
            {
                job = &partial_jobs_.front();
                job->choice_ = std::size_t(-1);
                if (board_->replace_sketches(job, nullptr, job->get_result()))
                {
                    partial_jobs_.pop_front();
                }
                else
                {
                    std::lock_guard<std::mutex> lock{jobs_lock_};
                    jobs_.splice(jobs_.end(), partial_jobs_, partial_jobs_.begin());
                }
            }
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
        time - recent > std::chrono::operator""ms(delay_ms_))
    {
        job->process();
        if (job->is_simple())
        {
            job->choice_ = 0;
            if (board_->replace_sketches(job, nullptr, job->get_result()))
            {
                std::lock_guard<std::mutex> lock{jobs_lock_};
                jobs_.pop_front();
            }
        }
        else if (auto match = combine(job); match != partial_jobs_.end())
        {
            if (board_->replace_sketches(job, &(*match), job->get_result()))
            {
                partial_jobs_.erase(match);
                std::lock_guard<std::mutex> lock{jobs_lock_};
                jobs_.pop_front();
            }
            else
            {
                std::lock_guard<std::mutex> lock{jobs_lock_};
                jobs_.splice(jobs_.end(), partial_jobs_, match);
            }
        }
        else
        {
            std::lock_guard<std::mutex> lock{jobs_lock_};
            partial_jobs_.splice(partial_jobs_.end(), jobs_, jobs_.begin());
        }
        return false;
    }
    // TODO Combine with text in left if size and position and style match
    // Choose between lowercase and uppercase
    // Combine . "
    // modify using sticky points
    // create sticky points*/
    return true;
}

std::list<Job>::iterator Ocr::combine(Job* job)
{
    // TODO mix jobs together
    return partial_jobs_.end();
}
