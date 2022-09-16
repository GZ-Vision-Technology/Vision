//
// Created by Zero on 10/09/2022.
//

#include "descriptions.h"
#include "parameter_set.h"

namespace vision {

void TransformDesc::init(const ParameterSet &ps) noexcept {
    name = ps["type"].as_string("look_at");
    ParameterSet param = ps["param"];
}
void ShapeDesc::init(const ParameterSet &ps) noexcept {
}
void SamplerDesc::init(const ParameterSet &ps) noexcept {
}
void FilterDesc::init(const ParameterSet &ps) noexcept {
}
void SensorDesc::init(const ParameterSet &ps) noexcept {
    name = ps["type"].as_string("ThinLensCamera");
    ParameterSet param = ps["param"];
    velocity = param["velocity"].as_float(5.f);
    fov_y = param["fov_y"].as_float(20.f);
    transform_desc.init(param["transform"]);
}
void IntegratorDesc::init(const ParameterSet &ps) noexcept {
    name = ps["type"].as_string("PTIntegrator");
    ParameterSet param = ps["param"];
    max_depth = param["max_depth"].as_uint(10);
    min_depth = param["min_depth"].as_uint(0);
    rr_threshold = param["rr_threshold"].as_float(1);
}
void MaterialDesc::init(const ParameterSet &ps) noexcept {
}
void LightDesc::init(const ParameterSet &ps) noexcept {
}
void TextureDesc::init(const ParameterSet &ps) noexcept {
}
void LightSamplerDesc::init(const ParameterSet &ps) noexcept {
}
void FilmDesc::init(const ParameterSet &ps) noexcept {
}
}// namespace vision