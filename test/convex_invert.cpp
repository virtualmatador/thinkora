#include <cstdlib>
#include <iostream>
#include <vector>

#include "convex.h"

int main()
{
    std::vector<Point> forward
    {
        { 0.0, 0.0 },
        { 1.0, 0.0 },
    };
    std::vector<Point> reversed
    {
        { 1.0, 0.0 },
        { 0.0, 0.0 },
    };

    Convex expected{ forward };
    Convex observed{ reversed };
    observed.invert();

    auto match = expected.compare_best_rotation(observed);
    if (match.diff > 1e-9)
    {
        std::cerr << "Expected inverted convex to match without chord penalty. "
                  << "diff=" << match.diff << '\n';
        std::cerr.flush();
        std::_Exit(1);
    }
}
