#ifndef THINKORA_SRC_BOARD_H
#define THINKORA_SRC_BOARD_H

#include <array>
#include <atomic>
#include <cstddef>
#include <map>
#include <mutex>
#include <list>
#include <set>
#include <stack>

#include <gtkmm.h>

#include "ocr.h"
#include "sketch.h"
#include "shape.h"

class Bar;

class Board: public Gtk::DrawingArea
{
public:
    Board(Bar& bar);
    ~Board();
    bool check_modified();
    void redraw(bool pass_on);
    bool is_drawing();
    void apply_ocr(const std::list<const Sketch*>& sources, int zoom,
        const std::list<Shape*>& results);

private:
    bool on_draw(const Cairo::RefPtr<Cairo::Context>& cr) override;
    bool on_button_press_event(GdkEventButton* button_event) override;
    bool on_motion_notify_event(GdkEventMotion* motion_event) override;
    bool on_button_release_event(GdkEventButton* release_event) override;
    bool on_scroll_event(GdkEventScroll *scroll_event) override;
    bool on_enter_notify_event(GdkEventCrossing* crossing_event) override;
    void on_save();
    void on_open();
    void on_origin();

private:
    void clear_data();
    void add_reference(const int& zoom, const Shape* shape);
    void remove_reference(const int& zoom, const Shape* shape);
    void clamp_position();
    bool check_zoom(const int& zoom, const Point& center) const;
    std::string choose_file(Gtk::FileChooserAction action) const;
    Point get_input_position(const Point& point) const;
    void finish_ocr();

private:
    int zoom_;
    Point center_;
    mutable bool modified_;
    Sketch* sketch_;
    std::map<int,
        std::map<std::pair<int, int>, std::set<const Shape*>>> shapes_;
    mutable std::mutex shapes_lock_;
    Point center_pre_pad_;
    Point mouse_position_;
    Point mouse_pre_pad_;
    std::stack<Point> zoom_lag_;
    std::atomic<int> mouse_button_;
    Ocr ocr_;
    Glib::Dispatcher queue_draw_;
    Bar& bar_;

public:
    static std::vector<std::vector<std::vector<double>>> dashes_;

private:
    friend class Bar;
};

#endif // THINKORA_SRC_BOARD_H
