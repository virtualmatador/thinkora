#ifndef BAR_H
#define BAR_H

#include <gtkmm.h>

#include "shape.h"

class Board;

class Bar: public Gtk::ActionBar
{
public:
    Bar(Board* board);
    ~Bar();
    void redraw(bool pass_on);

private:
    void add_open();
    void add_save();
    void add_origin();
    void add_color();
    void add_line();
    void add_zoom();
    void add_position();
    void add_path();
    void draw_line(const Cairo::RefPtr<Cairo::Context>& cr,
        const int& width, const Shape::Style& style,
        const Gdk::RGBA& color);
    void set_zoom();
    void set_position();
    void set_path();

private:
    Gtk::Button open_;
    Gtk::Button save_;
    Gtk::Button origin_;
    std::pair<Gtk::Button, Gtk::DrawingArea> color_;
    std::pair<Gtk::Button, Gtk::DrawingArea> line_;
    Gtk::Label zoom_;
    Gtk::Label position_;
    Gtk::Label path_;
    Gdk::RGBA marker_color_;
    int marker_width_;
    Shape::Style marker_style_;
    Board* board_;

private:
    friend class Board;
};

#endif // BAR_H
