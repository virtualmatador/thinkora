#include "window.h"

Window::Window()
{
    fullscreen();
    add(box_);
    box_.set_orientation(Gtk::Orientation::ORIENTATION_VERTICAL);
    box_.pack_start(bar_, Gtk::PackOptions::PACK_SHRINK);
    bar_.show();
    box_.pack_start(board_);
    board_.show();
    box_.show();
}

Window::~Window()
{
}
