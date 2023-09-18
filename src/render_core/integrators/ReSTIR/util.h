//
// Created by Zero on 2023/9/3.
//

#pragma once

#include "core/basic_types.h"
#include "core/stl.h"
#include "dsl/common.h"

namespace vision {
using namespace ocarina;
struct RSVSample {
    uint light_index{};
    uint prim_id{};
    float2 u;
    float p_hat;
    float pdf;
    array<float, 3> pos;
    [[nodiscard]] auto p_light() const noexcept {
        return make_float3(pos[0], pos[1], pos[2]);
    }
    void set_pos(float3 p) noexcept {
        pos[0] = p[0];
        pos[1] = p[1];
        pos[2] = p[2];
    }
};
}// namespace vision

// clang-format off
OC_STRUCT(vision::RSVSample, light_index, prim_id, u, p_hat, pdf, pos) {
    [[nodiscard]] auto p_light() const noexcept {
        return make_float3(pos[0], pos[1], pos[2]);
    }
    void set_pos(Float3 p) noexcept {
        pos[0] = p[0];
        pos[1] = p[1];
        pos[2] = p[2];
    }
};
// clang-format on

namespace vision {
using namespace ocarina;
struct GData {
    Hit hit{};
    float t{};
};

}// namespace vision

// clang-format off
OC_STRUCT(vision::GData, hit, t) {
};
// clang-format on

namespace vision {
using OCGData = Var<GData>;
using OCRSVSample = Var<RSVSample>;
}

namespace vision {
using namespace ocarina;

struct Reservoir {
public:
    static constexpr EPort p = H;
    oc_float<p> weight_sum{};
    oc_uint<p> M{};
    RSVSample sample{};

public:
    void update(oc_float<p> u, oc_float<p> weight, RSVSample v) {
        weight_sum += weight;
        M += 1;
        sample = ocarina::select(u < (weight / M), v, sample);
    }
    [[nodiscard]] auto W() const noexcept {
        auto f = weight_sum / (M * sample.p_hat);
        return ocarina::select(M * sample.p_hat == 0.f, 0.f, f);
    }
    void invalidate() noexcept { weight_sum = 0.f; }
    [[nodiscard]] auto valid() const noexcept { return weight_sum > 0.f; }
};

}// namespace vision

OC_STRUCT(vision::Reservoir, weight_sum, M, sample) {
    static constexpr EPort p = D;
    void update(oc_float<p> u, oc_float<p> weight, vision::OCRSVSample v) {
        weight_sum += weight;
        M += 1;
        sample = select(u < (weight / weight_sum), v, sample);
    }
    [[nodiscard]] auto W() const noexcept {
        auto f = weight_sum / (M * sample.p_hat);
        return ocarina::select(M * sample.p_hat == 0.f, 0.f, f);
    }
    void invalidate() noexcept { weight_sum = 0.f; }
    [[nodiscard]] auto valid() const noexcept { return weight_sum > 0.f; }
};

namespace vision {
using namespace ocarina;
using OCReservoir = Var<Reservoir>;
[[nodiscard]] inline OCReservoir combine_reservoir(const OCReservoir &r0,
                                                   const OCReservoir &r1,
                                                   const Float &u) noexcept {
    OCReservoir ret;
    ret = r0;
    ret->update(u, r1.weight_sum, r1.sample);
    ret.M = r0.M + r1.M;
    return ret;
}
}// namespace vision