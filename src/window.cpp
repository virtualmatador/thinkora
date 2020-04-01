#include "window.h"

Window::Window()
    : bar_{&board_}
    , board_{&bar_}
{
    fullscreen();
    add(box_);
    box_.set_orientation(Gtk::Orientation::ORIENTATION_VERTICAL);
    box_.pack_start(bar_, Gtk::PackOptions::PACK_SHRINK);
    bar_.show();
    box_.pack_start(board_);
    board_.show();
    box_.show();
    bar_.redraw(true);
}

Window::~Window()
{
}
