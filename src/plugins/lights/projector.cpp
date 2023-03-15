//
// Created by Zero on 27/11/2022.
//

#include "base/light.h"
#include "base/shader_node.h"
#include "base/mgr/render_pipeline.h"

namespace vision {
class Projector : public IPointLight {
private:
    float4x4 _o2w;
    float _ratio;
    const ShaderNode *_intensity{};
    float _angle_y;
    float _scale{1.f};

public:
    explicit Projector(const LightDesc &desc)
        : IPointLight(desc),
          _ratio(desc["ratio"].as_float(1.f)),
          _angle_y(radians(ocarina::clamp(desc["angle"].as_float(45.f), 1.f, 89.f))),
          _o2w(desc.o2w.mat),
          _scale(desc["scale"].as_float(1.f)) {
        _intensity = desc.scene->load_shader_node(desc.color_desc);
        if (_ratio == 0) {
            uint2 res = _intensity->resolution();
            _ratio = float(res.x) / res.y;
        }
    }
    [[nodiscard]] float3 position() const noexcept override { return _o2w[3].xyz(); }
    [[nodiscard]] SampledSpectrum Li(const LightSampleContext &p_ref,
                             const LightEvalContext &p_light,
                             const SampledWavelengths &swl) const noexcept override {
        Float3 p = transform_point(inverse(_o2w), p_ref.pos);
        Float d2 = length_squared(p);
        Bool valid = p.z > 0;
        p = p / p.z;
        float tan_y = tan(_angle_y);
        float tan_x = _ratio * tan_y;
        float2 tan_xy = make_float2(tan_x, tan_y);
        Float2 uv = (p.xy() + tan_xy) / (2.f * tan_xy);
        valid = valid && all(uv >= 0.f && uv <= 1.f);
        return select(valid, 1.f, 0.f) * _intensity->eval_illumination_spectrum(uv, swl).sample / d2 * _scale;
    }
};
}// namespace vision

VS_MAKE_CLASS_CREATOR(vision::Projector)