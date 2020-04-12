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
    std::vector<double> xs, ys;
    std::array<std::array<int, 2>, 2> frame;
    for (std::size_t i = begin; i < begin + 2; ++i)
    {
        xs.emplace_back(points[i][0]);
        ys.emplace_back(points[i][1]);
    }
    frame[0][0] = std::min(points[0][0], points[1][0]);
    frame[0][1] = std::min(points[0][1], points[1][1]);
    frame[1][0] = std::max(points[0][0], points[1][0]);
    frame[1][1] = std::max(points[0][1], points[1][1]);
    for (std::size_t i = begin + 2; i < begin + count; ++i)
    {
        if ((points[i - 1][0] - points[i - 2][0]) *
            (points[i][1] - points[i - 1][1]) ==
            (points[i - 1][1] - points[i - 2][1]) *
            (points[i][0] - points[i - 1][0]))
        {
            xs.back() = points[i][0];
            ys.back() = points[i][1];
        }
        else
        {
            xs.emplace_back(points[i][0]);
            ys.emplace_back(points[i][1]);
        }
        frame[0][0] = std::min(frame[0][0], points[i][0]);
        frame[0][1] = std::min(frame[0][1], points[i][1]);
        frame[1][0] = std::max(frame[1][0], points[i][0]);
        frame[1][1] = std::max(frame[1][1], points[i][1]);
    }
    if (frame[1][0] - frame[0][0] < frame[1][1] - frame[0][1])
    {
        std::swap(xs, ys);
    }
    double c0, c1, cov00, cov01, cov11, sumsq;
    gsl_fit_linear(xs.data(), 1, ys.data(), 1, xs.size(), &c0, &c1, &cov00, &cov01, &cov11, &sumsq);
//    if (error < 10)
    {
        Line* line = new Line{job->line_width_, job->color_, job->style_};
        if (frame[1][0] - frame[0][0] < frame[1][1] - frame[0][1])
        {
            line->set_line(
            {
                c0 + c1 * xs.front(), xs.front(),
                c0 + c1 * xs.back(), xs.back(),
            });
        }
        else
        {
            line->set_line(
            {
                xs.front(), c0 + c1 * xs.front(),
                xs.back(), c0 + c1 * xs.back(),
            });
        }
        elements.emplace_back(line);
    }
//    else
    {
//        simplify(job, elements, points, begin, count / 2 + 1);
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
