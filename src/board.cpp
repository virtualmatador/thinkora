#include <array>
#include <cmath>
#include <fstream>
#include <set>

#include "bar.h"
#include "circle.h"
#include "polyline.h"
#include "toolbox.h"

#include "board.h"

std::vector<std::vector<std::vector<double>>> Board::dashes_;

Board::Board(Bar* bar)
    : zoom_{0}
    , center_{0, 0}
    , modified_{false}
    , sketch_{nullptr}
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
        bar_->redraw(false);
    }
    queue_draw();
}

void Board::clear_data()
{
    shapes_lock_.lock();
    clear_map(shapes_);
    sketches_lock_.lock();
    shapes_lock_.unlock();
    clear_map(sketches_);
    sketches_lock_.unlock();
    if (sketch_)
    {
        delete sketch_;
    }
}

void Board::clear_map(Map& map)
{
    for (auto& [zoom, plane]: map)
    {
        for (auto& [position, shapes]: plane)
        {
            while (!shapes.empty())
            {
                auto shape = *shapes.begin();
                remove_reference(map, zoom, shape);
                if (shape->get_type() == Shape::Type::SKETCH)
                {
                    static_cast<Sketch*>(shape)->kill();
                }
                else
                {
                    delete shape;
                }                
            }
        }
    }
    map.clear();
}

void Board::add_reference(Map& map, const int& zoom, Shape* shape)
{
    auto frame = regionize(shape->get_frame(), +tile_size_);
    for (auto x = frame[0][0]; x <= frame[1][0]; ++x)
    {
        for (auto y = frame[0][1]; y <= frame[1][1]; ++y)
        {
            map[zoom][{x, y}].insert(shape);
        }
    }
}

void Board::remove_reference(Map& map, const int& zoom, Shape* shape)
{
    auto frame = regionize(shape->get_frame(), +tile_size_);
    for (auto x = frame[0][0]; x <= frame[1][0]; ++x)
    {
        for (auto y = frame[0][1]; y <= frame[1][1]; ++y)
        {
            map[zoom][{x, y}].erase(shape);
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
    shapes_lock_.lock();
    draw_layers(cr, shapes_, area);
    sketches_lock_.lock();
    shapes_lock_.unlock();
    draw_layers(cr, sketches_, area);
    sketches_lock_.unlock();
    if (sketch_)
    {
        sketch_->draw(cr, 0, area[0]);
    }
    return true;
}

void Board::draw_layers(const Cairo::RefPtr<Cairo::Context>& cr,
    const Map& references, std::array<std::array<int, 2>, 2> area) const
{
    for (int zoom_delta = draw_level_limit_; zoom_delta >= -draw_level_limit_;
        --zoom_delta)
    {
        auto layer = references.find(zoom_ - zoom_delta);
        if (layer != references.end())
        {
            std::set<const Shape*> targets;
            std::array<std::array<int, 2>, 2> view;
            view[0] = zoom(area[0], -zoom_delta);
            view[1] = zoom(area[1], -zoom_delta);
            view = regionize(view, +tile_size_);
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
}

bool Board::on_button_press_event(GdkEventButton* button_event)
{
    if (mouse_button_ == 0)
    {
        if (button_event->button == 1)
        {
            mouse_button_ = 1;
            sketch_ = new Sketch{bar_->marker_width_,
                bar_->marker_color_, bar_->marker_style_};
            sketch_->add_point(mouse_position_);
            redraw(true);
        }
        else if (button_event->button == 2)
        {
            mouse_button_ = 2;
            center_pre_pad_ = center_;
            mouse_pre_pad_ = {int(button_event->x), int(button_event->y)};
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
        mouse_position_ = get_input_position(motion_event->x, motion_event->y);
        sketch_->add_point(mouse_position_);
        redraw(true);
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
            sketch_->set_birth();
            sketch_->set_frame();
            sketches_lock_.lock();
            add_reference(sketches_, zoom_, sketch_);
            sketches_lock_.unlock();
            ocr_.push({sketch_, zoom_});
            sketch_ = nullptr;
            modified_ = true;
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
    if (mouse_button_ == 0)
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
                mouse_position_ =
                    get_input_position(scroll_event->x, scroll_event->y);
                redraw(true);
                return true;
            }
            break;
        case GdkScrollDirection::GDK_SCROLL_DOWN:
            zoom = zoom_ - 1;
            center[0] = center_[0] - mouse_position_[0] / 2;
            center[1] = center_[1] - mouse_position_[1] / 2;
            if (check_zoom(zoom, center))
            {
                zoom_lag_.push({mouse_position_[0] % 2,
                    mouse_position_[1] % 2});
                mouse_position_ =
                    get_input_position(scroll_event->x, scroll_event->y);
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
    mouse_position_ = get_input_position(crossing_event->x, crossing_event->y);
    bar_->redraw(false);
    return true;
}

void Board::on_save() const
{
    auto file_name =
        choose_file(Gtk::FileChooserAction::FILE_CHOOSER_ACTION_SAVE);
    if (!file_name.empty())
    {
        std::ofstream file(file_name);
        if (file)
        {
            shapes_lock_.lock();
            save_map(file, shapes_, false);
            sketches_lock_.lock();
            shapes_lock_.unlock();
            save_map(file, sketches_, true);
            sketches_lock_.unlock();
            modified_ = false;
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

void Board::save_map(std::ostream& os, const Map& map, const bool& sketch) const
{
    os << map.size() << std::endl;
    for (const auto [zoom, layer]: map)
    {
        std::set<const Shape*> targets;
        for (const auto [position, shapeset]: layer)
        {
            for (const auto& shape: shapeset)
            {
                targets.insert(shape);
            }
        }
        os << zoom << " " << targets.size() << std::endl << std::endl;
        for (const auto& target: targets)
        {
            if (!sketch)
            {
                os << int(target->get_type()) << std::endl;
            }
            os << *target << std::endl;
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
            modified_ = false;
            clear_data();
            shapes_lock_.lock();
            open_map(file, shapes_, false);
            sketches_lock_.lock();
            shapes_lock_.unlock();
            open_map(file, sketches_, true);
            sketches_lock_.unlock();
            redraw(true);
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

void Board::open_map(std::istream& is, Map& map, const bool& sketch)
{
    std::size_t size;
    std::vector<std::pair<Sketch*, int>> ocr_jobs;
    is >> size;
    for (std::size_t i = 0; i < size; ++i)
    {
        int zoom;
        Shape::Type type;
        Shape* shape;
        if (is >> zoom >> size)
        {
            for (std::size_t i = 0; i < size; ++i)
            {
                if (sketch)
                {
                    shape = new Sketch;
                    ocr_jobs.emplace_back(static_cast<Sketch*>(shape), zoom);
                }
                else
                {
                    is >> (int&)(type);
                    switch (type)
                    {
                    case Shape::Type::POLYLINE:
                        shape = new Polyline;
                        break;
                    case Shape::Type::CIRCLE:
                        shape = new Circle;
                        break;
                    default:
                        is.setstate(std::ios_base::badbit);
                        break;
                    }
                }
                is >> *shape;
                add_reference(map, zoom_, shape);
            }
        }
    }
    if (sketch)
    {
        std::sort(ocr_jobs.begin(), ocr_jobs.end(), [](const auto& a, const auto&b)
        {
            return a.first->get_birth() < b.first->get_birth();
        });
        for (const auto& ocr_job: ocr_jobs)
        {
            ocr_.push(ocr_job);
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
