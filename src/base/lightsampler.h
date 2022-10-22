//
// Created by Zero on 09/09/2022.
//

#pragma once

#include "dsl/common.h"
#include "node.h"
#include "core/stl.h"
#include "light.h"

namespace vision {
using namespace ocarina;
class LightSampler : public Node {
public:
    using Desc = LightSamplerDesc;

protected:
    vector<Light *> _lights;

public:
    explicit LightSampler(const LightSamplerDesc *desc) : Node(desc) {}
    [[nodiscard]] span<const Light *const> lights() const noexcept { return _lights; }
    [[nodiscard]] size_t light_num() const noexcept { return _lights.size(); }
    void add_light(Light *light) noexcept { _lights.push_back(light); }
};
}// namespace vision