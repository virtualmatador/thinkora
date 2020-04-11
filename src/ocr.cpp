#include <gsl/gsl_fit.h>

#include "board.h"
#include "circle.h"
#include "line.h"
#include "toolbox.h"

#include "ocr.h"

Ocr::Ocr(Board* board)
    : run_{true}
    , board_{board}
{
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
    auto sketches = board_->list_sketches(job);
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
    else if (sketches.size() == 0 || process(job, sketches))
    {
        std::lock_guard<std::mutex> lock{jobs_lock_};
        jobs_.pop_front();
    }
    return false;
}

bool Ocr::process(const Job* job, std::vector<Sketch>& sketches)
{
    std::vector<Shape*> elements;
    for (const auto& sketch: sketches)
    {
        simplify(job, elements, sketch.get_points(), 0, sketch.get_points().size());
    }
    std::vector<Shape*> shapes = combine(elements);
    // modify using sticky points
    // create sticky points
    return board_->replace_sketches(job, sketches, shapes);
}

void Ocr::simplify(const Job* job, std::vector<Shape*>& elements,
    const std::vector<std::array<int, 2>>& points,
    std::size_t begin, std::size_t count)
{
    //int gsl_fit_linear(const double * x, const size_t xstride, const double * y, const size_t ystride, size_t n, double * c0, double * c1, double * cov00, double * cov01, double * cov11, double * sumsq)
//    auto factors = calcregression(points, begin, count);
//    if (error < 10)
    {
        Line* line = new Line{job->line_width_, job->color_, job->style_};
        line->set_line(job->frame_);
        elements.emplace_back(line);
    }
//    else
    {
//        simplify(job, elements, points, begin, count / 2);
//        simplify(job, elements, points, begin + count / 2, count - count / 2);
    }    
}

std::vector<Shape*> Ocr::combine(std::vector<Shape*>& elements)
{
    std::vector<Shape*> shapes;
    for (const auto& element: elements)
    {
        shapes.emplace_back(element);
    }
    return shapes;
}
