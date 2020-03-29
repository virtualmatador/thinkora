#include <cmath>

#include "board.h"

Board::Board()
    : zoom_{0}
    , x_{0}
    , y_{0}
{
}

Board::~Board()
{
    for (auto& [zoom, plane]: refrences_)
    {
        for (auto& [position, shapes]: plane)
        {
            while (!shapes.empty())
            {
                auto shape = *shapes.begin();
                remove_shape(shape);
                delete shape;
            }
        }
    }
}

void Board::add_shape(Shape* shape)
{
    for (int x = get_region(shape->x1_); x <= get_region(shape->x2_); ++x)
    {
        for (int y = get_region(shape->y1_); y <= get_region(shape->y2_); ++y)
        {
            refrences_[shape->zoom_][{x, y}].insert(shape);
        }
    }
}

void Board::remove_shape(Shape* shape)
{
    for (int x = get_region(shape->x1_); x <= get_region(shape->x2_); ++x)
    {
        for (int y = get_region(shape->y1_); y <= get_region(shape->y2_); ++y)
        {
            refrences_[shape->zoom_][{x, y}].erase(shape);
        }
    }
}

bool Board::on_draw(const Cairo::RefPtr<Cairo::Context>& cr)
{
    cr->set_source_rgb(1.0, 0.0, 0.0);
    cr->paint();
    return true;
}

int Board::get_region(const int position)
{
    return position / 512 - (position % 512 < 0 ? 1 : 0);
}
