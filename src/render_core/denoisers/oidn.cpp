//
// Created by Zero on 2023/5/30.
//

#include "base/denoiser.h"
#include "OpenImageDenoise/oidn.hpp"

namespace vision {

using namespace ocarina;

class OidnDenoiser : public Denoiser {
private:
    oidn::DeviceRef *_device{};

public:
    explicit OidnDenoiser(const DenoiserDesc &desc)
        : Denoiser(desc), _device{new oidn::DeviceRef(oidn::newDevice())} {
        _device->commit();
    }

    void apply(uint2 res, float4 *output, float4 *color,
               float4 *normal, float4 *albedo) const noexcept override {
        oidn::FilterRef filter = _device->newFilter("RT");
        filter.setImage("output", output, oidn::Format::Float3, res.x, res.y, 0, sizeof(float4));
        filter.setImage("color", color, oidn::Format::Float3, res.x, res.y, 0, sizeof(float4));
        if (normal && albedo) {
            filter.setImage("normal", normal, oidn::Format::Float3, res.x, res.y, 0, sizeof(float4));
            filter.setImage("albedo", albedo, oidn::Format::Float3, res.x, res.y, 0, sizeof(float4));
        }
        // color image is HDR
        filter.set("hdr", true);
        filter.commit();
        filter.execute();
    }
};

}// namespace vision

VS_MAKE_CLASS_CREATOR(vision::OidnDenoiser)