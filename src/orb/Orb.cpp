#include <orb/Orb.hpp>
#include <glm/gtc/constants.hpp>

namespace sim
{

namespace {
float safe_cast_d_to_f(double d, double eps = 1.0e-4) {
    return (d < eps ? 0.0f : static_cast<float>(d));
}
}

Orb::Orb(std::size_t size) : fft_vals_(size) {}

void Orb::update(double curr_time, double)
{
    for (unsigned i = 0; i < fft_vals_.size(); ++i) {
        double val = (i * glm::pi<double>() * 2.0) / fft_vals_.size();
        val = glm::sin(val + curr_time);
        fft_vals_[i] = safe_cast_d_to_f(val);
    }
}

const std::vector<float> &Orb::get_fft_vals() const
{
    return fft_vals_;
}
} // namespace sim

