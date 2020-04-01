#ifndef BOARD_H
#define BOARD_H

#include <array>
#include <map>
#include <set>
#include <stack>

#include <gtkmm.h>

#include "shape.h"

class Bar;

class Board: public Gtk::DrawingArea
{
public:
    Board(Bar* bar);
    ~Board();
    bool check_modified();
    void redraw(bool pass_on);

private:
    bool on_draw(const Cairo::RefPtr<Cairo::Context>& cr) override;
    bool on_button_press_event(GdkEventButton* button_event) override;
    bool on_motion_notify_event(GdkEventMotion* motion_event) override;
    bool on_button_release_event(GdkEventButton* release_event) override;
    bool on_scroll_event(GdkEventScroll *scroll_event) override;
    void on_save() const;
    void on_open();
    void on_pad_origin();

private:
    void clear_data();
    void add_reference(Shape* shape, const int& zoom);
    void remove_reference(Shape* shape, const int& zoom);
    void clamp_position();
    bool check_zoom(const int& zoom, const std::array<int, 2>& center);
    std::string choose_file(Gtk::FileChooserAction action) const;
    std::array<int, 2> get_input_position(const int& x, const int& y) const;
    void regionize(std::array<std::array<int, 2>, 2>& frame);

private:
    int zoom_;
    std::array<int, 2> center_;
    std::map<int, 
        std::map<std::pair<int, int>,
        std::set<Shape*>>> references_;
    mutable bool modified_;
    std::array<int, 2> center_pre_pad_;
    std::array<int, 2> mouse_position_;
    std::array<int, 2> mouse_pre_pad_;
    std::stack<std::array<int, 2>> zoom_lag_;
    int mouse_button_;
    Bar* bar_;

private:
    static const int tile_size_ = 512;
    static const int zoom_limit_ = 64;
    static const int position_limit_ = 1000000000;
    static const int draw_level_limit_ = 5;

private:
    friend class Bar;
};

#endif // BOARD_H
