//
// Created by Zero on 21/12/2022.
//

#include "base/color/spectrum.h"

namespace vision {

class SRGBSpectrum : public Spectrum {
public:
    explicit SRGBSpectrum(const SpectrumDesc &desc)
        : Spectrum(desc) {}

    [[nodiscard]] SampledWavelengths sample_wavelength(Sampler *sampler) const noexcept override {
        return SampledWavelengths(3);
    }

    [[nodiscard]] Float4 srgb(const SampledSpectrum &sp,const SampledWavelengths &swl) const noexcept override {
        return make_float4(sp[0], sp[1], sp[2], 1.f);
    }
    [[nodiscard]] Float4 decode_to_albedo(Float3 rgb) const noexcept override {
        return make_float4(rgb, luminance(rgb));
    }
    [[nodiscard]] Float4 decode_to_illumination(Float3 rgb) const noexcept override {
        return make_float4(rgb, luminance(rgb));
    }
};

}// namespace vision

VS_MAKE_CLASS_CREATOR(vision::SRGBSpectrum)