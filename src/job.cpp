#include "toolbox.h"

#include "circle.h"
#include "sketch.h"
#include "text.h"

#include "job.h"

std::vector<Pattern> Job::shape_patterns_;
std::map<std::string, std::vector<Pattern>> Job::char_patterns_;

Job::Job(Sketch* sketch, const int& zoom)
    : sketch_{sketch}
    , zoom_{zoom}
{
}

Job::~Job()
{
}

void Job::process()
{
    simplify();
    choice_ = std::size_t(-1);
    match();
}

bool Job::is_simple() const
{
    return pattern_->is_simple();
}

const int& Job::get_zoom() const
{
    return zoom_;
}

Sketch* Job::get_sketch() const
{
    return sketch_;
}

Shape* Job::get_result() const
{
    Shape* shape;
    /*
    if (choice_ == std::size_t(-1))
    {
        for (const auto& convex: convexes_)
        {
            // TODO Polyline* pl = new Polyline(element);
            //shapes.emplace_back(pl);
        }
        //return pl;
    }
    else
    {
        auto character = pattern_->get_character(choice_);
        if (character == "circle")
        {
            Circle* circle = new Circle(line_width_, color_, style_);
            circle->set_circle(get_center(frame_),
                std::pow(std::pow(get_diameter(frame_), 2.0) / 2.0, 0.5) / 2.0);
            shape = circle;
        }
        else
        {
            // TODO calc size
            
            Text* text = new Text(line_width_, color_, style_);
            auto region = Cairo::Region::create();
            auto dc = board_->get_window()->begin_draw_frame(region);
            text->set_text(dc->get_cairo_context(), frame_[0],
                frame_[1][1] - frame_[0][1], character);
            board_->get_window()->end_draw_frame(dc);
            shape = text;
            
        }
    }
    */
    return shape;
}

void Job::simplify()
{
    double tolerance = get_diameter(sketch_->get_frame()) / 24.0;
    std::vector<std::tuple<double, double, std::size_t>> redondents;
    do
    {
        redondents.clear();
        for (std::size_t i = 2; i < points_.size(); ++i)
        {
            double len1, len2;
            auto angle = get_angle(points_[i - 2], points_[i - 1], points_[i],
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
            points_.erase(points_.begin() + std::get<2>(redondent));
        }
    } while (redondents.size() > 0);
    convexes_ = Convex::extract(points_, sketch_->get_frame());
}

void Job::match()
{
    pattern_ = nullptr;
    double min_difference = Convex::treshold_;
    for (const auto& [language, patterns]: char_patterns_)
    {
        for (const auto& pattern: patterns)
        {
            double difference = 0.0; // TODO pattern.match(convexes_);
            if (min_difference > difference)
            {
                min_difference = difference;
                pattern_ = &pattern;
            }
        }
    }
    for (const auto& pattern: shape_patterns_)
    {
        double difference = 0.0; // TODO pattern.match(convexes_);
        if (min_difference > difference)
        {
            min_difference = difference;
            pattern_ = &pattern;
        }
    }
}
