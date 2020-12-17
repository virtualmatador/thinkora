#include <array>
#include <cmath>
#include <fstream>
#include <set>

#include "bar.h"
#include "circle.h"
#include "line.h"
#include "dot.h"
#include "toolbox.h"

#include "board.h"

std::vector<std::vector<std::vector<double>>> Board::dashes_;

Board::Board(Bar& bar)
    : zoom_{ 0 }
    , center_{ 0.0, 0.0 }
    , modified_{ false }
    , sketch_{ nullptr }
    , center_pre_pad_{ 0.0, 0.0 }
    , mouse_position_{ 0.0, 0.0 }
    , mouse_pre_pad_{ 0.0, 0.0 }
    , mouse_button_{ 0 }
    , ocr_{ *this }
    , bar_{ bar }
{
    add_events(
        Gdk::EventMask::BUTTON_PRESS_MASK |
        Gdk::EventMask::BUTTON_RELEASE_MASK |
        Gdk::EventMask::SCROLL_MASK |
        Gdk::EventMask::BUTTON1_MOTION_MASK |
        Gdk::EventMask::BUTTON2_MOTION_MASK |
        Gdk::EventMask::BUTTON3_MOTION_MASK |
        Gdk::EventMask::POINTER_MOTION_MASK |
        Gdk::EventMask::ENTER_NOTIFY_MASK);
    for (std::size_t i = 1; i <= width_limit_; ++i)
    {
        dashes_.emplace_back(std::vector<std::vector<double>>(
        {
            {},
            {4.0 * i, 4.0 * i},
            {4.0 * i, 4.0 * i, 1.0 * i, 4.0 * i},
            {1.0 * i, 4.0 * i},
        }));
    }
    queue_draw_.connect(sigc::mem_fun(*this, &Widget::queue_draw));
}

Board::~Board()
{
    clear_data();
}

bool Board::check_modified()
{
    if (modified_)
    {
        Gtk::MessageDialog error_message(*(Gtk::Window*)get_toplevel(),
            "You have unsaved changes. Do you want to save them?",
            false, Gtk::MessageType::MESSAGE_WARNING,
            Gtk::ButtonsType::BUTTONS_YES_NO, true);
        if (error_message.run() == Gtk::ResponseType::RESPONSE_YES)
        {
            return true;
        }
    }
    return false;
}

void Board::redraw(bool pass_on)
{
    if (pass_on)
    {
        bar_.redraw(false);
    }
    queue_draw();
}

void Board::apply_ocr(const std::list<const Sketch*>& sources, int zoom,
    const std::list<Shape*>& results)
{
    shapes_lock_.lock();
    for (auto& source : sources)
    {
        remove_reference(zoom, source);
        delete source;
    }
    for (auto& result : results)
    {
        add_reference(zoom, result);
    }
    shapes_lock_.unlock();
    queue_draw_();
}

void Board::clear_data()
{
    ocr_.cancel();
    shapes_lock_.lock();
    for (auto& [zoom, plane]: shapes_)
    {
        for (auto& [position, shapes]: plane)
        {
            while (!shapes.empty())
            {
                auto shape = *shapes.begin();
                remove_reference(zoom, shape);
                delete shape;
            }
        }
    }
    shapes_.clear();
    shapes_lock_.unlock();
}

void Board::add_reference(const int& zoom, const Shape* shape)
{
    auto frame = regionize(shape->get_frame());
    auto& layer = shapes_[zoom];
    for (int x = frame[0][0]; x <= frame[1][0]; ++x)
    {
        for (int y = frame[0][1]; y <= frame[1][1]; ++y)
        {
            layer[{x, y}].insert(shape);
        }
    }
}

void Board::remove_reference(const int& zoom, const Shape* shape)
{
    auto frame = regionize(shape->get_frame());
    auto layer = shapes_.find(zoom);
    for (int x = frame[0][0]; x <= frame[1][0]; ++x)
    {
        for (int y = frame[0][1]; y <= frame[1][1]; ++y)
        {
            auto region = layer->second.find({x, y});
            region->second.erase(shape);
            if (region->second.size() == 0)
            {
                layer->second.erase(region);
            }
        }
    }
    if (layer->second.size() == 0)
    {
        shapes_.erase(layer);
    }
}

