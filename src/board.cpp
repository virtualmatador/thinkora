#include <array>
#include <cmath>
#include <set>

#include "bar.h"
#include "circle.h"
#include "toolbox.h"

#include "board.h"

Board::Board(Bar* bar)
    : zoom_{0}
    , center_{0, 0}
    , center_pre_pad_{0, 0}
    , mouse_position_{0, 0}
    , mouse_pre_pad_{0, 0}
    , mouse_button_{0}
    , bar_{bar}
{
    add_events(
        Gdk::EventMask::BUTTON_PRESS_MASK |
        Gdk::EventMask::BUTTON_RELEASE_MASK |
        Gdk::EventMask::SCROLL_MASK |
        Gdk::EventMask::BUTTON1_MOTION_MASK |
        Gdk::EventMask::BUTTON2_MOTION_MASK |
        Gdk::EventMask::BUTTON3_MOTION_MASK |
        Gdk::EventMask::POINTER_MOTION_MASK);
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
                remove_reference(shape, zoom);
                delete shape;
            }
        }
    }
}

void Board::redraw(bool pass_on)
{
    if (pass_on)
    {
        bar_->redraw(false);
    }
    queue_draw();
}

void Board::add_reference(Shape* shape, const int& zoom)
{
    auto frame = shape->frame_;
    regionize(frame);
    for (auto x = frame[0][0]; x <= frame[1][0]; ++x)
    {
        for (auto y = frame[0][1]; y <= frame[1][1]; ++y)
        {
            references_[zoom][{x, y}].insert(shape);
        }
    }
}

void Board::remove_reference(Shape* shape, const int& zoom)
{
    auto frame = shape->frame_;
    regionize(frame);
    for (auto x = frame[0][0]; x <= frame[1][0]; ++x)
    {
        for (auto y = frame[0][1]; y <= frame[1][1]; ++y)
        {
            references_[zoom][{x, y}].erase(shape);
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
    if (button_event->button == 1)
    {
        if (mouse_button_ == 0)
        {
            mouse_button_ = 1;
            redraw(true);
        }
    }
    else if (button_event->button == 2)
    {
        if (mouse_button_ == 0)
        {
            mouse_button_ = 2;
            center_pre_pad_ = center_;
            mouse_pre_pad_ = {int(button_event->x), int(button_event->y)};
            redraw(true);
        }
    }
    return true;
}

bool Board::on_motion_notify_event(GdkEventMotion* motion_event)
{
    if (mouse_button_ == 1)
    {
        mouse_position_ = get_input_position(motion_event->x, motion_event->y);
// TODO change with skatch and run OCR
///////////////////////////////////////////////////////////////////////////////
        Circle* circle = new Circle({
            {mouse_position_[0] - 10, mouse_position_[1] - 10},
            {mouse_position_[0] + 10, mouse_position_[1] + 10}});
        circle->set_color({1.0, 0, 0, 1.0});
        add_reference(circle, zoom_);
        redraw(true);
///////////////////////////////////////////////////////////////////////////////
    }
    else if (mouse_button_ == 2)
    {
        center_[0] = center_pre_pad_[0] + mouse_pre_pad_[0] -
            int(motion_event->x);
        center_[1] = center_pre_pad_[1] + mouse_pre_pad_[1] -
            int(motion_event->y);
        clamp_position();
        mouse_position_ = get_input_position(motion_event->x, motion_event->y);
        redraw(true);
    }
    else
    {
        mouse_position_ = get_input_position(motion_event->x, motion_event->y);
        bar_->redraw(false);
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
            redraw(true);
        }
    }
    else if (release_event->button == 2)
    {
        if (mouse_button_ == 2)
        {
            mouse_button_ = 0;
            redraw(true);
        }
    }
    return true;
}

bool Board::on_scroll_event(GdkEventScroll* scroll_event)
{
    int zoom;
    std::array<int, 2> center;
    switch (scroll_event->direction)
    {
    case GdkScrollDirection::GDK_SCROLL_UP:
        zoom = zoom_ + 1;
        center[0] = center_[0] + mouse_position_[0];
        center[1] = center_[1] + mouse_position_[1];
        if (!zoom_lag_.empty())
        {
            center[0] -= zoom_lag_.top()[0];
            center[1] -= zoom_lag_.top()[1];
        }
        if (check_zoom(zoom, center))
        {
            if (!zoom_lag_.empty())
            {
                zoom_lag_.pop();
            }
            mouse_position_ = get_input_position(scroll_event->x, scroll_event->y);
            redraw(true);
        }
        break;
    case GdkScrollDirection::GDK_SCROLL_DOWN:
        zoom = zoom_ - 1;
        center[0] = center_[0] - mouse_position_[0] / 2;
        center[1] = center_[1] - mouse_position_[1] / 2;
        if (check_zoom(zoom, center))
        {
            zoom_lag_.push({mouse_position_[0] % 2, mouse_position_[1] % 2});
            mouse_position_ = get_input_position(scroll_event->x, scroll_event->y);
            redraw(true);
        }
        break;
    }
	return true;
}

void Board::on_pad_origin()
{
    mouse_position_[0] -= center_[0];
    mouse_position_[1] -= center_[1];
    center_[0] = 0;
    center_[1] = 0;
    redraw(true);
}

void Board::clamp_position()
{
    center_[0] = std::max(center_[0], -position_limit_);
    center_[0] = std::min(center_[0], +position_limit_);
    center_[1] = std::max(center_[1], -position_limit_);
    center_[1] = std::min(center_[1], +position_limit_);
}

bool Board::check_zoom(const int& zoom, const std::array<int, 2>& center)
{
    if (center[0] >= -position_limit_ && center[0] <= position_limit_ &&
        center[1] >= -position_limit_ && center[1] <= position_limit_ &&
        zoom >= -zoom_limit_ && zoom <= zoom_limit_)
    {
        zoom_ = zoom;
        center_ = center;
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
