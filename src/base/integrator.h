//
// Created by Zero on 10/09/2022.
//

#pragma once

#include "node.h"
#include "rhi/common.h"
#include "base/scattering/interaction.h"
#include "base/illumination/lightsampler.h"
#include "base/scattering/material.h"
#include "sampler.h"
#include "math/warp.h"

namespace vision {

class Pipeline;

class ScatterFunction;

class Sampler;

//"integrator": {
//    "type": "pt",
//    "param": {
//        "min_depth": 5,
//        "max_depth": 16,
//        "rr_threshold": 1
//    }
//}
class Integrator : public Node {
public:
    using Desc = IntegratorDesc;
    using signature = void(uint);

protected:
    optional<Uint> _frame_index{};
    uint _max_depth{};
    uint _min_depth{};
    float _rr_threshold{};
    ocarina::Shader<signature> _shader;

public:
    explicit Integrator(const IntegratorDesc &desc)
        : Node(desc),
          _max_depth(desc["max_depth"].as_uint(16)),
          _min_depth(desc["min_depth"].as_uint(5)),
          _rr_threshold(desc["rr_threshold"].as_float(1.f)) {}
    virtual void compile_shader() noexcept = 0;
    virtual void render() const noexcept = 0;
    [[nodiscard]] Uint frame_index() const noexcept { return *_frame_index; }
};
}// namespace vision