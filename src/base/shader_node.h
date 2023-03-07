//
// Created by Zero on 09/09/2022.
//

#pragma once

#include "node.h"
#include "util/image_io.h"
#include "base/color/spectrum.h"
#include "base/scattering/interaction.h"

namespace vision {

class ShaderNode : public Node {
protected:
    AttrType _type{};

public:
    using Desc = ShaderNodeDesc;

public:
    template<typename T = float4>
    [[nodiscard]] static Float4 eval(const ShaderNode *tex, const AttrEvalContext &ctx,
                                     T val = T{}) noexcept {
        float4 default_val = make_float4(0);
        if constexpr (is_scalar_v<T>) {
            default_val = make_float4(val);
        } else if constexpr (is_vector2_v<T>) {
            default_val = make_float4(val, 0, 0);
        } else if constexpr (is_vector3_v<T>) {
            default_val = make_float4(val, 0);
        } else {
            default_val = val;
        }
        return tex ? tex->eval(ctx) : Float4(val);
    }
    [[nodiscard]] static ColorDecode eval_albedo_spectrum(const ShaderNode *tex,
                                                          const AttrEvalContext &ctx,
                                                          const SampledWavelengths &swl) noexcept {
        return tex ? tex->eval_albedo_spectrum(ctx, swl) : ColorDecode::zero(swl.dimension());
    }
    [[nodiscard]] static ColorDecode eval_illumination_spectrum(const ShaderNode *tex,
                                                                const AttrEvalContext &ctx,
                                                                const SampledWavelengths &swl) noexcept {
        return tex ? tex->eval_illumination_spectrum(ctx, swl) : ColorDecode::zero(swl.dimension());
    }
    [[nodiscard]] static bool is_zero(const ShaderNode *tex) noexcept {
        return tex ? tex->is_zero() : true;
    }
    [[nodiscard]] static bool nonzero(const ShaderNode *tex) noexcept {
        return !is_zero(tex);
    }

public:
    explicit ShaderNode(const ShaderNodeDesc &desc) : Node(desc), _type(desc.type) {}
    [[nodiscard]] virtual bool is_zero() const noexcept { return false; }
    [[nodiscard]] virtual Float4 eval(const AttrEvalContext &tec) const noexcept = 0;
    [[nodiscard]] virtual Float4 eval(const Float2 &uv) const noexcept {
        return eval(AttrEvalContext(uv));
    }
    [[nodiscard]] virtual ColorDecode eval_albedo_spectrum(const AttrEvalContext &tec,
                                                           const SampledWavelengths &swl) const noexcept;
    [[nodiscard]] virtual ColorDecode eval_illumination_spectrum(const AttrEvalContext &tec,
                                                                 const SampledWavelengths &swl) const noexcept;
    [[nodiscard]] virtual ColorDecode eval_albedo_spectrum(const Float2 &uv,
                                                           const SampledWavelengths &swl) const noexcept;
    [[nodiscard]] virtual ColorDecode eval_illumination_spectrum(const Float2 &uv,
                                                                 const SampledWavelengths &swl) const noexcept;
    virtual void for_each_pixel(const function<ImageIO::foreach_signature> &func) const noexcept {
        OC_ERROR("call error");
    }
    [[nodiscard]] virtual uint2 resolution() const noexcept { return make_uint2(0); }
};

}// namespace vision