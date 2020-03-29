#ifndef BOARD_H
#define BOARD_H

#include <map>
#include <set>

#include <gtkmm.h>

#include "shape.h"

class Board: public Gtk::DrawingArea
{
public:
    Board();
    ~Board();
    void add_shape(Shape* shape);
    void remove_shape(Shape* shape);

protected:
    bool on_draw(const Cairo::RefPtr<Cairo::Context>& cr) override;

private:
    int get_region(const int position);

private:
    int zoom_;
    int x_;
    int y_;
    std::map<int, 
        std::map<std::pair<int, int>,
        std::set<Shape*>>> refrences_;
};

#endif // BOARD_H
