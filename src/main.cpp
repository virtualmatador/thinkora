#include <iostream>

#include <gtkmm.h>

#include "window.h"

int main(int argc, char* argv[])
{
    auto app = Gtk::Application::create("com.shaidin.thinkora");
    Window window;
    return app->run(window);
}
