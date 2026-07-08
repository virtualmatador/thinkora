#include <array>
#include <cmath>
#include <fstream>
#include <set>

#include "bar.h"

#include "board.h"

namespace
{
int run_dialog(Gtk::Dialog& dialog)
{
    int response = Gtk::ResponseType::NONE;
    auto loop = Glib::MainLoop::create();
    dialog.signal_response().connect([&](int id)
    {
        response = id;
        dialog.set_visible(false);
        loop->quit();
    });
    dialog.present();
    loop->run();
    return response;
}

Gtk::Window& parent_window(Gtk::Widget& widget)
{
    return *dynamic_cast<Gtk::Window*>(widget.get_root());
}
}

std::vector<std::vector<std::vector<double>>> Board::dashes_;

Board::Board(Bar& bar)
    : zoom_{ 0 }
    , center_{ 0.0, 0.0 }
    , modified_{ false }
    , sketch_{ nullptr }
    , center_pre_pad_{ 0.0, 0.0 }
    , mouse_position_{ 0.0, 0.0 }
    , pointer_position_{ 0.0, 0.0 }
    , mouse_pre_pad_{ 0.0, 0.0 }
    , mouse_button_{ 0 }
    , ocr_{ *this }
    , bar_{ bar }
{
    set_draw_func(sigc::mem_fun(*this, &Board::on_draw));

    auto primary_click = Gtk::GestureClick::create();
    primary_click->set_button(1);
    primary_click->signal_pressed().connect([this](int, double x, double y)
    {
        on_button_press(1, x, y);
    });
    primary_click->signal_released().connect([this](int, double x, double y)
    {
        on_button_release(1, x, y);
    });
    add_controller(primary_click);

    auto middle_click = Gtk::GestureClick::create();
    middle_click->set_button(2);
    middle_click->signal_pressed().connect([this](int, double x, double y)
    {
        on_button_press(2, x, y);
    });
    middle_click->signal_released().connect([this](int, double x, double y)
    {
        on_button_release(2, x, y);
    });
    add_controller(middle_click);

    auto motion = Gtk::EventControllerMotion::create();
    motion->signal_enter().connect(sigc::mem_fun(*this, &Board::on_enter));
    motion->signal_motion().connect(sigc::mem_fun(*this, &Board::on_motion));
    add_controller(motion);

    auto scroll = Gtk::EventControllerScroll::create();
    scroll->set_flags(Gtk::EventControllerScroll::Flags::VERTICAL |
        Gtk::EventControllerScroll::Flags::DISCRETE);
    scroll->signal_scroll().connect(sigc::mem_fun(*this, &Board::on_scroll),
        false);
    add_controller(scroll);

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
    queue_draw_.connect(sigc::mem_fun(*this, &Gtk::Widget::queue_draw));
    Ocr::read_characters();
    Ocr::read_shapes();
}

Board::~Board()
{
    finish_ocr();
    clear_data();
}

