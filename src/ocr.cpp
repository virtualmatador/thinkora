#include <cstddef>
#include <memory>
#include <vector>

#include <gsl/gsl_fit.h>

#include "board.h"
#include "circle.h"
#include "line.h"
#include "point.h"
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
    for (auto& sketch: sketches)
    {
        if (sketch.get_points().size() == 1)
        {
            Point* point =
                new Point{job->line_width_, job->color_, job->style_};
            point->set_point(sketch.get_points().front());
            elements.emplace_back(point);
        }
        else
        {
            simplify(sketch, elements);
        }
    }
    std::vector<Shape*> shapes = combine(elements);
    // modify using sticky points
    // create sticky points
    return board_->replace_sketches(job, sketches, shapes);
}

void Ocr::simplify(Sketch& sketch, std::vector<Shape*>& elements)
{
    auto& points = sketch.get_points();
    double tolerance = get_diameter(sketch.get_frame()) / 24.0;
    std::vector<std::tuple<double, double, std::size_t>> redondents;
    do
    {
        redondents.clear();
        for (std::size_t i = 2; i < points.size(); ++i)
        {
            double len1, len2;
            auto angle = get_angle(points[i - 2], points[i - 1], points[i],
                &len1, &len2);
            if (std::pow(std::cos(angle) + 1.0, 0.6) * std::min(len1, len2) < tolerance)
            {
                redondents.emplace_back(angle, len1 * len2, i - 1);
                ++i;
            }
        }
        std::sort(redondents.begin(), redondents.end(), [](auto& a, auto&b)
        {
            if (std::get<0>(a) == std::get<0>(b))
            {
                return std::get<1>(a) < std::get<1>(b);
            }
            return std::get<0>(a) > std::get<0>(b);
        });
        redondents.resize((redondents.size() + 1) / 2);
        std::sort(redondents.begin(), redondents.end(), [](auto& a, auto&b)
        {
            return std::get<2>(a) > std::get<2>(b);
        });
        for (const auto& redondent: redondents)
        {
            points.erase(points.begin() + std::get<2>(redondent));
        }
    } while (redondents.size() > 0);
    /*
    Check Segment
        Check convex subsegments
            Start: {region, angle}
            End: {region, angle}
            End - Start: {length, angle}
            Max-Frame
    */
    for (std::size_t i = 1; i < points.size(); ++i)
    {
        Line* line = new Line(sketch.get_line_width(), sketch.get_color(),
            sketch.get_style());
        line->set_line({points[i - 1], points[i]});
        elements.emplace_back(line);
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
