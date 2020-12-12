#include "result.h"

Result::Result(const jsonio::json& json)
{
    top_ = json["top"].get_double();
    bottom_ = json["bottom"].get_double();
}

Result::~Result()
{
}

std::size_t Result::get_size() const
{
    return patterns_.size();
}
