#include <algorithm>
#include <array>
#include <cmath>
#include <limits>
#include <numbers>
#include <utility>

#include "character.h"

namespace
{
constexpr double minimum_dimension = 1e-6;
constexpr double rotation_consistency_slack = std::numbers::pi / 36.0;
constexpr double rotation_consistency_unit = std::numbers::pi / 4.0;
constexpr double inverted_match_penalty = 0.2;
constexpr std::size_t max_match_states = 32;

struct PartMatch
{
    double diff = std::numeric_limits<double>::max();
    double shape_diff = std::numeric_limits<double>::max();
    double rotation = 0.0;
};

struct MatchState
{
    std::list<std::size_t> deficients;
    std::vector<double> rotations;
    std::vector<CharacterPartMatch> parts;
    double diff = 0.0;
};

void include_frame(Rectangle& destination, const Rectangle& source)
{
    destination[0][0] = std::min(destination[0][0], source[0][0]);
    destination[0][1] = std::min(destination[0][1], source[0][1]);
    destination[1][0] = std::max(destination[1][0], source[1][0]);
    destination[1][1] = std::max(destination[1][1], source[1][1]);
}

double width(const Rectangle& frame)
{
    return frame[1][0] - frame[0][0];
}

double height(const Rectangle& frame)
{
    return frame[1][1] - frame[0][1];
}

Point center(const Rectangle& frame)
{
    return
    {
        (frame[0][0] + frame[1][0]) / 2.0,
        (frame[0][1] + frame[1][1]) / 2.0,
    };
}

double dot(const Point& first, const Point& second)
{
    return first[0] * second[0] + first[1] * second[1];
}

Rectangle normalize_frame(const Rectangle& frame, const Rectangle& bounds)
{
    auto w = width(bounds);
    auto h = height(bounds);
    if (std::abs(w) < minimum_dimension ||
        std::abs(h) < minimum_dimension)
    {
        return {{ { 0.0, 0.0 }, { 1.0, 1.0 } }};
    }
    return
    {{
        {
            (frame[0][0] - bounds[0][0]) / w,
            (frame[0][1] - bounds[0][1]) / h,
        },
        {
            (frame[1][0] - bounds[0][0]) / w,
            (frame[1][1] - bounds[0][1]) / h,
        },
    }};
}

Rectangle project_frame(const Rectangle& frame, double rotation)
{
    const Point advance { std::cos(rotation), std::sin(rotation) };
    const Point height_axis { -std::sin(rotation), std::cos(rotation) };
    Rectangle projected = empty_frame();
    std::array<Point, 4> corners
    {{
        frame[0],
        { frame[1][0], frame[0][1] },
        frame[1],
        { frame[0][0], frame[1][1] },
    }};
    for (const auto& corner : corners)
    {
        Point point { dot(corner, advance), dot(corner, height_axis) };
        include_frame(projected, {{ point, point }});
    }
    return projected;
}

double rectangle_score(const Rectangle& expected, const Rectangle& observed)
{
    auto expected_center = center(expected);
    auto observed_center = center(observed);
    double center_score = get_distance(expected_center, observed_center) /
        std::sqrt(2.0);
    double expected_width = std::max(width(expected), 0.05);
    double expected_height = std::max(height(expected), 0.05);
    double observed_width = std::max(width(observed), 0.05);
    double observed_height = std::max(height(observed), 0.05);
    double size_score =
        std::abs(std::log(observed_width / expected_width)) +
        std::abs(std::log(observed_height / expected_height));
    return center_score * 0.7 + size_score * 0.15;
}

double average_rotation(const std::vector<double>& rotations)
{
    if (rotations.empty())
    {
        return 0.0;
    }

    double rotation_x = 0.0;
    double rotation_y = 0.0;
    for (double rotation : rotations)
    {
        rotation_x += std::cos(rotation);
        rotation_y += std::sin(rotation);
    }
    return std::atan2(rotation_y, rotation_x);
}

double rotation_consistency_score(const std::vector<double>& rotations)
{
    if (rotations.size() < 2)
    {
        return 0.0;
    }

    double average = average_rotation(rotations);
    double diff = 0.0;
    for (double rotation : rotations)
    {
        auto delta = std::abs(get_rotation(average, rotation));
        diff += std::max(0.0, delta - rotation_consistency_slack) /
            rotation_consistency_unit;
    }
    return diff / rotations.size();
}

PartMatch convex_score(const Convex& expected, const Convex& observed)
{
    auto direct = expected.compare_best_rotation(observed);
    Convex inverted = observed;
    inverted.invert();
    auto inverted_score = expected.compare_best_rotation(inverted);
    inverted_score.diff += inverted_match_penalty;
    if (inverted_score.diff < direct.diff)
    {
        return
        {
            .diff = inverted_score.diff,
            .shape_diff = inverted_score.diff,
            .rotation = inverted_score.rotation,
        };
    }
    return
    {
        .diff = direct.diff,
        .shape_diff = direct.diff,
        .rotation = direct.rotation,
    };
}

PartMatch part_score(const Character& character, std::size_t index,
    const Convex& observed, const Rectangle& candidate_frame)
{
    const auto& expected = character.get_convexes()[index];
    auto shape_score = convex_score(expected, observed);
    if (shape_score.diff > 0.9)
    {
        return shape_score;
    }
    auto expected_frame = normalize_frame(
        expected.get_frame(), character.get_frame());
    auto projected_candidate =
        project_frame(candidate_frame, shape_score.rotation);
    auto projected_observed =
        project_frame(observed.get_frame(), shape_score.rotation);
    auto observed_frame =
        normalize_frame(projected_observed, projected_candidate);
    double position_score = rectangle_score(expected_frame, observed_frame);
    return
    {
        .diff = shape_score.diff * 0.6 + position_score * 0.4,
        .shape_diff = shape_score.diff,
        .rotation = shape_score.rotation,
    };
}

double stored_part_score(const Character& character,
    const CharacterPartMatch& part, const Rectangle& candidate_frame)
{
    if (!part.matched)
    {
        return part.fault_diff;
    }

    const auto& expected = character.get_convexes()[part.index];
    auto expected_frame = normalize_frame(
        expected.get_frame(), character.get_frame());
    auto projected_candidate =
        project_frame(candidate_frame, part.rotation);
    auto projected_observed =
        project_frame(part.observed.get_frame(), part.rotation);
    auto observed_frame =
        normalize_frame(projected_observed, projected_candidate);
    double position_score = rectangle_score(expected_frame, observed_frame);
    return part.shape_diff * 0.6 + position_score * 0.4;
}

double parts_score_sum(const Character& character,
    const std::vector<CharacterPartMatch>& parts,
    const Rectangle& candidate_frame)
{
    double score = 0.0;
    for (const auto& part : parts)
    {
        score += stored_part_score(character, part, candidate_frame);
    }
    return score;
}

double state_score(const MatchState& state)
{
    return state.diff + rotation_consistency_score(state.rotations);
}

void prune_states(std::vector<MatchState>& states)
{
    std::stable_sort(states.begin(), states.end(),
        [](const MatchState& first, const MatchState& second)
        {
            return state_score(first) < state_score(second);
        });
    if (states.size() > max_match_states)
    {
        states.resize(max_match_states);
    }
}
}

