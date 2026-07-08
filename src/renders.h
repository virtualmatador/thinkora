#ifndef THINKORA_SRC_RENDERS_H
#define THINKORA_SRC_RENDERS_H

#include <span>
#include <string_view>

#include "toolbox.h"

struct RenderPath
{
    enum class Type
    {
        Polyline,
        Arc,
    };

    Type type;
    std::span<const Point> points;
    bool closed = false;
    Point center { 0.0, 0.0 };
    double radius = 0.0;
    double start_angle = 0.0;
    double end_angle = 0.0;
};

struct RenderSource
{
    std::string_view name;
    std::span<const RenderPath> paths;
};

std::span<const RenderSource> get_renders();

#endif // THINKORA_SRC_RENDERS_H
