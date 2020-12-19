#include "sketch.h"

#include "pattern.h"

Pattern::Pattern(const std::string& name, const jsonio::json& pattern)
    : name_{ name }
    , frame_{ empty_frame() }
{
    for (const auto& json_point: pattern.get_array())
    {
        points_.push_back(
        {
            json_point[0].get_double(),
            json_point[1].get_double(),
        });
        extend_frame(frame_, points_.back());
    }
}

Pattern::~Pattern()
{
}

void Pattern::add_character(const Character& character, std::size_t index)
{
    characters_.emplace_back(character, index);
}

const std::string& Pattern::get_name() const
{
    return name_;
}

const std::vector<std::pair<const Character&, std::size_t>>&
    Pattern::get_characters() const
{
    return characters_;
}

double Pattern::match(
    const std::vector<Point>& points, const Rectangle& frame) const
{
    double total_dist = 0;
    if (points_.size() == 1)
    {
        if (points.size() != 1)
        {
            total_dist = 1.0;
        }
    }
    else if (points.size() ==1)
    {
        total_dist = 1.0;
    }
    else
    {
        double ratio = 
            (frame_[1][0] - frame_[0][0]) / (frame_[1][1] - frame_[0][1]) *
            (frame[1][1] - frame[0][1]) / (frame[1][0] - frame[0][0]);
        if (ratio < 4 && ratio > 0.25)
        {
            std::vector<Point> pts;
            for (const auto& point : points)
            {
                pts.push_back(
                {
                    (point[0] - frame[0][0]) * (frame_[1][0] - frame_[0][0]) /
                        (frame[1][0] - frame[0][0]) + frame_[0][0],
                    (point[1] - frame[0][1]) * (frame_[1][1] - frame_[0][1]) /
                        (frame[1][1] - frame[0][1]) + frame_[0][1],
                });
            }
            // TODO if path is polygon, arrange pts for best match
            auto s_it = pts.begin();
            auto p_it = points_.begin();
            auto p_it_next = std::next(p_it);
            for (;;)
            {
                auto p_angle = get_angle({
                    (*p_it_next)[0] - (*p_it)[0],
                    (*p_it_next)[1] - (*p_it)[1]});
                auto p_it_next_next = std::next(p_it_next);
                auto p_angle_next = p_angle;
                if (p_it_next_next != points_.end())
                {
                    p_angle_next = get_angle({
                        (*p_it_next_next)[0] - (*p_it_next)[0],
                        (*p_it_next_next)[1] - (*p_it_next)[1]});
                }
                Point p_v_start = *s_it, p_v_end =
                {
                    (*p_it_next)[0] - (*p_it)[0] + (*s_it)[0],
                    (*p_it_next)[1] - (*p_it)[1] + (*s_it)[1],
                };
                total_dist += get_distance(*p_it, *s_it);
                auto s_it_next = std::next(s_it);
                for (; s_it_next != pts.end(); ++s_it_next)
                {
                    auto s_angle = get_angle({
                        (*s_it_next)[0] - (*s_it)[0],
                        (*s_it_next)[1] - (*s_it)[1]});
                    if (std::abs(get_rotation(s_angle, p_angle)) >
                        std::abs(get_rotation(s_angle, p_angle_next)))
                    {
                        break;
                    }
                    total_dist += get_distance(*s_it_next,
                        get_nearst(*s_it_next, { p_v_start, p_v_end }));
                    s_it = s_it_next;
                }
                total_dist += get_distance(*p_it_next, *s_it);
                if (p_it_next_next == points_.end())
                {
                    break;
                }
                p_it = p_it_next;
                p_it_next = p_it_next_next;
            }
            auto s_it_next = std::next(s_it);
            for (; s_it_next != pts.end(); ++s_it_next)
            {
                total_dist += get_distance(*s_it, *s_it_next);
                s_it = s_it_next;
            }
            total_dist /= points_.size();
            total_dist /= get_distance(frame_[0], frame_[1]);
        }
        else
        {
            total_dist = 1.0;
        }
    }
    return total_dist;
}
