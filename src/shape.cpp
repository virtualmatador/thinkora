#include <exception>
#include <limits>

#include "board.h"
#include "toolbox.h"

#include "shape.h"

Shape::Shape()
    : thickness_{1}
    , color_{Gdk::RGBA("#000000")}
    , style_{Style::SOLID}
{
    set_frame();
}

Shape::Shape(std::vector<std::array<int, 2>>&& points, const int& thickness,
    const Gdk::RGBA& color, const Style& style)
    : points_{std::move(points)}
    , thickness_{thickness}
    , color_{color}
    , style_{style}
{
    set_frame();
}

Shape::~Shape()
{
}

void Shape::set_label(const std::string& label)
{
    label_ = label;
}

void Shape::add_point(const std::array<int, 2>& point)
{
    points_.emplace_back(point);
}

void Shape::finalize()
{
    set_frame();
    Cairo::ImageSurface bitmap();
    /*
    bitmap.

    Pix *image = pixRead("/usr/src/tesseract/testing/phototest.tif");
    Board::ocr_.SetImage(image);
    auto outText = Board::ocr_.GetUTF8Text();
    label_ = outText;
    delete[] outText;
    pixDestroy(&image);
    */
}

const std::array<std::array<int, 2>, 2>& Shape::get_frame() const
{
    return frame_;
}

void Shape::draw(const Cairo::RefPtr<Cairo::Context>& cr,
    const int& zoom_delta, const std::array<int, 2>& pad) const
{
    cr->set_source_rgba(color_.get_red(), color_.get_green(),
        color_.get_blue(), color_.get_alpha());
    cr->set_line_width(thickness_);
    cr->set_dash(Board::dashes_[thickness_ - 1][int(style_)], 0.0);
    draw_points(cr, transform(zoom_delta, pad));
    cr->stroke();
}

void Shape::set_frame()
{
    frame_[0] = {std::numeric_limits<int>::max(),
        std::numeric_limits<int>::max()};
    frame_[1] = {std::numeric_limits<int>::min(),
        std::numeric_limits<int>::min()};;
    for (const auto& point: points_)
    {
        frame_[0][0] = std::min(frame_[0][0], point[0]);
        frame_[0][1] = std::min(frame_[0][1], point[1]);
        frame_[1][0] = std::max(frame_[1][0], point[0]);
        frame_[1][1] = std::max(frame_[1][1], point[1]);
    }
}

std::vector<std::array<int, 2>> Shape::transform(const int& zoom_delta,
    const std::array<int, 2>& pad) const
{
    std::vector<std::array<int, 2>> transformed(points_.size());
    for (int i = 0; i < transformed.size(); ++i)
    {
        transformed[i] = zoom(points_[i], zoom_delta);
        transformed[i][0] -= pad[0];
        transformed[i][1] -= pad[1];
    }
    return transformed;
}

void Shape::write(std::ostream& os) const
{
    os << label_.size() << std::endl;
    os.write(label_.data(), label_.size());
    os << std::endl;
    os << points_.size() << std::endl;
    for (const auto& point: points_)
    {
        os << point[0] << ' ' << point[1] << std::endl;
    }
    os << thickness_ << std::endl;
    os <<
        color_.get_red() << ' ' <<
        color_.get_green() << ' ' <<
        color_.get_blue() << ' ' <<
        color_.get_alpha() << std::endl;
    os << int(style_) << std::endl;
}

void Shape::read(std::istream& is)
{
    std::size_t size;
    is >> size;
    label_.resize(size);
    is.read(label_.data(), label_.size());
    is >> size;
    for (std::size_t i = 0; i < size; ++i)
    {
        int x, y;
        is >> x >> y;
        points_.push_back({x, y});
    }
    set_frame();
    is >> thickness_ ;
    double color;
    is >> color;
    color_.set_red(color);
    is >> color;
    color_.set_green(color);
    is >> color;
    color_.set_blue(color);
        is >> color;
    color_.set_alpha(color);
    is >> (int&)style_;
}

std::ostream& operator<<(std::ostream& os, const Shape& shape)
{
    shape.write(os);
    return os;
}

std::istream& operator>>(std::istream& is, Shape& shape)
{
    shape.read(is);
    return is;
}
