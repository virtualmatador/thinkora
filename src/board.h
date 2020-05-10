#ifndef BOARD_H
#define BOARD_H

#include <array>
#include <cstddef>
#include <map>
#include <mutex>
#include <list>
#include <set>
#include <stack>

#include <gtkmm.h>

#include "ocr.h"
#include "sketch.h"

class Bar;

class Board: public Gtk::DrawingArea
{
public:
    using Map = std::map<int, std::map<std::pair<int, int>, std::set<Shape*>>>;

public:
    Board(Bar* bar);
    ~Board();
    bool check_modified();
    void redraw(bool pass_on);
    std::vector<Sketch> list_sketches(const Job* job,
        std::array<std::array<int, 2>, 2>& frame) const;
    bool replace_sketches(const Job* job, const std::vector<Sketch>& sketches,
        const std::vector<Shape*>& shapes);

private:
    bool on_draw(const Cairo::RefPtr<Cairo::Context>& cr) override;
    bool on_button_press_event(GdkEventButton* button_event) override;
    bool on_motion_notify_event(GdkEventMotion* motion_event) override;
    bool on_button_release_event(GdkEventButton* release_event) override;
    bool on_scroll_event(GdkEventScroll *scroll_event) override;
    bool on_enter_notify_event(GdkEventCrossing* crossing_event) override;
    void on_save() const;
    void on_open();
    void on_pad_origin();

private:
    void clear_data();
    void clear_map(Map& map);
    void push_sketches(const Job* job,
        std::function<void(Sketch&)> pusher) const;
    void draw_layers(const Cairo::RefPtr<Cairo::Context>& cr,
        const Map& map, const std::array<std::array<int, 2>, 2>& area) const;
    std::set<Shape*> list_shapes(const Map& map, const int& zoom,
        const std::array<std::array<int, 2>, 2>& view) const;
    void add_reference(Map& map, const int& zoom, Shape* shape);
    void remove_reference(Map& map, const int& zoom, Shape* shape);
    void save_map(std::ostream& os, const Map& map, const bool& sketch) const;
    void open_map(std::istream& is, Map& map, const bool& sketch);
    void clamp_position();
    bool check_zoom(const int& zoom, const std::array<int, 2>& center);
    std::string choose_file(Gtk::FileChooserAction action) const;
    std::array<int, 2> get_input_position(const int& x, const int& y) const;

private:
    int zoom_;
    std::array<int, 2> center_;
    mutable bool modified_;
    mutable bool cleared_;
    Sketch* sketch_;
    Map shapes_;
    mutable std::mutex shapes_lock_;
    Map sketches_;
    mutable std::mutex sketches_lock_;
    std::array<int, 2> center_pre_pad_;
    std::array<int, 2> mouse_position_;
    std::array<int, 2> mouse_pre_pad_;
    std::stack<std::array<int, 2>> zoom_lag_;
    int mouse_button_;
    Ocr ocr_;
    Bar* bar_;

public:
    static std::vector<std::vector<std::vector<double>>> dashes_;

private:
    friend class Bar;
};

#endif // BOARD_H
