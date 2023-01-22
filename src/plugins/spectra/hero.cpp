//
// Created by Zero on 21/12/2022.
//

#include "srgb2spec.h"
#include "base/color/spectrum.h"

namespace vision {

class RGBSigmoidPolynomial {
private:
    array<Float, 3> _c;

private:
    [[nodiscard]] static Float _s(Float x) noexcept {
        return select(isinf(x), cast<float>(x > 0.0f),
                      0.5f * fma(x, rsqrt(fma(x, x, 1.f)), 1.f));
    }

public:
    RGBSigmoidPolynomial() noexcept = default;
    RGBSigmoidPolynomial(Float c0, Float c1, Float c2) noexcept
        : _c{c0, c1, c2} {}
    explicit RGBSigmoidPolynomial(Float3 c) noexcept : _c{c[0], c[1], c[2]} {}
    [[nodiscard]] Float operator()(Float lambda) const noexcept {
        return _s(fma(lambda, fma(lambda, _c[0], _c[1]), _c[2]));// c0 * x * x + c1 * x + c2
    }
};

class HeroWavelengthSpectrum : public Spectrum {
private:
    uint _dimension{};

public:
    explicit HeroWavelengthSpectrum(const SpectrumDesc &desc)
        : Spectrum(desc), _dimension(desc.dimension) {}
    [[nodiscard]] uint dimension() const noexcept override { return _dimension; }
    [[nodiscard]] Float4 srgb(const SampledSpectrum &sp, const SampledWavelengths &swl) const noexcept override {
        return make_float4(0.f);
    }
    [[nodiscard]] SampledWavelengths sample_wavelength(Sampler *sampler) const noexcept override {
        return SampledWavelengths(4);
    }
    [[nodiscard]] Float4 decode_to_albedo(Float3 rgb) const noexcept override {
        return make_float4(rgb, luminance(rgb));
    }
    [[nodiscard]] Float4 decode_to_illumination(Float3 rgb) const noexcept override {
        return make_float4(rgb, luminance(rgb));
    }
};

}// namespace vision

VS_MAKE_CLASS_CREATOR(vision::HeroWavelengthSpectrum)