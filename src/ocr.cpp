#include "board.h"
#include "circle.h"
#include "polyline.h"
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
void Ocr::add(const int& zoom, const std::array<std::array<int, 2>, 2>& frame)
{
    std::lock_guard<std::mutex> lock{jobs_lock_};
    jobs_.emplace_back(zoom, frame);
}

bool Ocr::get_sketch()
{
    auto time = std::chrono::steady_clock::now();
    std::pair<int, std::array<std::array<int, 2UL>, 2UL>> job;
    {
        std::lock_guard<std::mutex> lock{jobs_lock_};
        if (jobs_.size() > 0)
        {
            job = jobs_.front();
        }
        else
        {
            return true;
        }
    }
    auto sketches = board_->list_sketches(job.first, job.second);
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
    else if (sketches.size() == 0 || process(job.first, job.second, sketches))
    {
        std::lock_guard<std::mutex> lock{jobs_lock_};
        jobs_.pop_front();
    }
    return false;
}

bool Ocr::process(const int& zoom, const std::array<std::array<int, 2>, 2>&
    frame, std::vector<Sketch>& sketches)
{
    // combine sketches [end-start] touch
    // process
    // modify using sticky points and grid
    // create sticky points
    // if delete sketches insert shape
    Shape* shape;
    Circle* circle = new Circle(4, Gdk::RGBA("#FF0000"), Shape::Style::DASH_DOT);
    circle->set_circle(
        {
            (frame[0][0] + frame[1][0]) / 2,
            (frame[0][1] + frame[1][1]) / 2,
        }, std::max(
            (frame[1][0] - frame[0][0]) / 2,
            (frame[1][1] - frame[0][1]) / 2)
    );
    shape = circle;
    return board_->replace_sketches(sketches, zoom, frame, shape);
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
