//
// Created by Zero on 03/10/2022.
//

#pragma once

#include "core/basic_types.h"
#include "dsl/common.h"
#include "math/geometry.h"

namespace vision {

using namespace ocarina;

template<typename T>
requires is_vector3_expr_v<T>
struct UVN : Frame<T> {
public:
    using vec_ty = T;

public:
    void set_frame(Frame<T> frame) {
        this->x = frame.x;
        this->y = frame.y;
        this->z = frame.z;
    }
    [[nodiscard]] vec_ty dp_du() const noexcept { return this->x; }
    [[nodiscard]] vec_ty dp_dv() const noexcept { return this->y; }
    [[nodiscard]] vec_ty normal() const noexcept { return this->z; }
    [[nodiscard]] boolean_t<T> valid() const noexcept { return nonzero(normal()); }
};

struct Interaction {
public:
    Float3 pos;
    Float3 wo;
    Float3 time;
    UVN<Float3> g_uvn;

public:
    Interaction() = default;
    [[nodiscard]] Bool on_surface() const noexcept { return g_uvn.valid(); }
    [[nodiscard]] OCRay spawn_ray(const Float3 &dir) const noexcept {
        return vision::spawn_ray(pos, g_uvn.normal(), dir);
    }
    [[nodiscard]] OCRay spawn_ray_to(const Float3 &p) const noexcept {
        return vision::spawn_ray_to(pos, g_uvn.normal(), p);
    }
    [[nodiscard]] OCRay spawn_ray_to(const Interaction &it) const noexcept {
        return vision::spawn_ray_to(pos, g_uvn.normal(), it.pos, it.g_uvn.normal());
    }
};

struct SurfaceInteraction : public Interaction {
    Float2 uv;
    UVN<Float3> s_uvn;
    Float prim_area{0.f};
    Uint prim_id{InvalidUI32};
    Uint light_id{InvalidUI32};
    Uint mat_id{InvalidUI32};
    [[nodiscard]] Bool has_emission() const noexcept { return light_id != InvalidUI32; }
    [[nodiscard]] Bool has_material() const noexcept { return mat_id != InvalidUI32; }
    [[nodiscard]] Bool valid() const noexcept { return prim_id != InvalidUI32; }
};

struct SurfacePoint {
    Float3 pos;
    Float3 ng;
    SurfacePoint() = default;
    explicit SurfacePoint(const Float3 &p) : pos(p) {}
    SurfacePoint(const Float3 &p, const Float3 &n)
        : pos(p), ng(n) {}
    explicit SurfacePoint(const Interaction &it)
        : pos(it.pos), ng(it.g_uvn.normal()) {}
    explicit SurfacePoint(const SurfaceInteraction &it)
        : pos(it.pos), ng(it.g_uvn.normal()) {}

    [[nodiscard]] Float3 robust_pos(const Float3 &dir) const noexcept {
        Float factor = select(dot(ng, dir) > 0, 1.f, -1.f);
        return offset_ray_origin(pos, ng * factor);
    }

    [[nodiscard]] OCRay spawn_ray(const Float3 &dir) const noexcept {
        return vision::spawn_ray(pos, ng, dir);
    }
    [[nodiscard]] OCRay spawn_ray_to(const Float3 &p) const noexcept{
        return vision::spawn_ray_to(pos, ng, p);
    }
    [[nodiscard]] OCRay spawn_ray_to(const SurfacePoint &lsc) const noexcept{
        return vision::spawn_ray_to(pos, ng, lsc.pos, lsc.ng);
    }
};

struct GeometrySurfacePoint : public SurfacePoint {
    Float2 uv{};
    GeometrySurfacePoint() = default;
    explicit GeometrySurfacePoint(const Float3 &p) : SurfacePoint(p) {}
    explicit GeometrySurfacePoint(const Interaction &it, Float2 uv)
        : SurfacePoint(it), uv(uv) {}
    GeometrySurfacePoint(Float3 p, Float3 ng, Float2 uv)
        : SurfacePoint{p, ng}, uv(uv) {}
};

/**
 * A point on light
 * used to eval light PDF or lighting to LightSampleContext
 */
struct LightEvalContext : public GeometrySurfacePoint {
    Float PDF_pos{};
    LightEvalContext() = default;
    explicit LightEvalContext(const Float3 &p) : GeometrySurfacePoint(p) {}
    LightEvalContext(const GeometrySurfacePoint &gsp, Float PDF_pos)
        : GeometrySurfacePoint(gsp), PDF_pos(PDF_pos) {}
    LightEvalContext(Float3 p, Float3 ng, Float2 uv, Float PDF_pos)
        : GeometrySurfacePoint{p, ng, uv}, PDF_pos(PDF_pos) {}
    LightEvalContext(const SurfaceInteraction &si)
        : GeometrySurfacePoint{si, si.uv}, PDF_pos(1.f / si.prim_area) {}
};

struct LightSampleContext : public SurfacePoint {
    Float3 ns;
    LightSampleContext() = default;
    LightSampleContext(const Interaction &it)
        : SurfacePoint(it), ns(it.g_uvn.normal()) {}
    LightSampleContext(const SurfaceInteraction &it)
        : SurfacePoint(it), ns(it.s_uvn.normal()) {}
    LightSampleContext(Float3 p, Float3 ng, Float3 ns)
        : SurfacePoint{p, ng}, ns(ns) {}
};

struct TextureEvalContext {
    Float3 pos;
    Float2 uv;
    TextureEvalContext() = default;
    TextureEvalContext(const SurfaceInteraction &si)
        : pos(si.pos), uv(si.uv) {}
};

struct MaterialEvalContext : public TextureEvalContext {
    Float3 wo;
    Float3 ng, ns;
    Float3 dp_dus;
    MaterialEvalContext() = default;
    MaterialEvalContext(const SurfaceInteraction &si)
        : TextureEvalContext(si),
          wo(si.wo),
          ng(si.g_uvn.normal()),
          ns(si.s_uvn.normal()),
          dp_dus(si.s_uvn.dp_du()) {}
};

}// namespace vision