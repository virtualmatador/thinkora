#include "board.h"

Board::Board()
{
}

Board::~Board()
{
}

bool Board::on_draw(const Cairo::RefPtr<Cairo::Context>& cr)
{
    cr->set_source_rgb(1.0, 0.0, 0.0);
    cr->paint();
    return true;
}
