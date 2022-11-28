//
// Created by Zero on 27/11/2022.
//

#include "base/light.h"
#include "base/texture.h"
#include "base/render_pipeline.h"

namespace vision {
class Projector : public IPointLight {
private:
    float4x4 _o2w;
    float _ratio;
    const Texture *_intensity{};
    float _angle_y;
    float _scale{1.f};

public:
    explicit Projector(const LightDesc &desc)
        : IPointLight(desc),
          _ratio(desc.ratio),
          _angle_y(radians(desc.angle)),
          _o2w(desc.o2w.mat),
          _scale(desc.scale) {
        _intensity = desc.scene->load<Texture>(desc.texture_desc);
        if (_ratio == 0) {
            uint2 res = _intensity->resolution();
            _ratio = float(res.x) / res.y;
        }
    }
    [[nodiscard]] float3 position() const noexcept override { return _o2w[3].xyz(); }
    [[nodiscard]] Float3 Li(const LightSampleContext &p_ref,
                            const LightEvalContext &p_light) const noexcept override {
        Float3 p = transform_point(inverse(_o2w), p_ref.pos);
        Float3 w = p / p.z;
        float tan_y = tan(_angle_y);
        float tan_x = _ratio * tan_y;
        Float u = (p.x + tan_x) / (2 * tan_x);
        Float v = (p.y + tan_y) / (2 * tan_y);
        return make_float3(0.f);
//        print("{},{},{}", p.x,p.y,p.z);
//        return _intensity->eval(make_float2(u,v)).xyz() / length(p) * _scale;
    }
};
}// namespace vision

VS_MAKE_CLASS_CREATOR(vision::Projector)