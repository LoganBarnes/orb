#pragma once

#include <glm/glm.hpp>
#include <vector>

namespace sim
{

class Orb
{
public:
    explicit Orb(std::size_t size);

    void update(double curr_time, double time_step);

    const std::vector<float> &get_fft_vals() const;
private:
    std::vector<float> fft_vals_; // normalized heights
};

} // namespace sim

