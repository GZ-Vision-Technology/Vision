//
// Created by Zero on 09/09/2022.
//

#include "base/scattering/material.h"
#include "base/shader_graph/shader_node.h"
#include "base/mgr/scene.h"
#include "base/mgr/pipeline.h"

namespace vision {

class LambertReflection : public BxDF {
private:
    SampledSpectrum Kr;

public:
    explicit LambertReflection(SampledSpectrum kr, const SampledWavelengths &swl)
        : BxDF(swl, BxDFFlag::DiffRefl),
          Kr(kr) {}
    [[nodiscard]] SampledSpectrum albedo() const noexcept override { return Kr; }
    [[nodiscard]] SampledSpectrum f(Float3 wo, Float3 wi, SP<Fresnel> fresnel) const noexcept override {
        return Kr * InvPi;
    }
};

class OrenNayar : public BxDF {
private:
    SampledSpectrum R;
    Float A, B;

public:
    OrenNayar(SampledSpectrum R, Float sigma, const SampledWavelengths &swl)
        : BxDF(swl, BxDFFlag::DiffRefl), R(R) {
        sigma = radians(sigma);
        Float sigma2 = ocarina::sqr(sigma * sigma);
        A = 1.f - (sigma2 / (2.f * (sigma2 + 0.33f)));
        B = 0.45f * sigma2 / (sigma2 + 0.09f);
    }
    [[nodiscard]] SampledSpectrum albedo() const noexcept override { return R; }
    [[nodiscard]] SampledSpectrum f(Float3 wo, Float3 wi, SP<Fresnel> fresnel) const noexcept override {
        Float sin_theta_i = sin_theta(wi);
        Float sin_theta_o = sin_theta(wo);

        Float sin_phi_i = sin_phi(wi);
        Float cos_phi_i = cos_phi(wi);
        Float sin_phi_o = sin_phi(wo);
        Float cos_phi_o = cos_phi(wo);
        Float d_cos = cos_phi_i * cos_phi_o + sin_phi_i * sin_phi_o;

        Float max_cos = ocarina::max(0.f, d_cos);

        Bool cond = abs_cos_theta(wi) > abs_cos_theta(wo);
        Float sin_alpha = select(cond, sin_theta_o, sin_theta_i);
        Float tan_beta = select(cond, sin_theta_i/ abs_cos_theta(wi),
                                sin_theta_o / abs_cos_theta(wo));


        return R * InvPi * (A + B * max_cos * sin_alpha * tan_beta);
    }
};

class MatteBxDFSet : public BxDFSet {
private:
    UP<BxDF> _bxdf;

public:
    MatteBxDFSet(const SampledSpectrum &kr, const SampledWavelengths &swl)
        : _bxdf(std::make_unique<LambertReflection>(kr, swl)) {}
    MatteBxDFSet(SampledSpectrum R, Float sigma, const SampledWavelengths &swl)
        : _bxdf(std::make_unique<OrenNayar>(R, sigma, swl)) {}

    [[nodiscard]] SampledSpectrum albedo() const noexcept override { return _bxdf->albedo(); }
    [[nodiscard]] ScatterEval evaluate_local(Float3 wo, Float3 wi, Uint flag) const noexcept override {
        return _bxdf->safe_evaluate(wo, wi, nullptr);
    }
    [[nodiscard]] BSDFSample sample_local(Float3 wo, Uint flag, Sampler *sampler) const noexcept override {
        return _bxdf->sample(wo, sampler, nullptr);
    }
    [[nodiscard]] SampledDirection sample_wi(Float3 wo, Uint flag, Sampler *sampler) const noexcept override {
        return _bxdf->sample_wi(wo, sampler->next_2d(), nullptr);
    }
};

class MatteMaterial : public Material {
private:
    Slot _color{};
    Slot _sigma{};

public:
    explicit MatteMaterial(const MaterialDesc &desc)
        : Material(desc), _color(scene().create_slot(desc.slot("color", make_float3(0.5f), Albedo))) {
        init_slot_cursor(&_color, 3);
        if (desc.has_attr("sigma")) {
            _sigma = scene().create_slot(desc.slot("sigma", 1.f, Number));
        }
    }

    [[nodiscard]] BSDF compute_BSDF(const Interaction &it, const SampledWavelengths &swl) const noexcept override {
        SampledSpectrum kr = _color.eval_albedo_spectrum(it, swl).sample;
        if (_sigma) {
            Float sigma = _sigma.evaluate(it, swl).as_scalar();
            return BSDF(it, make_unique<MatteBxDFSet>(kr, sigma, swl));
        }
        return BSDF(it, make_unique<MatteBxDFSet>(kr, swl));
    }
};
}// namespace vision

VS_MAKE_CLASS_CREATOR(vision::MatteMaterial)