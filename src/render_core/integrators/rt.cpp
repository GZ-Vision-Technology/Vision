//
// Created by Zero on 2023/9/11.
//

#include "base/integrator.h"
#include "base/mgr/pipeline.h"
#include "math/warp.h"
#include "base/color/spectrum.h"
#include "ReSTIR/direct.h"

namespace vision {

class RealTimeIntegrator : public IlluminationIntegrator {
private:
    ReSTIR _direct;

public:
    explicit RealTimeIntegrator(const IntegratorDesc &desc)
        : IlluminationIntegrator(desc),
          _direct(desc["M"].as_uint(1),
                  desc["n"].as_uint(3),
                  desc["spatial"].as_uint(1)) {}

    void prepare() noexcept override {
        _direct.prepare();
    }

    void compile() noexcept override {
        _direct.compile();

        Camera *camera = scene().camera().get();
        Sampler *sampler = scene().sampler();
        ocarina::Kernel<signature> kernel = [&](Uint frame_index) -> void {
            Uint2 pixel = dispatch_idx().xy();
            _frame_index.emplace(frame_index);
            sampler->start_pixel_sample(pixel, frame_index, 0);
            camera->load_data();
            SensorSample ss = sampler->sensor_sample(pixel, camera->filter());
            Float scatter_pdf = 1e16f;
            RayState rs = camera->generate_ray(ss);
            Float3 L = Li(rs, scatter_pdf, nullptr) * ss.filter_weight;
            camera->radiance_film()->add_sample(pixel, L, frame_index);
        };
        _shader = device().compile(kernel, "real time integrator");
    }

    void render() const noexcept override {
        const Pipeline *rp = pipeline();
        Stream &stream = rp->stream();
        stream << _shader(rp->frame_index()).dispatch(rp->resolution());
        stream << synchronize();
        stream << commit();
    }
};

}// namespace vision

VS_MAKE_CLASS_CREATOR(vision::RealTimeIntegrator)