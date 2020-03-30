#ifndef BOARD_H
#define BOARD_H

#include <array>
#include <map>
#include <set>
#include <stack>

#include <gtkmm.h>

#include "shape.h"

class Board: public Gtk::DrawingArea
{
public:
    Board();
    ~Board();

protected:
    bool on_draw(const Cairo::RefPtr<Cairo::Context>& cr) override;
    bool on_button_press_event(GdkEventButton* button_event) override;
    bool on_motion_notify_event(GdkEventMotion* motion_event) override;
    bool on_button_release_event(GdkEventButton* release_event) override;
    bool on_scroll_event(GdkEventScroll *scroll_event) override;

private:
    void add_reference(Shape* shape);
    void remove_reference(Shape* shape);
    void on_change_view();
    void on_change_position();
    bool on_change_zoom(const int& zoom, const std::array<int, 2>& center);
    std::array<int, 2> get_input_position(const int& x, const int& y) const;
    void regionize(std::array<std::array<int, 2>, 2>& frame);

private:
    int zoom_;
    std::array<int, 2> center_;
    std::map<int, 
        std::map<std::pair<int, int>,
        std::set<Shape*>>> references_;
    std::array<int, 2> mouse_center_;
    std::array<double, 2> mouse_position_;
    std::stack<std::array<int, 2>> zoom_lag_;
    int mouse_button_;

private:
    static const int tile_size_ = 512;
    static const int zoom_limit_ = 24;
    static const int position_limit_ = 1000000000;
    static const int draw_level_limit_ = 5;
};

#endif // BOARD_H