bool Board::check_modified()
{
    if (modified_)
    {
        Gtk::MessageDialog error_message(parent_window(*this),
            "You have unsaved changes. Do you want to save them?",
            false, Gtk::MessageType::WARNING, Gtk::ButtonsType::YES_NO, true);
        if (run_dialog(error_message) == Gtk::ResponseType::YES)
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

bool Board::is_drawing()
{
    return mouse_button_ == 1;
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
    std::set<const Shape*> shapes;
    shapes_lock_.lock();
    for (auto& [zoom, plane]: shapes_)
    {
        for (auto& [position, region_shapes]: plane)
        {
            for (auto shape: region_shapes)
            {
                shapes.insert(shape);
            }
        }
    }
    shapes_.clear();
    shapes_lock_.unlock();
    for (auto shape: shapes)
    {
        delete shape;
    }
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

void Board::on_draw(const Cairo::RefPtr<Cairo::Context>& cr, int, int)
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
}

void Board::on_button_press(unsigned int button, double x, double y)
{
    pointer_position_ = { x, y };
    mouse_position_ = get_input_position(pointer_position_);
    if (mouse_button_ == 0)
    {
        if (button == 1)
        {
            mouse_button_ = 1;
            bar_.set_sensitive(false);
            sketch_ = new Sketch{ bar_.marker_width_,
                bar_.marker_color_, bar_.marker_style_ };
            sketch_->set_sketch(zoom_);
            sketch_->add_point(mouse_position_);
            redraw(true);
        }
        else if (button == 2)
        {
            mouse_button_ = 2;
            bar_.set_sensitive(false);
            center_pre_pad_ = center_;
            mouse_pre_pad_ = pointer_position_;
            redraw(true);
        }
    }
}

void Board::on_motion(double x, double y)
{
    pointer_position_ = { x, y };
    if (mouse_button_ == 1)
    {
        mouse_position_ = get_input_position(pointer_position_);
        sketch_->add_point(mouse_position_);
        redraw(true);
    }
    else if (mouse_button_ == 2)
    {
        center_[0] = center_pre_pad_[0] + mouse_pre_pad_[0] -
            pointer_position_[0];
        center_[1] = center_pre_pad_[1] + mouse_pre_pad_[1] -
            pointer_position_[1];
        clamp_position();
        mouse_position_ = get_input_position(pointer_position_);
        redraw(true);
    }
    else
    {
        mouse_position_ = get_input_position(pointer_position_);
        bar_.redraw(false);
    }
}

void Board::on_button_release(unsigned int button, double x, double y)
{
    pointer_position_ = { x, y };
    mouse_position_ = get_input_position(pointer_position_);
    if (button == 1)
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
    else if (button == 2)
    {
        if (mouse_button_ == 2)
        {
            mouse_button_ = 0;
            redraw(true);
            bar_.set_sensitive(true);
        }
    }
}

bool Board::on_scroll(double, double dy)
{
    if (mouse_button_ == 0)
    {
        int zoom;
        Point center;
        if (dy < 0.0)
        {
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
                mouse_position_ = get_input_position(pointer_position_);
                redraw(true);
                return true;
            }
        }
        else if (dy > 0.0)
        {
            zoom = zoom_ - 1;
            center[0] = center_[0] - mouse_position_[0] / 2.0;
            center[1] = center_[1] - mouse_position_[1] / 2.0;
            if (check_zoom(zoom, center))
            {
                auto pre_center = center_;
                zoom_ = zoom;
                center_ = center;
                mouse_position_ = get_input_position(pointer_position_);
                zoom_lag_.push(
                {
                    pre_center[0] - (center_[0] + mouse_position_[0]),
                    pre_center[1] - (center_[1] + mouse_position_[1]),
                });
                redraw(true);
                return true;
            }
        }
    }
	return false;
}

void Board::on_enter(double x, double y)
{
    pointer_position_ = { x, y };
    mouse_position_ = get_input_position(pointer_position_);
    bar_.redraw(false);
}

void Board::on_save()
{
    finish_ocr();
    auto file_name =
        choose_file(Gtk::FileChooser::Action::SAVE);
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
            Gtk::MessageDialog error_message(parent_window(*this),
                "Failed to save file.", false, Gtk::MessageType::ERROR,
                Gtk::ButtonsType::OK, true);
            run_dialog(error_message);
        }
    }
}

void Board::on_open()
{
    finish_ocr();
    if (check_modified())
    {
        return;
    }
    auto file_name =
        choose_file(Gtk::FileChooser::Action::OPEN);
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
        }
        else
        {
            Gtk::MessageDialog error_message(parent_window(*this),
                "Failed to open file.", false, Gtk::MessageType::ERROR,
                Gtk::ButtonsType::OK, true);
            run_dialog(error_message);
        }
    }
}

std::string Board::choose_file(Gtk::FileChooser::Action action) const
{
    Gtk::FileChooserDialog file_chooser(parent_window(
        const_cast<Board&>(*this)), "Thinkora", action);
    const char* select_button;
    switch (action)
    {
    case Gtk::FileChooser::Action::OPEN:
        select_button = "Open";
        break;
    case Gtk::FileChooser::Action::SAVE:
        select_button = "Save";
        break;
    default:
        select_button = "Select";
        break;
    }
    file_chooser.add_button(select_button, Gtk::ResponseType::OK);
    file_chooser.add_button("Cancel", Gtk::ResponseType::CANCEL);
    auto file_filter = Gtk::FileFilter::create();
    file_filter->set_name("Thinkora files");
    file_filter->add_pattern("*.thinkora");
    file_chooser.add_filter(file_filter);
    if (run_dialog(file_chooser) == Gtk::ResponseType::OK)
    {
        auto file = file_chooser.get_file();
        if (file)
        {
            return file->get_path();
        }
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

void Board::finish_ocr()
{
    set_cursor("wait");
    ocr_.finish();
    set_cursor();
}
