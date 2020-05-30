#ifndef THINKORA_SRC_RESULT_H
#define THINKORA_SRC_RESULT_H

#include <string>

#include <json.h>

class Result
{
public:
    Result(const jsonio::json& json);
    ~Result();

private:
    double top_;
    double bottom_;
    std::string combine_;
    std::string direction_;
    std::string character_;

public:
    friend class Pattern;
};
    
#endif // THINKORA_SRC_RESULT_H
