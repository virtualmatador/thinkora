#include <array>
#include <iomanip>
#include <sstream>
#include <string>

#include "board.h"
#include "toolbox.h"

#include "bar.h"

Bar::Bar(Board& board)
    : board_{board}
    , marker_color_{"#FFFFFF"}
    , marker_width_{1}
    , marker_style_{Shape::Style::SOLID}
{
    add_open();
    add_save();
    add_origin();
    add_color();
    add_line();
    add_zoom();
    add_position();
    add_path();
}

Bar::~Bar()
{
}

void Bar::redraw(bool pass_on)
{
    if (pass_on)
    {
        board_.redraw(false);
    }
    set_zoom();
    set_position();
    set_path();
}

void Bar::add_open()
{
    open_.signal_clicked().connect([this]()
    {
        board_.on_open();
    });
    open_.set_label("Open");
    add(open_);
    open_.show();
}

void Bar::add_save()
{
    save_.signal_clicked().connect([this]()
    {
        board_.on_save();
    });
    save_.set_label("Save");
    add(save_);
    save_.show();
}

void Bar::add_origin()
{
    origin_.signal_clicked().connect([this]()
    {
        board_.on_origin();
    });
    origin_.set_label("Origin");
    add(origin_);
    origin_.show();
}

void Bar::add_color()
{
    color_.second.signal_draw().connect(
        [this](const Cairo::RefPtr<Cairo::Context>& cr)
    {
        auto rgba = marker_color_;
        cr->set_source_rgb(rgba.get_red(), rgba.get_green(), rgba.get_blue());
        cr->paint();
        std::array<std::array<double, 2>, 2> frame;
        cr->get_clip_extents(
            frame[0][0], frame[0][1], frame[1][0], frame[1][1]);
        cr->set_source_rgba(0.0, 0.0, 0.0, 1.0);
        cr->rectangle(frame[0][0], frame[0][1], frame[1][0], frame[1][1]);
        cr->stroke();
        return true;
    });
    color_.first.add(color_.second);
    color_.second.show();

    color_.first.signal_clicked().connect([this]()
    {
        auto dioalog = Gtk::ColorChooserDialog("Marker");
        dioalog.set_transient_for(*(Gtk::Window*)get_toplevel());
        dioalog.set_rgba(marker_color_);
        if (dioalog.run() == Gtk::ResponseType::RESPONSE_OK)
        {
            marker_color_ = dioalog.get_rgba();
            color_.second.queue_draw();
        }
    });
    color_.first.set_size_request(50);
    add(color_.first);
    color_.first.show();
}

