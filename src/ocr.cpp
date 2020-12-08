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
    Job::shape_patterns_ = read_patterns("../pattern/shape/");
    for (auto& dir: std::filesystem::directory_iterator("../pattern/character/"))
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
            if (do_job())
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

void Ocr::add(Sketch* sketch, const int& zoom)
{
    std::lock_guard<std::mutex> lock{jobs_lock_};
    sketch->set_job(jobs_.emplace_back(std::make_shared<Job>(sketch, zoom)));
}

void Ocr::cancel()
{
    // TODO
}

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
            /*
            else if (job->need_base_line())
            {
                // TODO get base line from left or right
                // Choose between lowercase and uppercase
            }
            */
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
        /*
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
        */
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
