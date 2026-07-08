#include "window.h"

Window::Window()
    : bar_{ board_ }
    , board_{ bar_ }
{
    fullscreen();
    set_child(box_);
    box_.set_orientation(Gtk::Orientation::VERTICAL);
    box_.append(bar_);
    bar_.set_visible(true);
    board_.set_expand(true);
    box_.append(board_);
    board_.set_visible(true);
    box_.set_visible(true);
    bar_.redraw(true);
}

Window::~Window()
{
}

bool Window::on_close_request()
{
    if (board_.check_modified())
    {
        return true;
    }
    return false;
}
