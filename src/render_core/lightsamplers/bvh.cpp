//
// Created by Zero on 28/10/2022.
//

#include "base/lightsampler.h"
#include "base/mgr/render_pipeline.h"
#include "base/sampler.h"

namespace vision {

struct LightBound {

};

struct LightBVHNode {

};

class BVHLightSampler : public LightSampler {
public:
    explicit BVHLightSampler(const LightSamplerDesc &desc)
        : LightSampler(desc) {}

    void prepare() noexcept override {
        LightSampler::prepare();
        build_bvh();
    }

    void build_bvh() noexcept {

    }
};
}// namespace vision