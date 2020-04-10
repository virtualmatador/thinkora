#include "board.h"
#include "toolbox.h"

#include "ocr.h"

Ocr::Ocr(Board* board)
    : run_{true}
    , reset_{false}
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
            auto recent = get_sketch();
            if (recent != std::chrono::steady_clock::time_point::min())
            {
                auto passed = std::chrono::steady_clock::now() - recent;
                if (passed > std::chrono::operator""s(1))
                {
                    if (sketches_.size() > 0)
                    {
                        process();
                    }
                }
                else
                {
                    std::this_thread::sleep_for(std::chrono::operator""s(1) - passed);
                }
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

void Ocr::clear()
{
    std::lock_guard<std::mutex> lock{jobs_lock_};
    jobs_.clear();
    reset_ = true;
}

std::chrono::steady_clock::time_point Ocr::get_sketch()
{
    std::pair<int, std::array<std::array<int, 2UL>, 2UL>> job;
    bool job_found;
    sketches_.clear();
    {
        std::lock_guard<std::mutex> lock{jobs_lock_};
        if (jobs_.size() > 0)
        {
            job = jobs_.front();
            jobs_.pop_front();
            job_found = true;
        }
        else
        {
            job_found = false;
        }
    }
    std::chrono::steady_clock::time_point recent;
    if (job_found)
    {
        recent = std::chrono::steady_clock::time_point::min();
        sketches_ = board_->list_sketches(job.first, job.second);
    }
    else
    {
        recent = std::chrono::steady_clock::now();
    }
    return recent;
}

void Ocr::process()
{
    // combine sketches [end-start] touch
    // process
    // modify using sticky points and grid
    // create sticky points
    {
        std::lock_guard<std::mutex> lock{jobs_lock_};
        if (reset_)
        {
            reset_ = false;
            // throw results away
        }
    }
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
