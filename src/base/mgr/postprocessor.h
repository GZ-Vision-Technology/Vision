//
// Created by Zero on 2023/6/1.
//

#pragma once

#include "base/denoiser.h"
#include "base/tonemapper.h"

namespace vision {
class RenderPipeline;
class Postprocessor {
private:
    using signature = void(Buffer<float4>, Buffer<float4>);

private:
    RenderPipeline *_rp{};
    Denoiser *_denoiser{};
    ToneMapper *_tone_mapper{};
    ocarina::Shader<signature> _shader;

public:
    explicit Postprocessor(RenderPipeline *rp);
    void set_denoiser(Denoiser *denoiser) noexcept { _denoiser = denoiser; }
    void set_tone_mapper(ToneMapper *tone_mapper) noexcept {
        if (_tone_mapper && tone_mapper->type_hash() == _tone_mapper->type_hash()) {
            return;
        }
        _tone_mapper = tone_mapper;
        compile_kernel();
    }
    void compile_kernel() noexcept;
    template<typename... Args>
    void denoise(Args &&...args) const noexcept {
        _denoiser->apply(OC_FORWARD(args)...);
    }
    void tone_mapping(RegistrableManaged<float4> &input, RegistrableManaged<float4> &output) const noexcept {
    }
};
}// namespace vision