//
// Created by Zero on 28/10/2022.
//

#include "base/material.h"
#include "base/texture.h"
#include "base/scene.h"

namespace vision {

class MirrorBSDF : public BSDF {
private:
    SP<Fresnel> _fresnel;
    MicrofacetReflection _bxdf;

public:
    MirrorBSDF(const SurfaceInteraction &si, const SP<Fresnel> &fresnel, MicrofacetReflection bxdf)
        : BSDF(si), _fresnel(fresnel), _bxdf(std::move(bxdf)) {}
    [[nodiscard]] Float3 albedo() const noexcept override { return _bxdf.albedo(); }
    [[nodiscard]] BSDFEval evaluate_local(Float3 wo, Float3 wi, Uchar flag) const noexcept override {
        return _bxdf.safe_evaluate(wo, wi, _fresnel);
    }
    [[nodiscard]] BSDFSample sample_local(Float3 wo, Float uc, Float2 u,
                                          Uchar flag) const noexcept override {
        return _bxdf.sample(wo, u, _fresnel);
    }
};

class MirrorMaterial : public Material {
private:
    Texture *_color{};
    Texture *_roughness{};

public:
    explicit MirrorMaterial(const MaterialDesc &desc)
        : Material(desc), _color(desc.scene->load<Texture>(desc.color)),
          _roughness(desc.scene->load<Texture>(desc.roughness)) {}

    [[nodiscard]] UP<BSDF> get_BSDF(const SurfaceInteraction &si) const noexcept override {
        Float3 kr = _color ? _color->eval(si).xyz() : make_float3(0.f);
        Float2 alpha = _roughness ? _roughness->eval(si).xy() : make_float2(0.001f);
        auto microfacet = make_shared<Microfacet<D>>(alpha.x, alpha.y);
        auto fresnel = make_shared<FresnelNoOp>();
        MicrofacetReflection bxdf(kr, microfacet);
        return make_unique<MirrorBSDF>(si, fresnel, move(bxdf));
    }
};

}// namespace vision

VS_MAKE_CLASS_CREATOR(vision::MirrorMaterial)