#ifndef THINKORA_SRC_FIT_H
#define THINKORA_SRC_FIT_H

class Result;
class Sketch;

class Fit
{
public:
    Fit(const Result& result, const Sketch& sketch, double similarity);
    ~Fit();

private:
    double similarity_;
};

#endif // THINKORA_SRC_FIT_H
