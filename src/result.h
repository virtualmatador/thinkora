#ifndef THINKORA_SRC_RESULT_H
#define THINKORA_SRC_RESULT_H

#include <string>
#include <vector>

#include <json.h>

class Result
{
public:
    Result(const jsonio::json& json);
    ~Result();
    std::size_t get_size() const;

private:
    std::string title_;
    std::vector<std::string> patterns_;
    double top_;
    double bottom_;
};
    
#endif // THINKORA_SRC_RESULT_H
