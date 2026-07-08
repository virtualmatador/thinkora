#include <cstdint>
#include <cmath>
#include <cstring>
#include <iostream>
#include <numbers>

#include <gtkmm.h>

#define private public
#include "text.h"
#undef private

namespace
{
struct Bounds
{
    int top;
    int bottom;
};

Bounds ink_bounds(const Cairo::RefPtr<Cairo::ImageSurface>& surface)
{
    surface->flush();

    const unsigned char* data = surface->get_data();
    const int stride = surface->get_stride();
    Bounds bounds{ surface->get_height(), -1 };

    for (int y = 0; y < surface->get_height(); ++y)
    {
        for (int x = 0; x < surface->get_width(); ++x)
        {
            std::uint32_t pixel;
            std::memcpy(&pixel, data + y * stride + x * 4, sizeof(pixel));
            const std::uint8_t alpha = (pixel >> 24) & 0xff;
            if (alpha > 8)
            {
                if (bounds.top > y)
                {
                    bounds.top = y;
                }
                if (bounds.bottom < y)
                {
                    bounds.bottom = y;
                }
            }
        }
    }

    return bounds;
}
}

int main()
{
    constexpr int frame_top = 20;
    constexpr int frame_bottom = 120;

    auto surface = Cairo::ImageSurface::create(
        Cairo::Surface::Format::ARGB32, 220, 160);
    auto cr = Cairo::Context::create(surface);
    cr->set_source_rgba(1.0, 1.0, 1.0, 1.0);

    Text text;
    text.set_text("H", {{{40.0, frame_top}, {160.0, frame_bottom}}});
    text.draw_details(cr, 0, {0.0, 0.0});

    const auto bounds = ink_bounds(surface);
    if (bounds.bottom < bounds.top)
    {
        std::cerr << "Expected text to render ink.\n";
        return 1;
    }

    const int ink_height = bounds.bottom - bounds.top + 1;
    if (bounds.top > frame_top + 8 || bounds.bottom < frame_bottom - 8 ||
        ink_height < 85)
    {
        std::cerr << "Expected text ink to fill the frame height. bounds=("
                  << bounds.top << ", " << bounds.bottom
                  << ") height=" << ink_height << '\n';
        return 1;
    }

    constexpr double rotation = std::numbers::pi / 4.0;
    Text rotated;
    Rectangle rotated_frame {{{100.0, 20.0}, {220.0, 120.0}}};
    rotated.set_text("H", rotated_frame, rotation);

    if (std::abs(rotated.get_rotation() - rotation) > 1e-9 ||
        rotated.get_text_frame() != rotated_frame)
    {
        std::cerr << "Expected text to store rotated frame state.\n";
        return 1;
    }

    const auto& world_frame = rotated.get_frame();
    const double world_width = world_frame[1][0] - world_frame[0][0];
    const double world_height = world_frame[1][1] - world_frame[0][1];
    if (world_width <= rotated_frame[1][0] - rotated_frame[0][0] ||
        world_height <= rotated_frame[1][1] - rotated_frame[0][1])
    {
        std::cerr << "Expected rotated text frame to update world bounds.\n";
        return 1;
    }

    return 0;
}
