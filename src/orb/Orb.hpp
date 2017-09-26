#pragma once

#include <glm/glm.hpp>
#include <vector>

namespace vmp
{

class Orb
{
public:
    explicit Orb(std::size_t size);

    void update(double curr_time, double scale);

    const std::vector<float> &get_fft_vals() const;
private:
    std::vector<float> fft_vals_; // normalized heights
};

} // namespace vmp

