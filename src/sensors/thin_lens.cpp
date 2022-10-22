//
// Created by Zero on 09/09/2022.
//

#include "base/sensor.h"
#include "core/render_pipeline.h"

namespace vision {
struct ThinLensCameraData {
    float focal_distance{};
    float lens_radius{0.f};
};
}// namespace vision

OC_STRUCT(vision::ThinLensCameraData, focal_distance, lens_radius){};

namespace vision {
class ThinLensCamera : public Camera {
private:
    ThinLensCameraData _host_data;

public:
    explicit ThinLensCamera(const SensorDesc &desc)
        : Camera(desc) {
        _host_data.focal_distance = desc.focal_distance;
        _host_data.lens_radius = desc.lens_radius;
    }
    void prepare(RenderPipeline *rp) noexcept override {
        Camera::prepare(rp);
    }
};
}// namespace vision

VS_MAKE_CLASS_CREATOR(vision::ThinLensCamera)