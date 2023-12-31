//
// Created by Zero on 28/10/2022.
//

#include "base/illumination/lightsampler.h"
#include "base/warper.h"
#include "base/mgr/pipeline.h"
#include "base/sampler.h"

namespace vision {
class PowerLightSampler : public LightSampler {
private:
    SP<Warper> _warper{};

public:
    explicit PowerLightSampler(const LightSamplerDesc &desc)
        : LightSampler(desc) {}

    [[nodiscard]] Float PMF(const LightSampleContext &lsc, const Uint &index) const noexcept override {
        return _warper->PMF(index);
    }

    [[nodiscard]] SampledLight select_light(const LightSampleContext &lsc, const Float &u) const noexcept override {
        SampledLight ret;
        ret.light_index = _warper->sample_discrete(u, nullptr, nullptr);
        ret.PMF = PMF(lsc, ret.light_index);
        return ret;
    }

    void prepare() noexcept override {
        LightSampler::prepare();
        _warper = scene().load_warper();
        vector<float> weights;
        _lights.for_each_instance([&](SP<Light> light) {
            weights.push_back(luminance(light->power()));
        });
        _warper->build(std::move(weights));
        _warper->prepare();
    }
};
}// namespace vision

VS_MAKE_CLASS_CREATOR(vision::PowerLightSampler)