void Bar::add_line()
{
    line_.second.signal_draw().connect(
        [this](const Cairo::RefPtr<Cairo::Context>& cr)
    {
        draw_line(cr, marker_width_, marker_style_, Gdk::RGBA("#FFFFFF"));
        return true;
    });
    line_.first.add(line_.second);
    line_.second.show();

    line_.first.signal_clicked().connect([this]()
    {
        Gtk::Dialog dialog("Line Type");
        dialog.set_resizable(false);
        dialog.set_transient_for(*(Gtk::Window*)get_toplevel());
        Gtk::Frame groups[2];
        std::string group_labels[2] =
        {
            "Width",
            "Style"
        };
        Gtk::HBox boxes[2];
        auto width = marker_width_;
        auto style = marker_style_;
        Gtk::Button actions[2];
        std::string action_labels[2] =
        {
            "Cancel",
            "Select"
        };
        Gtk::ResponseType responses[2] =
        {
            Gtk::ResponseType::RESPONSE_CANCEL,
            Gtk::ResponseType::RESPONSE_OK
        };
        for (std::size_t i = 0; i < 2; ++i)
        {
            actions[i].set_label(action_labels[i]);
            actions[i].set_margin_top(10);
            actions[i].set_margin_left(5);
            actions[i].set_margin_right(5);
            actions[i].set_margin_bottom(10);
            actions[i].show();
            dialog.add_action_widget(actions[i], responses[i]);
        }
        std::vector<std::pair<Gtk::Button, Gtk::DrawingArea>> types[2];
        for (std::size_t i = 0; i < 2; ++i)
        {
            for (std::size_t j = 0; j < (i == 0 ? width_limit_ :
                int(Shape::Style::SIZE)); ++j)
            {
                types[i].emplace_back(std::pair<Gtk::Button,
                    Gtk::DrawingArea>());
                types[i][j].second.signal_draw().connect(
                    [&, i, j](const Cairo::RefPtr<Cairo::Context>& cr)
                {
                    Gdk::RGBA color;
                    if (i == 0)
                    {
                        if (width == j + 1)
                        {
                            color = Gdk::RGBA("#BBBBBBFF");
                        }
                        else
                        {
                            color = Gdk::RGBA("#FFFFFFFF");
                        }
                    }
                    else
                    {
                        if (style == Shape::Style(j))
                        {
                            color = Gdk::RGBA("#BBBBBBFF");
                        }
                        else
                        {
                            color = Gdk::RGBA("#FFFFFFFF");
                        }
                    }                        
                    if (i == 0)
                    {
                        draw_line(cr, j + 1, style, color);
                    }
                    else
                    {
                        draw_line(cr, width, Shape::Style(j), color);
                    }
                    return true;
                });
                types[i][j].second.show();
                types[i][j].first.add(types[i][j].second);
                types[i][j].first.signal_clicked().connect([&, i, j]()
                {
                    if (i == 0)
                    {
                        auto old_width = width;
                        width = j + 1;
                        types[i][old_width - 1].second.queue_draw();
                        types[i][width - 1].second.queue_draw();
                        for (auto& type: types[1])
                        {
                            type.second.queue_draw();
                        }
                    }
                    else
                    {
                        auto old_style = style;
                        style = Shape::Style(j);
                        types[i][int(old_style)].second.queue_draw();
                        types[i][int(style)].second.queue_draw();
                        for (auto& type: types[0])
                        {
                            type.second.queue_draw();
                        }
                    }
                });
                types[i][j].first.show();
                types[i][j].first.set_margin_left(10);
                types[i][j].first.set_margin_right(10);
                types[i][j].first.set_margin_top(10);
                types[i][j].first.set_margin_bottom(10);
                types[i][j].first.set_size_request(100);
                boxes[i].add(types[i][j].first);
            }
            groups[i].add(boxes[i]);
            boxes[i].show();
            groups[i].set_label(group_labels[i]);
            groups[i].set_label_align(Gtk::Align::ALIGN_CENTER);
            groups[i].set_margin_bottom(10);
            dialog.get_vbox()->add(groups[i]);
            groups[i].show();
        }
        if (dialog.run() == Gtk::ResponseType::RESPONSE_OK)
        {
            if (marker_width_ != width || marker_style_ != style)
            {
                marker_width_ = width;
                marker_style_ = style;
                line_.second.queue_draw();
            }
        }
    });
    line_.first.set_size_request(80);
    add(line_.first);
    line_.first.show();
}

void Bar::add_zoom()
{
    zoom_.override_font(Pango::FontDescription("monospace"));
    add(zoom_);
    zoom_.show();
}

void Bar::add_position()
{
    position_.override_font(Pango::FontDescription("monospace"));
    add(position_);
    position_.show();
}

void Bar::add_path()
{
    path_.override_font(Pango::FontDescription("monospace"));
    add(path_);
    path_.show();
}

void Bar::draw_line(const Cairo::RefPtr<Cairo::Context>& cr,
    const double& width, const Shape::Style& style, const Gdk::RGBA& color)
{
    cr->set_source_rgba(color.get_red(), color.get_green(), color.get_blue(),
        color.get_alpha());
    cr->paint();
    std::array<std::array<double, 2>, 2> frame;
    cr->get_clip_extents(frame[0][0], frame[0][1], frame[1][0], frame[1][1]);
    cr->set_source_rgba(0.0, 0.0, 0.0, 1.0);
    cr->rectangle(frame[0][0], frame[0][1], frame[1][0], frame[1][1]);
    cr->stroke();
    cr->set_line_width(width);
    cr->set_dash(Board::dashes_[width - 1][int(style)], 0.0);
    cr->move_to(frame[0][0] + 5.0, (frame[0][1] + frame[1][1]) / 2.0);
    cr->line_to(frame[1][0] - 5.0, (frame[0][1] + frame[1][1]) / 2.0);
    cr->stroke();
}

void Bar::set_zoom()
{
    std::ostringstream formatter;
    formatter << std::left << "Z: " << std::setw(3) << board_.zoom_;
    zoom_.set_label(formatter.str());
}

void Bar::set_position()
{
    std::ostringstream formatter;
    formatter << std::left << "X: " <<
        std::setw(11) << board_.mouse_position_[0] << " Y: " <<
        std::setw(11) << board_.mouse_position_[1];
    position_.set_label(formatter.str());
}

void Bar::set_path()
{
    std::ostringstream formatter;
    formatter << "Path: ";
    path_.set_label(formatter.str());
}
