//
// Created by Zero on 28/10/2022.
//

#include "material.h"
#include "base/sampler.h"
#include "base/mgr/scene.h"

namespace vision {

Uint BSDF::combine_flag(Float3 wo, Float3 wi, Uint flag) noexcept {
    Bool reflect = same_hemisphere(wo, wi);
    Uint non_reflect = ~BxDFFlag::Reflection;
    Uint non_trans = ~BxDFFlag::Transmission;
    return select(reflect, flag & non_trans, flag & non_reflect);
}

ScatterEval BSDF::evaluate(Float3 world_wo, Float3 world_wi) const noexcept {
    Float3 wo = shading_frame.to_local(world_wo);
    Float3 wi = shading_frame.to_local(world_wi);
    ScatterEval ret = evaluate_local(wo, wi, BxDFFlag::All);
    ret.f *= abs_cos_theta(wi);
    return ret;
}

BSDFSample BSDF::sample(Float3 world_wo, Sampler *sampler) const noexcept {
    Float3 wo = shading_frame.to_local(world_wo);
    BSDFSample ret = sample_local(wo, BxDFFlag::All, sampler);
    ret.eval.f *= abs_cos_theta(ret.wi);
    ret.wi = shading_frame.to_world(ret.wi);
    return ret;
}

ScatterEval DielectricBSDF::evaluate_local(Float3 wo, Float3 wi, Uint flag) const noexcept {
    ScatterEval ret{swl.dimension()};
    auto fresnel = _fresnel->clone();
    Float cos_theta_o = cos_theta(wo);
    fresnel->correct_eta(cos_theta_o);
    $if(same_hemisphere(wo, wi)) {
        ret = _refl.evaluate(wo, wi, fresnel);
    }
    $else {
        ret = _trans.evaluate(wo, wi, fresnel);
    };
    return ret;
}

BSDFSample DielectricBSDF::sample_local(Float3 wo, Uint flag, Sampler *sampler) const noexcept {
    BSDFSample ret{swl.dimension()};
    Float uc = sampler->next_1d();
    auto fresnel = _fresnel->clone();
    Float cos_theta_o = cos_theta(wo);
    fresnel->correct_eta(cos_theta_o);
    Float fr = fresnel->evaluate(abs_cos_theta(wo))[0];
    $if(uc < fr) {
        ret = _refl.sample(wo, sampler, fresnel);
        ret.eval.pdf *= fr;
    }
    $else {
        ret = _trans.sample(wo, sampler, fresnel);
        ret.eval.pdf *= 1 - fr;
    };
    return ret;
}

void Material::fill_data(ManagedWrapper<float> &datas) const noexcept {
    for (int i = 0; i < _slot_cursor.num; ++i) {
        const Slot &slot = _slot_cursor.ptr[i];
        slot->fill_data(datas);
    }
}

uint Material::data_size() const noexcept {
    uint ret = 0;
    for (int i = 0; i < _slot_cursor.num; ++i) {
        const Slot &slot = _slot_cursor.ptr[i];
        ret += slot->data_size();
    }
    return ret;
}

uint64_t Material::_compute_type_hash() const noexcept {
    uint64_t ret = Hash64::default_seed;
    for (int i = 0; i < _slot_cursor.num; ++i) {
        const Slot &slot = _slot_cursor.ptr[i];
        ret = hash64(ret, slot.type_hash());
    }
    return ret;
}

uint64_t Material::_compute_hash() const noexcept {
    uint64_t ret = Hash64::default_seed;
    for (int i = 0; i < _slot_cursor.num; ++i) {
        const Slot &slot = _slot_cursor.ptr[i];
        ret = hash64(ret, slot.hash());
    }
    return ret;
}
}// namespace vision