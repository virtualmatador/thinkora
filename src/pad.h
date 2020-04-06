#ifndef PAD_H
#define PAD_H

#include <list>
#include <mutex>

#include "sketch.h"

class Pad
{
public:
    void push(std::pair<const Sketch*, int> sketch);
    std::pair<const Shape*, int> front();
    void pop();
    void clear();

private:
    std::list<std::pair<const Sketch*, int>> sketchs_;
    std::mutex lock_sketchs_;

private:
    friend class Board;
};

#endif // PAD_H
