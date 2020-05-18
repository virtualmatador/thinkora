#ifndef THINKORA_SRC_WINDOW_H
#define THINKORA_SRC_WINDOW_H

#include <gtkmm.h>

#include "board.h"
#include "bar.h"

class Window: public Gtk::Window
{
public:
    Window();
    ~Window();
    bool on_delete(GdkEventAny* any_event);

private:
    Gtk::Box box_;
    Bar bar_;
    Board board_;
};

#endif // THINKORA_SRC_WINDOW_H
