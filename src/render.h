#ifndef THINKORA_SRC_RENDER_H
#define THINKORA_SRC_RENDER_H

#include <string>

#include "renders.h"
#include "shape.h"

class Render: public Shape
{
public:
    using Shape::Shape;
    void set_render(const std::string& name, const Point& center,
        double cx, double cy, double rotation);
    const std::string& get_name() const;
    const Point& get_center() const;
    double get_cx() const;
    double get_cy() const;
    double get_rotation() const;

public:
    Type get_type() const override;

private:
    Point render_point(const Point& point) const;
    Point draw_point(const Point& point, const int& zoom_delta,
        const Point& pad) const;
    void update_frame();
    void append_render_transform(const Cairo::RefPtr<Cairo::Context>& cr,
        const int& zoom_delta, const Point& pad) const;
    const RenderSource* get_render() const;
    void draw_details(const Cairo::RefPtr<Cairo::Context>& cr,
        const int& zoom_delta, const Point& pad) const override;
    void write_dtails(std::ostream& os) const override;
    void read_details(std::istream& is) override;

private:
    std::string name_;
    Point center_ { 0.0, 0.0 };
    double cx_ = 0.0;
    double cy_ = 0.0;
    double rotation_ = 0.0;
};

#endif // THINKORA_SRC_RENDER_H
