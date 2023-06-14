//
// Created by Zero on 09/09/2022.
//

#pragma once

#include "dsl/common.h"
#include "base/node.h"
#include "core/stl.h"
#include "light.h"
#include "base/scattering/interaction.h"

namespace vision {
using namespace ocarina;

struct SampledLight {
    Uint light_index;
    Float PMF;
};

class Sampler;

class LightSampler : public Node {
public:
    using Desc = LightSamplerDesc;

protected:
    Polymorphic<Light *> _lights;
    Light *_env_light{};
    uint _env_index{InvalidUI32};
    float _env_prob{};

public:
    explicit LightSampler(const LightSamplerDesc &desc);
    void prepare() noexcept override;
    template<typename... Args>
    void set_mode(Args &&...args) noexcept {
        _lights.set_mode(OC_FORWARD(args)...);
    }
    [[nodiscard]] const Light *env_light() const noexcept { return _env_light; }
    [[nodiscard]] const Polymorphic<Light *> &lights() const noexcept { return _lights; }
    [[nodiscard]] Polymorphic<Light *> &lights() noexcept { return _lights; }
    [[nodiscard]] uint light_num() const noexcept { return _lights.size(); }
    void add_light(Light *light) noexcept { _lights.push_back(light); }
    [[nodiscard]] virtual Float PMF(const LightSampleContext &lsc, const Uint &index) const noexcept = 0;
    [[nodiscard]] virtual LightEval evaluate_hit(const LightSampleContext &p_ref, const Interaction &it,
                                                 const SampledWavelengths &swl) const noexcept;
    [[nodiscard]] virtual LightEval evaluate_miss(const LightSampleContext &p_ref, Float3 wi,
                                                  const SampledWavelengths &swl) const noexcept;
    [[nodiscard]] virtual SampledLight select_light(const LightSampleContext &lsc, const Float &u) const noexcept = 0;
    [[nodiscard]] pair<Uint, Uint> extract_light_id(const Uint &index) const noexcept;
    [[nodiscard]] Uint combine_to_light_index(const Uint &type_id, const Uint &inst_id) const noexcept;
    [[nodiscard]] virtual LightSample sample(const LightSampleContext &lsc, Sampler *sampler,
                                             const SampledWavelengths &swl) const noexcept;
    void dispatch_light(const Uint &id, const std::function<void(const Light *)> &func) const noexcept;
    void dispatch_light(const Uint &type_id, const Uint &inst_id, const std::function<void(const Light *)> &func) const noexcept;
    template<typename Func>
    void for_each(Func &&func) noexcept {
        if constexpr (std::invocable<Func, Light *>) {
            for (Light *light : _lights) {
                func(light);
            }
        } else {
            uint i = 0u;
            for (Light *light : _lights) {
                func(light, i++);
            }
        }
    }

    template<typename Func>
    void for_each(Func &&func) const noexcept {
        if constexpr (std::invocable<Func, const Light *>) {
            for (const Light *light : _lights) {
                func(light);
            }
        } else {
            uint i = 0u;
            for (const Light *light : _lights) {
                func(light, i++);
            }
        }
    }
};
}// namespace vision