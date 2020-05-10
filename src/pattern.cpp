#include "pattern.h"

Pattern::Pattern(const jsonio::json& pattern)
{
    character_ = pattern["name"].get_string();
    for (const auto& json_segment: pattern["info"].get_array())
    {
        std::vector<Convex> segment;
        for (const auto& json_convex: json_segment.get_array())
        {
            Convex convex;
            const jsonio::json_value* value;
            auto& json = json_convex.get_object();
            value = json.get_value("b_a");
            if (value)
            {
                convex.b_a_b_ = true;
                convex.b_a_ = json["b_a"].get_long();
                convex.b_x_ = json["b_x"].get_double();
                convex.b_y_ = json["b_y"].get_double();
            }
            else
            {
                convex.b_a_b_ = false;
            }
            value = json.get_value("e_a");
            if (value)
            {
                convex.e_a_b_ = true;
                convex.e_a_ = json["e_a"].get_long();
                convex.e_x_ = json["e_x"].get_double();
                convex.e_y_ = json["e_y"].get_double();
            }
            else
            {
                convex.e_a_b_ = false;
            }
            value = json.get_value("d_a");
            if (value)
            {
                convex.d_a_b_ = true;
                convex.d_a_ = json["d_a"].get_long();
            }
            else
            {
                convex.d_a_b_ = false;
            }
            convex.d_x_ = json["d_x"].get_double();
            convex.d_y_ = json["d_y"].get_double();
            convex.d_t_ = json["d_t"].get_long();
            convex.d_x_ = json["f_x"].get_double();
            convex.d_y_ = json["f_y"].get_double();
            convex.n_b_ = json["n_b"].get_long();
            convex.n_e_ = json["n_e"].get_long();
            segment.emplace_back(convex);
        }
        segments_.emplace_back(segment);
    }
}

Pattern::~Pattern()
{
}

double Pattern::Match(const std::vector<std::array<int, 2>>& points)
{
    return 0;
}
