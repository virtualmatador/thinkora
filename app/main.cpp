#include <gtkmm.h>

#include "window.h"

int main(int argc, char* argv[])
{
    auto app = Gtk::Application::create("com.shaidin.thinkora");
    return app->make_window_and_run<Window>(argc, argv);
}