bool Board::on_draw(const Cairo::RefPtr<Cairo::Context>& cr)
{
    cr->set_source_rgb(0.0, 0.0, 0.0);
    cr->paint();
    const auto allocation = get_allocation();
    Rectangle area = 
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
    shapes_lock_.lock();
    for (int zoom_delta = draw_level_limit_; zoom_delta >= -draw_level_limit_;
        --zoom_delta)
    {
        Rectangle view;
        view[0] = apply_zoom(area[0], -zoom_delta);
        view[1] = apply_zoom(area[1], -zoom_delta);
        view = regionize(view);
        std::set<const Shape*> shapes;
        auto layer = shapes_.find(zoom_ - zoom_delta);
        if (layer != shapes_.end())
        {
            for (int x = view[0][0]; x <= view[1][0]; ++x)
            {
                for (int y = view[0][1]; y <= view[1][1]; ++y)
                {
                    auto shapeset = layer->second.find({x, y});
                    if (shapeset != layer->second.end())
                    {
                        for (auto& shape: shapeset->second)
                        {
                            shapes.insert(shape);
                        }
                    }
                }
            }
        }
        for (const auto& shape: shapes)
        {
            shape->draw(cr, zoom_delta, area[0]);
        }
    }
    shapes_lock_.unlock();
    if (sketch_)
    {
        sketch_->draw(cr, 0, area[0]);
    }
    return true;
}

bool Board::on_button_press_event(GdkEventButton* button_event)
{
    if (mouse_button_ == 0)
    {
        if (button_event->button == 1)
        {
            mouse_button_ = 1;
            bar_.set_sensitive(false);
            sketch_ = new Sketch{ bar_.marker_width_,
                bar_.marker_color_, bar_.marker_style_ };
            sketch_->set_sketch(zoom_);
            sketch_->add_point(mouse_position_);
            redraw(true);
        }
        else if (button_event->button == 2)
        {
            mouse_button_ = 2;
            bar_.set_sensitive(false);
            center_pre_pad_ = center_;
            mouse_pre_pad_ = { button_event->x, button_event->y };
            redraw(true);
        }
        return true;
    }
    return false;
}

bool Board::on_motion_notify_event(GdkEventMotion* motion_event)
{
    if (mouse_button_ == 1)
    {
        mouse_position_ = get_input_position(
            { motion_event->x, motion_event->y });
        sketch_->add_point(mouse_position_);
        redraw(true);
    }
    else if (mouse_button_ == 2)
    {
        center_[0] = center_pre_pad_[0] + mouse_pre_pad_[0] -
            motion_event->x;
        center_[1] = center_pre_pad_[1] + mouse_pre_pad_[1] -
            motion_event->y;
        clamp_position();
        mouse_position_ = get_input_position(
            { motion_event->x, motion_event->y });
        redraw(true);
    }
    else
    {
        mouse_position_ = get_input_position(
            { motion_event->x, motion_event->y });
        bar_.redraw(false);
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
            shapes_lock_.lock();
            add_reference(zoom_, sketch_);
            shapes_lock_.unlock();
            ocr_.add(sketch_);
            sketch_ = nullptr;
            modified_ = true;
            redraw(true);
            bar_.set_sensitive(true);
        }
    }
    else if (release_event->button == 2)
    {
        if (mouse_button_ == 2)
        {
            mouse_button_ = 0;
            redraw(true);
            bar_.set_sensitive(true);
        }
    }
    return true;
}

bool Board::on_scroll_event(GdkEventScroll* scroll_event)
{
    if (mouse_button_ == 0)
    {
        int zoom;
        Point center;
        switch (scroll_event->direction)
        {
        case GdkScrollDirection::GDK_SCROLL_UP:
            zoom = zoom_ + 1;
            center[0] = center_[0] + mouse_position_[0];
            center[1] = center_[1] + mouse_position_[1];
            if (!zoom_lag_.empty())
            {
                center[0] += zoom_lag_.top()[0];
                center[1] += zoom_lag_.top()[1];
            }
            if (check_zoom(zoom, center))
            {
                zoom_ = zoom;
                center_ = center;
                if (!zoom_lag_.empty())
                {
                    zoom_lag_.pop();
                }
                mouse_position_ =
                    get_input_position({ scroll_event->x, scroll_event->y });
                redraw(true);
                return true;
            }
            break;
        case GdkScrollDirection::GDK_SCROLL_DOWN:
            zoom = zoom_ - 1;
            center[0] = center_[0] - mouse_position_[0] / 2.0;
            center[1] = center_[1] - mouse_position_[1] / 2.0;
            if (check_zoom(zoom, center))
            {
                auto pre_center = center_;
                zoom_ = zoom;
                center_ = center;
                mouse_position_ = get_input_position(
                    { scroll_event->x, scroll_event->y });
                zoom_lag_.push(
                {
                    pre_center[0] - (center_[0] + mouse_position_[0]),
                    pre_center[1] - (center_[1] + mouse_position_[1]),
                });
                redraw(true);
                return true;
            }
            break;
        }
    }
	return false;
}

