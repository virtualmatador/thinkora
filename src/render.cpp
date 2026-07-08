#include <algorithm>
#include <array>
#include <cmath>
#include <istream>
#include <iterator>
#include <ostream>

#include "render.h"
#include "toolbox.h"

namespace
{
double zoom_factor(const int& zoom)
{
    if (zoom > 0)
    {
        return std::pow(2.0, zoom);
    }
    if (zoom < 0)
    {
        return 1.0 / std::pow(2.0, -zoom);
    }
    return 1.0;
}

}

void Render::set_render(const std::string& name,
    const Point& center, double cx, double cy, double rotation)
{
    name_ = name;
    center_ = center;
    cx_ = cx;
    cy_ = cy;
    rotation_ = rotation;
    update_frame();
}

const std::string& Render::get_name() const
{
    return name_;
}

const Point& Render::get_center() const
{
    return center_;
}

double Render::get_cx() const
{
    return cx_;
}

double Render::get_cy() const
{
    return cy_;
}

double Render::get_rotation() const
{
    return rotation_;
}

Shape::Type Render::get_type() const
{
    return Type::RENDER;
}

Point Render::render_point(const Point& point) const
{
    double x = (point[0] - 0.5) * cx_ * 2.0;
    double y = (point[1] - 0.5) * cy_ * 2.0;
    double c = std::cos(rotation_);
    double s = std::sin(rotation_);
    return
    {
        center_[0] + x * c - y * s,
        center_[1] + x * s + y * c,
    };
}

Point Render::draw_point(const Point& point, const int& zoom_delta,
    const Point& pad) const
{
    return transform(render_point(point), zoom_delta, pad);
}

void Render::update_frame()
{
    frame_ = empty_frame();
    std::array<Point, 4> corners
    {{
        { 0.0, 0.0 },
        { 1.0, 0.0 },
        { 1.0, 1.0 },
        { 0.0, 1.0 },
    }};
    for (const auto& corner : corners)
    {
        extend_frame(frame_, render_point(corner));
    }
}

void Render::append_render_transform(
    const Cairo::RefPtr<Cairo::Context>& cr, const int& zoom_delta,
    const Point& pad) const
{
    cr->translate(-pad[0], -pad[1]);
    auto z = zoom_factor(zoom_delta);
    cr->scale(z, z);
    cr->translate(center_[0], center_[1]);
    cr->rotate(rotation_);
    cr->scale(cx_ * 2.0, cy_ * 2.0);
    cr->translate(-0.5, -0.5);
}

const RenderSource* Render::get_render() const
{
    for (const auto& render : get_renders())
    {
        if (render.name == name_)
        {
            return &render;
        }
    }
    return nullptr;
}

void Render::draw_details(const Cairo::RefPtr<Cairo::Context>& cr,
    const int& zoom_delta, const Point& pad) const
{
    auto render = get_render();
    if (!render)
    {
        return;
    }

    bool has_round_cap = false;
    for (const auto& path : render->paths)
    {
        if (path.type == RenderPath::Type::Polyline)
        {
            const auto& points = path.points;
            if (points.empty())
            {
                continue;
            }
            if (points.size() == 1)
            {
                has_round_cap = true;
                auto point = draw_point(points[0], zoom_delta, pad);
                cr->move_to(point[0], point[1]);
                cr->line_to(point[0], point[1]);
            }
            else
            {
                auto first = draw_point(points[0], zoom_delta, pad);
                cr->move_to(first[0], first[1]);
                for (std::size_t i = 1; i < points.size(); ++i)
                {
                    auto point = draw_point(points[i], zoom_delta, pad);
                    cr->line_to(point[0], point[1]);
                }
                if (path.closed)
                {
                    cr->close_path();
                }
            }
        }
        else if (path.type == RenderPath::Type::Arc)
        {
            cr->save();
            append_render_transform(cr, zoom_delta, pad);
            cr->arc(path.center[0], path.center[1], path.radius,
                path.start_angle, path.end_angle);
            cr->restore();
        }
    }
    if (has_round_cap)
    {
        cr->set_line_cap(Cairo::Context::LineCap::ROUND);
    }
    cr->stroke();
}

void Render::write_dtails(std::ostream& os) const
{
    os << name_.size();
    os.write(name_.c_str(), name_.size());
    os << std::endl;
    os <<
        center_[0] << ' ' <<
        center_[1] << ' ' <<
        cx_ << ' ' <<
        cy_ << std::endl;
    os << rotation_ << std::endl;
}

void Render::read_details(std::istream& is)
{
    std::size_t size;
    is >> size;
    name_.resize(0);
    name_.reserve(size);
    std::copy_n(std::istreambuf_iterator(is), size,
        std::back_inserter(name_));
    is.ignore(1);
    is >>
        center_[0] >>
        center_[1] >>
        cx_ >>
        cy_;
    is >> rotation_;
    update_frame();
}
