#include "pad.h"

void Pad::push(std::pair<const Sketch*, int> sketch)
{
    std::lock_guard<std::mutex> lock {lock_sketchs_};
    sketchs_.emplace_back(sketch);
}

std::pair<const Shape*, int> Pad::front()
{
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
    return sketch;
}

void Pad::pop()
{
    std::lock_guard<std::mutex> lock{lock_sketchs_};
    sketchs_.pop_front();
}

void Pad::clear()
{
    std::lock_guard<std::mutex> lock{lock_sketchs_};
    sketchs_.clear();
}
