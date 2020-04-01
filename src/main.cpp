#include <iostream>

#include <gtkmm.h>

#include "window.h"

int main(int argc, char* argv[])
{
    auto app = Gtk::Application::create("com.shaidin.thinkora");
    Window window;
    window.signal_delete_event().connect(
        sigc::mem_fun(&window, &Window::on_delete));
    return app->run(window);
}
