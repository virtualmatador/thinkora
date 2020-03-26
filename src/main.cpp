#include <iostream>

#include <gtkmm.h>

int main(int argc, char* argv[])
{
    auto app = Gtk::Application::create("com.shaidin.thinkora");
    Gtk::Window window;
    window.set_title("Thinkora");
    window.set_default_size(200, 200);
    return app->run(window);
}