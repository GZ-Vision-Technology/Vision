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
    float _dot_threshold{};
    float _depth_threshold{};
    mutable RegistrableManaged<Reservoir> _reservoirs;
    mutable RegistrableManaged<Reservoir> _prev_reservoirs;
    mutable RegistrableManaged<SurfaceData> _surfaces;
    RegistrableManaged<float3> &_motion_vec;

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
    explicit ReSTIR(const IntegratorDesc &desc, RegistrableManaged<float3> &motion_vec)
        : M(desc["M"].as_uint(1)),
          _iterate_num(desc["n"].as_uint(3)),
          _spatial(desc["spatial"].as_uint(1)),
          _dot_threshold(cosf(radians(desc["theta"].as_float(20)))),
          _depth_threshold(desc["depth"].as_float(0.01f)),
          _motion_vec(motion_vec){}

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
    [[nodiscard]] Bool is_neighbor(const OCSurfaceData &cur_surface,
                                   const OCSurfaceData &another_surface) const noexcept;
    void compile_shader0() noexcept;
    void compile_shader1() noexcept;
    [[nodiscard]] CommandList estimate() const noexcept;
};

}// namespace vision