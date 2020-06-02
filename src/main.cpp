#include <iostream>

#include <gtkmm.h>

#include "window.h"

int main(int argc, char* argv[])
{
    auto app = Gtk::Application::create("com.shaidin.thinkora");
    Gdk::Rectangle monitor_rect;
    Gdk::Screen::get_default()->get_monitor_geometry(0, monitor_rect);
    Window window;
    window.move(monitor_rect.get_x(), monitor_rect.get_y());
    window.signal_delete_event().connect(
        sigc::mem_fun(&window, &Window::on_delete));
    return app->run(window);
}
