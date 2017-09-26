#include <orb/Orb.hpp>
#include <glm/gtc/constants.hpp>
#include <orb/OrbUtil.hpp>

namespace vmp
{

Orb::Orb(std::size_t size)
    : fft_vals_(size, 0)
{}

void Orb::update(double curr_time, double scale)
{
    for (unsigned i = 0; i < fft_vals_.size(); ++i) {
        double val = (i * -glm::pi<double>() * scale) / fft_vals_.size();
        val = glm::sin(val + curr_time * 5.0);
        val = val * 0.5 + 0.5; // normalize
        fft_vals_[i] = safe_float_cast(val);
    }
}

const std::vector<float> &Orb::get_fft_vals() const
{
    return fft_vals_;
}
} // namespace vmp