Character::Character(std::string name, std::vector<Convex> convexes)
    : name_{ std::move(name) }
    , convexes_{ std::move(convexes) }
    , frame_{ empty_frame() }
{
    for (const auto& convex : convexes_)
    {
        include_frame(frame_, convex.get_frame());
    }
}

Character::~Character()
{
}

const std::string& Character::get_name() const
{
    return name_;
}

const std::vector<Convex>& Character::get_convexes() const
{
    return convexes_;
}

const Rectangle& Character::get_frame() const
{
    return frame_;
}

std::vector<CharacterMatch> Character::match(
    const std::list<std::size_t>& deficients,
    const std::vector<double>& rotations,
    const std::vector<Convex>& observed,
    const Rectangle& candidate_frame, double match_threshold,
    double fault_threshold, double fault_diameter,
    std::vector<CharacterPartMatch> initial_parts) const
{
    std::vector<MatchState> states
    {{
        .deficients = deficients,
        .rotations = rotations,
        .parts = std::move(initial_parts),
        .diff = 0.0,
    }};
    states.front().diff =
        parts_score_sum(*this, states.front().parts, candidate_frame);

    for (const auto& convex : observed)
    {
        std::vector<MatchState> next_states;
        for (const auto& state : states)
        {
            bool matched = false;
            for (auto index : state.deficients)
            {
                auto match = part_score(*this, index, convex, candidate_frame);
                if (match.diff > match_threshold)
                {
                    continue;
                }

                MatchState next = state;
                next.rotations.emplace_back(match.rotation);
                next.parts.emplace_back(CharacterPartMatch
                {
                    .matched = true,
                    .index = index,
                    .observed = convex,
                    .shape_diff = match.shape_diff,
                    .rotation = match.rotation,
                    .fault_diff = 0.0,
                });
                auto it = std::find(
                    next.deficients.begin(), next.deficients.end(), index);
                if (it != next.deficients.end())
                {
                    next.deficients.erase(it);
                }
                next.diff =
                    parts_score_sum(*this, next.parts, candidate_frame);
                next_states.emplace_back(std::move(next));
                matched = true;
            }

            if (!matched)
            {
                auto fault =
                    get_distance(convex.get_frame()[0],
                        convex.get_frame()[1]) /
                    std::max(fault_diameter, minimum_dimension);
                if (fault < fault_threshold)
                {
                    MatchState next = state;
                    next.parts.emplace_back(CharacterPartMatch
                    {
                        .matched = false,
                        .index = 0,
                        .observed = convex,
                        .shape_diff = 0.0,
                        .rotation = 0.0,
                        .fault_diff = fault,
                    });
                    next.diff =
                        parts_score_sum(*this, next.parts, candidate_frame);
                    next_states.emplace_back(std::move(next));
                }
            }
        }

        states = std::move(next_states);
        if (states.empty())
        {
            return {};
        }
        prune_states(states);
    }

    std::vector<CharacterMatch> matches;
    matches.reserve(states.size());
    for (auto& state : states)
    {
        std::size_t score_count = state.parts.size();
        double score_sum =
            parts_score_sum(*this, state.parts, candidate_frame);
        double diff =
            score_sum / std::max<std::size_t>(score_count, 1);
        if (state.deficients.empty())
        {
            diff += rotation_consistency_score(state.rotations);
        }
        matches.emplace_back(CharacterMatch
        {
            .diff = diff,
            .score_sum = score_sum,
            .score_count = score_count,
            .rotation = average_rotation(state.rotations),
            .deficients = std::move(state.deficients),
            .rotations = std::move(state.rotations),
            .parts = std::move(state.parts),
        });
    }
    std::stable_sort(matches.begin(), matches.end(),
        [](const CharacterMatch& first, const CharacterMatch& second)
        {
            return first.diff < second.diff;
        });
    return matches;
}
