#include <gtkmm.h>

class Board: public Gtk::DrawingArea
{
public:
    Board();
    ~Board();

private:
    bool on_draw(const Cairo::RefPtr<Cairo::Context>& cr) override;
};