#include "board.h"
#include "circle.h"
#include "line.h"
#include "toolbox.h"

#include "ocr.h"

Ocr::Ocr(Board* board)
    : run_{true}
    , board_{board}
{
    /*
    if (ocr_.Init(nullptr, "eng"))
    {
        throw std::runtime_error("ocr init");
    }
    */
    thread_ = std::thread([this]()
    {
        while (run_)
        {
            if (get_sketch())
            {
                std::this_thread::sleep_for(std::chrono::operator""ms(100));
            }
        }
    });
}

Ocr::~Ocr()
{
    run_ = false;
    thread_.join();
    //ocr_.End();
}
void Ocr::add(const Job& job)
{
    std::lock_guard<std::mutex> lock{jobs_lock_};
    jobs_.emplace_back(job);
}

bool Ocr::get_sketch()
{
    Job* job;
    {
        std::lock_guard<std::mutex> lock{jobs_lock_};
        if (jobs_.size() > 0)
        {
            job = &jobs_.front();
        }
        else
        {
            return true;
        }
    }
    auto time = std::chrono::steady_clock::now();
    auto sketches = board_->list_sketches(job);
    std::chrono::steady_clock::time_point recent =
        std::chrono::steady_clock::time_point::min();
    for (const auto& sketch: sketches)
    {
        if (recent < sketch.get_birth())
        {
            recent = sketch.get_birth();
        }
    }
    if (recent != std::chrono::steady_clock::time_point::min() &&
        (recent == std::chrono::steady_clock::time_point::max() ||
        time - recent < std::chrono::operator""s(1)))
    {
        return true;
    }
    else if (sketches.size() == 0 || process(job, sketches))
    {
        std::lock_guard<std::mutex> lock{jobs_lock_};
        jobs_.pop_front();
    }
    return false;
}

bool Ocr::process(const Job* job, std::vector<Sketch>& sketches)
{
    std::vector<Shape*> elements;
    for (const auto& sketch: sketches)
    {
        elements.emplace_back(simplify(sketch));
    }
    std::vector<Shape*> shapes = combine(elements);
    // modify using sticky points
    // create sticky points
    return board_->replace_sketches(job, sketches, shapes);
}

Shape* Ocr::simplify(const Sketch& sketch)
{
    Shape* shape;
    Circle* circle = new Circle(sketch.get_line_width(), sketch.get_color(), sketch.get_style());
    circle->set_circle(
        {
            (sketch.get_frame()[0][0] + sketch.get_frame()[1][0]) / 2,
            (sketch.get_frame()[0][1] + sketch.get_frame()[1][1]) / 2,
        }, std::max(
            (sketch.get_frame()[1][0] - sketch.get_frame()[0][0]) / 2,
            (sketch.get_frame()[1][1] - sketch.get_frame()[0][1]) / 2)
    );
    shape = circle;
    return shape;
}

std::vector<Shape*> Ocr::combine(std::vector<Shape*>& elements)
{
    std::vector<Shape*> shapes;
    for (const auto& element: elements)
    {
        shapes.emplace_back(element);
    }
    return shapes;
}

/*
void Board::process()
{
    auto bitmap = Cairo::ImageSurface::create(Cairo::Format::FORMAT_A8,
        frame_[1][0] - frame_[0][0] + 10, frame_[1][1] - frame_[0][1] + 10);
    auto cr = Cairo::Context::create(bitmap);
    cr->set_source_rgba(1.0, 1.0, 1.0, 1.0);
    draw_points(cr, transform(0, {frame_[0][0] + 5, frame_[0][1] + 5}));
    cr->stroke();
    Board::ocr_.SetImage(bitmap->get_data(), bitmap->get_width(),
        bitmap->get_height(), 1, bitmap->get_width());
    auto outText = Board::ocr_.GetUTF8Text();
    label_ = "TXT: ";
    label_ += outText;
    delete[] outText;
}
*/
