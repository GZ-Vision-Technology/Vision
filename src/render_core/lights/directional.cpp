//
// Created by Zero on 2023/8/7.
//

#include "base/illumination/light.h"
#include "base/mgr/pipeline.h"
#include "base/color/spectrum.h"

namespace vision {

//    "type": "directional",
//    "param": {
//        "color" : {
//            "channels" : "xyz",
//            "node" : [17,12,4]
//        },
//        "scale" : 1,
//        "direction" : [1,1,1]
//    }
class DirectionalLight : public Light {
private:
    Serial<float3> _direction;
    Serial<float> _world_radius;
    Serial<float3> _world_center;

public:
    explicit DirectionalLight(const LightDesc &desc)
        : Light(desc, LightType::DeltaDirection),
          _direction(desc["direction"].as_float3(make_float3(-1.f))) {}

    void prepare() noexcept override {
        _world_radius = scene().world_radius();
        _world_center = scene().world_center();
    }

    [[nodiscard]] Float3 w_light() const noexcept {
        return -(*_direction);
    }

    [[nodiscard]] float3 power() const noexcept override {
        return average() * Pi * ocarina::sqr(_world_radius.hv());
    }

    [[nodiscard]] LightSample sample_Li(const LightSampleContext &p_ref, Float2 u,
                                        const SampledWavelengths &swl) const noexcept override {
        LightSample ret{swl.dimension()};
        ret.p_light = p_ref.pos + w_light() * *_world_radius;
        ret.eval = evaluate(p_ref, LightEvalContext(ret.p_light), swl);
        return ret;
    }

    [[nodiscard]] SampledSpectrum Li(const LightSampleContext &p_ref,
                                     const LightEvalContext &p_light,
                                     const SampledWavelengths &swl) const noexcept override {
        return _color.eval_illumination_spectrum(p_light.uv, swl).sample * scale();
    }

    [[nodiscard]] Float PDF_Li(const LightSampleContext &p_ref,
                               const LightEvalContext &p_light) const noexcept override {
        // using -1 for delta light
        return -1.f;
    }
};

}// namespace vision

VS_MAKE_CLASS_CREATOR(vision::DirectionalLight)