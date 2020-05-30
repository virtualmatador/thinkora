#include "result.h"

Result::Result(const jsonio::json& json)
{
    top_ = json["top"].get_double();
    bottom_ = json["bottom"].get_double();
    combine_ = json["combine"].get_string();
    direction_ = json["direction"].get_string();
    character_ = json["char"].get_string();
}

Result::~Result()
{
}
