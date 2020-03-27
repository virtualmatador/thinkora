#include <gtkmm.h>

#include "board.h"
#include "bar.h"

class Window: public Gtk::Window
{
private:
    Gtk::Box box_;
    Bar bar_;
    Board board_;

public:
    Window();
    ~Window();
};