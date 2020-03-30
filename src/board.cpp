#include <array>
#include <cmath>
#include <set>

#include "circle.h"
#include "toolbox.h"

#include "board.h"

Board::Board()
    : zoom_{0}
    , center_{0, 0}
    , mouse_center_{0, 0}
    , mouse_position_{0, 0}
    , mouse_button_{0}
{
    add_events(
        Gdk::EventMask::BUTTON_PRESS_MASK |
        Gdk::EventMask::BUTTON_RELEASE_MASK |
        Gdk::EventMask::SCROLL_MASK |
        Gdk::EventMask::BUTTON1_MOTION_MASK |
        Gdk::EventMask::BUTTON2_MOTION_MASK |
        Gdk::EventMask::BUTTON3_MOTION_MASK);
}

Board::~Board()
{
    for (auto& [zoom, plane]: references_)
    {
        for (auto& [position, shapes]: plane)
        {
            while (!shapes.empty())
            {
                auto shape = *shapes.begin();
                remove_reference(shape);
                delete shape;
            }
        }
    }
}

void Board::add_reference(Shape* shape)
{
    auto frame = shape->frame_;
    regionize(frame);
    for (auto x = frame[0][0]; x <= frame[1][0]; ++x)
    {
        for (auto y = frame[0][1]; y <= frame[1][1]; ++y)
        {
            references_[zoom_][{x, y}].insert(shape);
        }
    }
}

void Board::remove_reference(Shape* shape)
{
    auto frame = shape->frame_;
    regionize(frame);
    for (auto x = frame[0][0]; x <= frame[1][0]; ++x)
    {
        for (auto y = frame[0][1]; y <= frame[1][1]; ++y)
        {
            references_[zoom_][{x, y}].erase(shape);
        }
    }
}

bool Board::on_draw(const Cairo::RefPtr<Cairo::Context>& cr)
{
    cr->set_source_rgb(0.0, 0.0, 0.0);
    cr->paint();
    const auto allocation = get_allocation();
    std::array<std::array<int, 2>, 2> area = 
    {{
        {
            center_[0] - allocation.get_width() / 2,
            center_[1] - allocation.get_height() / 2,
        },
        {
            center_[0] + allocation.get_width() / 2,
            center_[1] + allocation.get_height() / 2,
        }
    }};
    for (int zoom_delta = draw_level_limit_; zoom_delta >= -draw_level_limit_;
        --zoom_delta)
    {
        auto layer = references_.find(zoom_ - zoom_delta);
        if (layer != references_.end())
        {
            std::set<const Shape*> targets;
            std::array<std::array<int, 2>, 2> view;
            view[0] = zoom(area[0], -zoom_delta);
            view[1] = zoom(area[1], -zoom_delta);
            regionize(view);
            for (auto x = view[0][0]; x <= view[1][0]; ++x)
            {
                for (auto y = view[0][1]; y <= view[1][1]; ++y)
                {
                    auto shapeset = layer->second.find({x, y});
                    if (shapeset != layer->second.end())
                    {
                        for (const auto& shape: shapeset->second)
                        {
                            targets.insert(shape);
                        }
                    }
                }
            }
            for (const auto& target: targets)
            {
                target->draw(cr, zoom_delta, area[0]);
            }
        }
    }
    return true;
}

bool Board::on_button_press_event(GdkEventButton* button_event)
{
    const auto mouse = get_input_position(button_event->x, button_event->y);
    if (button_event->button == 1)
    {
        if (mouse_button_ == 0)
        {
            mouse_button_ = 1;
            on_change_view();
        }
    }
    else if (button_event->button == 2)
    {
        if (mouse_button_ == 0)
    {
            mouse_button_ = 2;
        mouse_center_ = center_;
        mouse_position_ = {button_event->x, button_event->y};
            on_change_view();
    }
    }
    return true;
}

bool Board::on_motion_notify_event(GdkEventMotion* motion_event)
{
    if (mouse_button_ == 1)
    {
// TODO change with skatch and run OCR
///////////////////////////////////////////////////////////////////////////////
        const auto mouse = get_input_position(motion_event->x, motion_event->y);
        Circle* circle = new Circle({{mouse[0] - 10, mouse[1] - 10}, {mouse[0] + 10, mouse[1] + 10}});
        circle->set_color({1.0, 0, 0, 1.0});
        add_reference(circle);
        on_change_view();
///////////////////////////////////////////////////////////////////////////////
    }
    else if (mouse_button_ == 2)
    {
        center_[0] = mouse_center_[0] + int(mouse_position_[0] - motion_event->x);
        center_[1] = mouse_center_[1] + int(mouse_position_[1] - motion_event->y);
        on_change_position();
    }
    return true;
}

bool Board::on_button_release_event(GdkEventButton* release_event)
{
    if (release_event->button == 1)
    {
        if (mouse_button_ == 1)
        {
            mouse_button_ = 0;
            on_change_view();
        }
    }
    else if (release_event->button == 2)
    {
        if (mouse_button_ == 2)
    {
            mouse_button_ = 0;
            on_change_view();
        }
    }
    return true;
}

bool Board::on_scroll_event(GdkEventScroll* scroll_event)
{
    const auto mouse = get_input_position(scroll_event->x, scroll_event->y);
    int zoom;
    std::array<int, 2> center;
    switch (scroll_event->direction)
    {
    case GdkScrollDirection::GDK_SCROLL_UP:
        zoom = zoom_ + 1;
        center[0] = center_[0] + mouse[0];
        center[1] = center_[1] + mouse[1];
        if (!zoom_lag_.empty())
        {
            center[0] -= zoom_lag_.top()[0];
            center[1] -= zoom_lag_.top()[1];
        }
        if (on_change_zoom(zoom, center))
        {
            if (!zoom_lag_.empty())
            {
                zoom_lag_.pop();
            }
        }
        break;
    case GdkScrollDirection::GDK_SCROLL_DOWN:
        zoom = zoom_ - 1;
        center[0] = center_[0] - mouse[0] / 2;
        center[1] = center_[1] - mouse[1] / 2;
        if (on_change_zoom(zoom, center))
        {
            zoom_lag_.push({mouse[0] % 2, mouse[1] % 2});
        }
        break;
    }
	return true;
}

void Board::on_change_view()
{
    queue_draw();
}

void Board::on_change_position()
{
    center_[0] = std::max(center_[0], -position_limit_);
    center_[0] = std::min(center_[0], +position_limit_);
    center_[1] = std::max(center_[1], -position_limit_);
    center_[1] = std::min(center_[1], +position_limit_);
    queue_draw();
}

bool Board::on_change_zoom(const int& zoom, const std::array<int, 2>& center)
{
    if (center[0] >= -position_limit_ && center[0] <= position_limit_ &&
        center[1] >= -position_limit_ && center[1] <= position_limit_ &&
        zoom >= -zoom_limit_ && zoom <= zoom_limit_)
    {
        zoom_ = zoom;
        center_ = center;
        queue_draw();
        return true;
    }
    return false;
}

std::array<int, 2> Board::get_input_position(const int& x, const int& y) const
{
    const auto allocation = get_allocation();
    int xx = int(x - allocation.get_width() / 2) + center_[0];
    int yy = int(y - allocation.get_height() / 2) + center_[1];
    return {xx, yy};
}

void Board::regionize(std::array<std::array<int, 2>, 2>& frame)
{
    auto divide = [&](int& position)
    {
        position = position / tile_size_ - (position % tile_size_ < 0 ? 1 : 0);
    };
    divide(frame[0][0]);
    divide(frame[0][1]);
    divide(frame[1][0]);
    divide(frame[1][1]);
}
