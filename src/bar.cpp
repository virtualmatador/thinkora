#include <iomanip>
#include <sstream>
#include <string>

#include "board.h"

#include "bar.h"

Bar::Bar(Board* board)
    : board_{board}
{
    open_.set_label("Open");
    add(open_);
    open_.show();
    open_.signal_clicked().connect([this]()
    {
        board_->on_open();
    });

    save_.set_label("Save");
    add(save_);
    save_.show();
    save_.signal_clicked().connect([this]()
    {
        board_->on_save();
    });

    zero_.set_label("Origin");
    add(zero_);
    zero_.show();
    zero_.signal_clicked().connect([this]()
    {
        board_->on_pad_origin();
    });

    zoom_.override_font(Pango::FontDescription("monospace"));
    add(zoom_);
    zoom_.show();

    position_.override_font(Pango::FontDescription("monospace"));
    add(position_);
    position_.show();

    add(path_);
    path_.show();
}

Bar::~Bar()
{
}

void Bar::redraw(bool pass_on)
{
    if (pass_on)
    {
        board_->redraw(false);
    }
    set_zoom();
    set_position();
    set_path();
}

void Bar::set_zoom()
{
    std::ostringstream formatter;
    formatter << std::left << "Z: " << std::setw(3) << board_->zoom_;
    zoom_.set_label(formatter.str());
}

void Bar::set_position()
{
    std::ostringstream formatter;
    formatter << std::left << "X: " <<
        std::setw(11) << board_->mouse_position_[0] << " Y: " <<
        std::setw(11) << board_->mouse_position_[1];
    position_.set_label(formatter.str());
}

void Bar::set_path()
{
    std::ostringstream formatter;
    formatter << "Path: ";
    path_.set_label(formatter.str());
}
