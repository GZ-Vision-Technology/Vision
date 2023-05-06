//
// Created by Zero on 28/10/2022.
//

#include "base/scattering/material.h"
#include "base/shader_graph/shader_node.h"
#include "base/mgr/scene.h"
#include "metal_ior.h"

namespace vision {

class FresnelConductor : public Fresnel {
private:
    SampledSpectrum _eta, _k;

public:
    FresnelConductor(const SampledSpectrum &eta, const SampledSpectrum &k, const SampledWavelengths &swl, const RenderPipeline *rp)
        : Fresnel(swl, rp), _eta(eta), _k(k) {}
    [[nodiscard]] SampledSpectrum evaluate(Float abs_cos_theta) const noexcept override {
        return fresnel_complex(abs_cos_theta, _eta, _k);
    }
    [[nodiscard]] SP<Fresnel> clone() const noexcept override {
        return make_shared<FresnelConductor>(_eta, _k, _swl, _rp);
    }
};

class ConductorBSDF : public BxDFSet {
private:
    SP<const Fresnel> _fresnel;
    MicrofacetReflection _refl;

public:
    ConductorBSDF(const SP<Fresnel> &fresnel,
                  MicrofacetReflection refl)
        : _fresnel(fresnel), _refl(ocarina::move(refl)) {}
    [[nodiscard]] SampledSpectrum albedo() const noexcept override { return _refl.albedo(); }
    [[nodiscard]] ScatterEval evaluate_local(Float3 wo, Float3 wi, Uint flag) const noexcept override {
        return _refl.safe_evaluate(wo, wi, _fresnel->clone());
    }
    [[nodiscard]] BSDFSample sample_local(Float3 wo, Uint flag, Sampler *sampler) const noexcept override {
        return _refl.sample(wo, sampler, _fresnel->clone());
    }
};

class MetalMaterial : public Material {
private:
    Slot _eta;
    Slot _k;
    Slot _roughness{};
    bool _remapping_roughness{false};

public:
    explicit MetalMaterial(const MaterialDesc &desc)
        : Material(desc),
          _roughness(_scene->create_slot(desc.slot("roughness", make_float2(0.01f)))),
          _remapping_roughness(desc["remapping_roughness"].as_bool(false)) {
        init_ior(desc);
        init_slot_cursor(&_eta, 3);
    }

    void init_ior(const MaterialDesc &desc) noexcept {
        const ComplexIor &complex_ior = ComplexIorTable::instance()->get_ior(desc["material_name"].as_string());
        SlotDesc eta_slot;
        SlotDesc k_slot;
        if (spectrum().is_complete()) {
            eta_slot = SlotDesc(ShaderNodeDesc{ShaderNodeType::ESPD, "spd"}, 0);
            eta_slot.node.set_value("value", complex_ior.eta);
            k_slot = SlotDesc(ShaderNodeDesc{ShaderNodeType::ESPD, "spd"}, 0);
            k_slot.node.set_value("value", complex_ior.k);
        } else {
            SPD spd_eta = SPD(complex_ior.eta, nullptr);
            SPD spd_k = SPD(complex_ior.k, nullptr);
            float3 eta = spd_eta.eval(rgb_spectrum_peak_wavelengths);
            float3 k = spd_k.eval(rgb_spectrum_peak_wavelengths);
            eta_slot = desc.slot("", eta);
            k_slot = desc.slot("", k);
        }

        _eta = _scene->create_slot(eta_slot);
        _k = _scene->create_slot(k_slot);
    }

    void prepare() noexcept override {
        _eta->prepare();
        _k->prepare();
    }

    [[nodiscard]] BSDF compute_BSDF(const Interaction &it, const SampledWavelengths &swl) const noexcept override {
        SampledSpectrum kr{swl.dimension(), 1.f};
        Float2 alpha = _roughness.evaluate(it, swl).as_vec2();
        alpha = _remapping_roughness ? roughness_to_alpha(alpha) : alpha;
        alpha = clamp(alpha, make_float2(0.0001f), make_float2(1.f));
        SampledSpectrum eta = SampledSpectrum{_eta.evaluate(it, swl)};
        SampledSpectrum k = SampledSpectrum{_k.evaluate(it, swl)};
        auto microfacet = make_shared<GGXMicrofacet>(alpha.x, alpha.y);
        auto fresnel = make_shared<FresnelConductor>(eta, k, swl, render_pipeline());
        MicrofacetReflection bxdf(kr, swl, microfacet);
        return BSDF(it, swl, make_unique<ConductorBSDF>(fresnel, ocarina::move(bxdf)));
    }
};

}// namespace vision

VS_MAKE_CLASS_CREATOR(vision::MetalMaterial)