//
// Created by Zero on 22/10/2022.
//

#include "lightsampler.h"
#include "base/mgr/render_pipeline.h"

namespace vision {

LightSampler::LightSampler(const LightSamplerDesc &desc)
    : Node(desc), _env_prob(ocarina::clamp(desc["env_prob"].as_float(0.5f), 0.01f, 0.99f)) {
    for (const LightDesc &light_desc : desc.light_descs) {
        Light *light = desc.scene->load<Light>(light_desc);
        add_light(light);
        if (light->type() == LightType::Infinite) {
            _env_light = light;
        }
    }
    std::sort(_lights.begin(), _lights.end(), [&](Light *a, Light *b) {
        return a->type() > b->type();
    });
}

void LightSampler::prepare() noexcept {
    for_each([&](Light *light) noexcept {
        light->prepare();
    });
    auto rp = render_pipeline();
    _lights.prepare(rp->resource_array(), rp->device());
}

pair<Uint, Uint> LightSampler::extract_light_id(const Uint &index) const noexcept {
    Uint type_id = 0u;
    Uint inst_id = 0u;
    vector<uint> func;
    func.reserve(_lights.type_num());
    for (int i = 0; i < _lights.type_num(); ++i) {
        func.push_back(static_cast<uint>(_lights.instance_num(i)));
    }

    Uint accum = 0u;
    for (uint i = 0; i < func.size(); ++i) {
        type_id = select(index >= accum, i, type_id);
        inst_id = select(index >= accum, index - accum, inst_id);
        accum += func[i];
    }
    switch (_lights.mode()) {
        case ocarina::EInstance:
            return {type_id, index};
        case ocarina::EType:
            return {type_id, inst_id};
        default:
            break;
    }
    OC_ASSERT(false);
    return {type_id, inst_id};
}

LightEval LightSampler::evaluate_hit(const LightSampleContext &p_ref, const Interaction &it,
                                     const SampledWavelengths &swl) const noexcept {
    LightEval ret = {{swl.dimension(), 0.f}, 0.f};
    dispatch_light(it.light_id(), [&](const Light *light) {
        if (light->type() != LightType::Area) { return; }
        LightEvalContext p_light{it};
        p_light.PDF_pos *= light->PMF(it.prim_id);
        ret = light->evaluate(p_ref, p_light, swl);
    });
    Float pmf = PMF(p_ref, it.light_inst_id());
    ret.pdf *= pmf;
    return ret;
}

LightEval LightSampler::evaluate_miss(const LightSampleContext &p_ref, Float3 wi,
                                      const SampledWavelengths &swl) const noexcept {
    LightEvalContext p_light{p_ref.pos + wi};
    LightEval ret = env_light()->evaluate(p_ref, p_light, swl);
    Float pmf = 1.f / light_num();
    ret.pdf *= pmf;
    return ret;
}

void LightSampler::dispatch_light(const Uint &id, const std::function<void(const Light *)> &func) const noexcept {
    _lights.dispatch(id, func);
}

void LightSampler::dispatch_light(const Uint &type_id, const Uint &inst_id,
                                  const std::function<void(const Light *)> &func) const noexcept {
    _lights.dispatch(type_id, inst_id, func);
}

}// namespace vision