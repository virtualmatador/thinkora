#include "ocr.h"

Ocr::Ocr()
    : run_{true}
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
                process();
            }
            else
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

void Ocr::push(std::pair<const Sketch*, int> sketch)
{
    std::lock_guard<std::mutex> lock {lock_sketchs_};
    sketchs_.emplace_back(sketch);
}

bool Ocr::get_sketch()
{
    /*
    std::lock_guard<std::mutex> lock{lock_sketchs_};
    std::pair<const Shape*, int> sketch;
    if (sketchs_.empty())
    {
        sketch = {nullptr, 0};
    }
    else
    {
        sketch = sketchs_.front();
        sketchs_.pop_front();
    }



    std::lock_guard<std::mutex> lock{lock_sketchs_};
    sketchs_.pop_front();

*/

    return false;
}

void Ocr::process()
{

}

void Ocr::clear()
{
    std::lock_guard<std::mutex> lock{lock_sketchs_};
    sketchs_.clear();
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
