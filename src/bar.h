#ifndef BAR_H
#define BAR_H

#include <gtkmm.h>

class Board;

class Bar: public Gtk::ActionBar
{
public:
    Bar(Board* board);
    ~Bar();
    void redraw(bool pass_on);

private:
    void set_zoom();
    void set_position();
    void set_path();

private:
    Gtk::Button open_;
    Gtk::Button save_;
    Gtk::Button zero_;
    Gtk::Label zoom_;
    Gtk::Label position_;
    Gtk::Label path_;
    Board* board_;

private:
    friend class Board;
};

#endif // BAR_H
