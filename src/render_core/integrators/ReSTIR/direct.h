//
// Created by Zero on 2023/9/3.
//

#pragma once

#include "util.h"
#include "base/serial_object.h"
#include "base/mgr/global.h"

namespace vision {
/**
 * generate initial candidates
 * evaluate visibility for initial candidates
 * temporal reuse
 * spatial reuse and iterate
 */
class ReSTIR : public SerialObject, public Ctx {
private:
    uint M{};
    uint _iterate_num{};
    int _spatial{1};
    float _epsilon_dot{};
    float _epsilon_depth{};
    mutable RegistrableManaged<Reservoir> _reservoirs;
    mutable RegistrableManaged<Reservoir> _prev_reservoirs;
    mutable RegistrableManaged<GData> GBuffer;

    /**
     * generate initial candidates
     * check visibility
     */
    Shader<void(uint)> _shader0;
    /**
     * spatial temporal reuse and shading
     */
    Shader<void(uint)> _shader1;

public:
    explicit ReSTIR(uint M, uint n, uint spatial, float theta, float depth)
        : M(M), _iterate_num(n), _spatial(spatial),
          _epsilon_dot(cosf(radians(theta))), _epsilon_depth(depth) {}
    void prepare() noexcept;
    void compile() noexcept {
        compile_shader0();
        compile_shader1();
    }
    [[nodiscard]] OCReservoir RIS(Bool hit, const Interaction &it, SampledWavelengths &swl,
                                  const Uint &frame_index) const noexcept;
    [[nodiscard]] OCReservoir spatial_reuse(const Int2 &pixel, const Uint &frame_index) const noexcept;
    [[nodiscard]] OCReservoir temporal_reuse(const OCReservoir &rsv) const noexcept;
    [[nodiscard]] Float3 shading(const OCReservoir &rsv, const OCHit &hit,
                                 SampledWavelengths &swl, const Uint &frame_index) const noexcept;
    void compile_shader0() noexcept;
    void compile_shader1() noexcept;
    [[nodiscard]] CommandList estimate() const noexcept;
};

}// namespace vision