bool Board::on_enter_notify_event(GdkEventCrossing* crossing_event)
{
    mouse_position_ = get_input_position(
        { crossing_event->x, crossing_event->y });
    bar_.redraw(false);
    return true;
}

void Board::on_save()
{
    auto file_name =
        choose_file(Gtk::FileChooserAction::FILE_CHOOSER_ACTION_SAVE);
    if (!file_name.empty())
    {
        std::ofstream file(file_name);
        if (file)
        {
            shapes_lock_.lock();
            file << shapes_.size() << std::endl;
            for (const auto [zoom, layer]: shapes_)
            {
                std::set<const Shape*> shapes;
                for (const auto [position, shapeset]: layer)
                {
                    for (const auto& shape: shapeset)
                    {
                        shapes.insert(shape);
                    }
                }
                file << zoom << " " << shapes.size() << std::endl << std::endl;
                for (const auto& shape: shapes)
                {
                    file << static_cast<int>(shape->get_type()) << std::endl;
                    file << *shape << std::endl;
                }
            }
            modified_ = false;
            shapes_lock_.unlock();
        }
        else
        {
            Gtk::MessageDialog error_message(*(Gtk::Window*)get_toplevel(),
                "Failed to save file.", false, Gtk::MessageType::MESSAGE_ERROR,
                Gtk::ButtonsType::BUTTONS_OK, true);
            error_message.run();
        }
    }
}

void Board::on_open()
{
    if (check_modified())
    {
        return;
    }
    auto file_name =
        choose_file(Gtk::FileChooserAction::FILE_CHOOSER_ACTION_OPEN);
    if (!file_name.empty())
    {
        std::ifstream file(file_name);
        if (file)
        {
            clear_data();
            shapes_lock_.lock();
            std::size_t zoom_count;
            file >> zoom_count;
            for (std::size_t i = 0; i < zoom_count; ++i)
            {
                int zoom;
                int type;
                Shape* shape;
                std::size_t shape_count;
                if (file >> zoom >> shape_count)
                {
                    for (std::size_t i = 0; i < shape_count; ++i)
                    {
                        file >> type;
                        shape = Shape::create_shape(
                            static_cast<Shape::Type>(type));
                        file >> *shape;
                        add_reference(zoom, shape);
                    }
                }
            }
            modified_ = false;
            shapes_lock_.unlock();
            redraw(true);
            // TODO Pass sketches to ocr
        }
        else
        {
            Gtk::MessageDialog error_message(*(Gtk::Window*)get_toplevel(),
                "Failed to open file.", false, Gtk::MessageType::MESSAGE_ERROR,
                Gtk::ButtonsType::BUTTONS_OK, true);
            error_message.run();
        }
    }
}

std::string Board::choose_file(Gtk::FileChooserAction action) const
{
    Gtk::FileChooserDialog file_chooser(*(Gtk::Window*)get_toplevel(),
        "Thinkora", action);
    const char* select_button;
    switch (action)
    {
    case Gtk::FileChooserAction::FILE_CHOOSER_ACTION_OPEN:
        select_button = "Open";
        break;
    case Gtk::FileChooserAction::FILE_CHOOSER_ACTION_SAVE:
        select_button = "Save";
        break;
    }
    file_chooser.add_button(select_button, Gtk::RESPONSE_OK);
    file_chooser.add_button("Cancel", Gtk::RESPONSE_CANCEL);
    auto file_filter = Gtk::FileFilter::create();
    file_filter->set_name("Thinkora files");
    file_filter->add_pattern("*.thinkora");
    file_chooser.add_filter(file_filter);
    if (file_chooser.run() == Gtk::ResponseType::RESPONSE_OK)
    {
        return file_chooser.get_filename();
    }
    return "";
}

void Board::on_origin()
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

bool Board::check_zoom(const int& zoom, const Point& center) const
{
    return center[0] >= -position_limit_ && center[0] <= position_limit_ &&
        center[1] >= -position_limit_ && center[1] <= position_limit_ &&
        zoom >= -zoom_limit_ && zoom <= zoom_limit_;
}

Point Board::get_input_position(const Point& point) const
{
    const auto allocation = get_allocation();
    return {
        point[0] - allocation.get_width() / 2.0 + center_[0],
        point[1] - allocation.get_height() / 2.0 + center_[1]
    };
}
