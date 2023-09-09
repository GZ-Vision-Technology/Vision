//
// Created by Zero on 2023/9/3.
//

#pragma once

#include "core/basic_types.h"
#include "core/stl.h"
#include "dsl/common.h"

namespace vision {
using namespace ocarina;

struct Reservoir {
public:
    static constexpr EPort p = H;
    oc_float<p> weight_sum{};
    oc_float3<p> value{};
    oc_uint<p> sample_num{};
    oc_float3<p> W{};

public:
    void update(oc_float<p> u, oc_float<p> weight, oc_float3<p> v) {
        weight_sum += weight;
        sample_num += 1;
        value = select(u < (weight / weight_sum), v, value);
    }

    template<typename Func>
    void update_W(Func &&func) const noexcept {
        W = weight_sum / cast<float>(sample_num) / func(value);
    }
};

}// namespace vision

OC_STRUCT(vision::Reservoir, weight_sum, value, sample_num, W) {
    static constexpr EPort p = D;
    void update(oc_float<p> u, oc_float<p> weight, oc_float3<p> v) {
        weight_sum += weight;
        sample_num += 1;
        value = select(u < (weight / weight_sum), v, value);
    }
    template<typename Func>
    void update_W(Func && func) const noexcept {
        W = weight_sum / cast<float>(sample_num) / func(value);
    }
};

namespace vision {
using namespace ocarina;

using OCReservoir = Var<Reservoir>;



}// namespace vision