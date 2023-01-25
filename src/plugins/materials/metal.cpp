//
// Created by Zero on 28/10/2022.
//

#include "base/scattering/material.h"
#include "base/texture.h"
#include "base/mgr/scene.h"

namespace vision {

class FresnelConductor : public Fresnel {
private:
    SampledSpectrum _eta, _k;

public:
    FresnelConductor(const SampledSpectrum &eta,const SampledSpectrum &k, const SampledWavelengths &swl, const RenderPipeline *rp)
        : Fresnel(swl, rp), _eta(eta), _k(k) {}
    [[nodiscard]] SampledSpectrum evaluate(Float abs_cos_theta) const noexcept override {
        return fresnel_complex(abs_cos_theta, _eta, _k);
    }
    [[nodiscard]] SP<Fresnel> clone() const noexcept override {
        return make_shared<FresnelConductor>(_eta, _k, _swl, _rp);
    }
};

class ConductorBSDF : public BSDF {
private:
    SP<const Fresnel> _fresnel;
    MicrofacetReflection _refl;

public:
    ConductorBSDF(const Interaction &si,
              const SP<Fresnel> &fresnel,
              MicrofacetReflection refl)
        : BSDF(si), _fresnel(fresnel), _refl(move(refl)) {}
    [[nodiscard]] SampledSpectrum albedo() const noexcept override { return _refl.albedo(); }
    [[nodiscard]] ScatterEval evaluate_local(Float3 wo, Float3 wi, Uchar flag) const noexcept override {
        return _refl.safe_evaluate(wo, wi, _fresnel->clone());
    }
    [[nodiscard]] BSDFSample sample_local(Float3 wo, Uchar flag, Sampler *sampler) const noexcept override {
        return _refl.sample(wo, sampler, _fresnel->clone());
    }
};

class MetalMaterial : public Material {
private:
    const Texture *_eta{};
    const Texture *_k{};
    const Texture *_roughness{};
    bool _remapping_roughness{false};

public:
    explicit MetalMaterial(const MaterialDesc &desc)
        : Material(desc),
          _eta(desc.scene->load<Texture>(desc.eta)),
          _k(desc.scene->load<Texture>(desc.k)),
          _roughness(desc.scene->load<Texture>(desc.roughness)),
          _remapping_roughness(desc.remapping_roughness) {}

    [[nodiscard]] UP<BSDF> get_BSDF(const Interaction &si, const SampledWavelengths &swl) const noexcept override {
        SampledSpectrum kr{swl.dimension(), 1.f};
        Float2 alpha = Texture::eval(_roughness, si, 0.0001f).xy();
        alpha = _remapping_roughness ? roughness_to_alpha(alpha) : alpha;
        alpha = clamp(alpha, make_float2(0.0001f), make_float2(1.f));
        SampledSpectrum eta = Texture::eval_illumination_spectrum(_eta, si, swl).sample;
        SampledSpectrum k = Texture::eval_illumination_spectrum(_k, si, swl).sample;
        auto microfacet = make_shared<GGXMicrofacet>(alpha.x, alpha.y);
        auto fresnel = make_shared<FresnelConductor>(eta, k, swl, render_pipeline());
        MicrofacetReflection bxdf(kr, swl,microfacet);
        return make_unique<ConductorBSDF>(si, fresnel, move(bxdf));
    }
};

}// namespace vision

VS_MAKE_CLASS_CREATOR(vision::MetalMaterial)