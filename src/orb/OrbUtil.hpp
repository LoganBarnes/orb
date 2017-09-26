#pragma once

#include <limits>

namespace vmp
{

inline float safe_float_cast(double d, double eps = std::numeric_limits<float>::epsilon())
{
    return (d < eps ? 0.0f : static_cast<float>(d));
}

} // namespace vmp
