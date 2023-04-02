//
// Created by Zero on 09/09/2022.
//

#pragma once

#include "base/node.h"
#include "interaction.h"
#include "core/stl.h"
#include "base/scattering/bxdf.h"
#include "base/shader_graph/shader_node.h"

namespace vision {

struct BSDF {
public:
    UVN<Float3> shading_frame;
    Float3 ng;
    const SampledWavelengths &swl;

protected:
    [[nodiscard]] virtual ScatterEval evaluate_local(Float3 wo, Float3 wi, Uint flag) const noexcept {
        ScatterEval ret{swl.dimension()};
        ret.f = {swl.dimension(), 0.f};
        ret.pdf = 1.f;
        return ret;
    }

    [[nodiscard]] virtual BSDFSample sample_local(Float3 wo, Uint flag, Sampler *sampler) const noexcept {
        BSDFSample ret{swl.dimension()};
        return ret;
    }

public:
    BSDF() = default;
    explicit BSDF(const Interaction &it, const SampledWavelengths &swl)
        : shading_frame(it.s_uvn), ng(it.g_uvn.normal()), swl(swl) {}

    [[nodiscard]] virtual SampledSpectrum albedo() const noexcept {
        // todo
        return {swl.dimension(), 0.f};
    }
    [[nodiscard]] virtual optional<Bool> is_dispersive() const noexcept {
        return {};
    }
    [[nodiscard]] static Uint combine_flag(Float3 wo, Float3 wi, Uint flag) noexcept;
    [[nodiscard]] ScatterEval evaluate(Float3 world_wo, Float3 world_wi) const noexcept;
    [[nodiscard]] BSDFSample sample(Float3 world_wo, Sampler *sampler) const noexcept;
};

class DielectricBSDF : public BSDF {
private:
    SP<const Fresnel> _fresnel;
    MicrofacetReflection _refl;
    MicrofacetTransmission _trans;
    Bool _dispersive{};

public:
    DielectricBSDF(const Interaction &it,
                   const SP<Fresnel> &fresnel,
                   MicrofacetReflection refl,
                   MicrofacetTransmission trans,
                   const Bool &dispersive)
        : BSDF(it, refl.swl()), _fresnel(fresnel),
          _refl(move(refl)), _trans(move(trans)),
          _dispersive(dispersive) {}
    [[nodiscard]] SampledSpectrum albedo() const noexcept override { return _refl.albedo(); }
    [[nodiscard]] optional<Bool> is_dispersive() const noexcept override {
        return _dispersive;
    }
    [[nodiscard]] ScatterEval evaluate_local(Float3 wo, Float3 wi, Uint flag) const noexcept override;
    [[nodiscard]] BSDFSample sample_local(Float3 wo, Uint flag, Sampler *sampler) const noexcept override;
};

class Material : public Node {
public:
    using Desc = MaterialDesc;

public:
    explicit Material(const MaterialDesc &desc) : Node(desc) {}
    virtual void fill_data(ManagedWrapper<float> &datas) const noexcept {
        OC_ASSERT(false);
    }
    [[nodiscard]] virtual UP<BSDF> get_BSDF(const Interaction &it, DataAccessor &da, const SampledWavelengths &swl) const noexcept {
        return make_unique<BSDF>(it, swl);
    }
    [[nodiscard]] virtual UP<BSDF> get_BSDF(const Interaction &it, const SampledWavelengths &swl) const noexcept {
        return make_unique<BSDF>(it, swl);
    }
};
}// namespace vision