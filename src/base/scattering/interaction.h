//
// Created by Zero on 03/10/2022.
//

#pragma once

#include "core/basic_types.h"
#include "dsl/common.h"
#include "math/geometry.h"
#include "base/sample.h"

namespace vision {

using namespace ocarina;

template<typename T>
requires is_vector3_expr_v<T>
struct PartialDerivative : Frame<T, false> {
public:
    using vec_ty = T;

public:
    vec_ty dn_du;
    vec_ty dn_dv;

public:
    void set_frame(Frame<T> frame) {
        this->x = frame.x;
        this->y = frame.y;
        this->z = frame.z;
    }
    void update(const vec_ty &norm) noexcept {
        this->z = norm;
        this->x = normalize(cross(norm, this->y)) * length(this->x);
        this->y = normalize(cross(norm, this->x)) * length(this->y);
    }
    [[nodiscard]] vec_ty dp_du() const noexcept { return this->x; }
    [[nodiscard]] vec_ty dp_dv() const noexcept { return this->y; }
    [[nodiscard]] vec_ty normal() const noexcept { return this->z; }
    [[nodiscard]] boolean_t<T> valid() const noexcept { return nonzero(normal()); }
};

struct MediumInterface {
public:
    Uint inside{InvalidUI32};
    Uint outside{InvalidUI32};

public:
    MediumInterface() = default;
    MediumInterface(Uint in, Uint out) : inside(in), outside(out) {}
    explicit MediumInterface(Uint medium_id) : inside(medium_id), outside(medium_id) {}
    [[nodiscard]] Bool is_transition() const noexcept { return inside != outside; }
    [[nodiscard]] Bool has_inside() const noexcept { return inside != InvalidUI32; }
    [[nodiscard]] Bool has_outside() const noexcept { return outside != InvalidUI32; }
};

template<EPort p = D>
[[nodiscard]] Float phase_HG(Float cos_theta, Float g) {
    Float denom = 1 + sqr(g) + 2 * g * cos_theta;
    return Inv4Pi * (1 - sqr(g)) / (denom * sqrt(denom));
}

class Sampler;

class PhaseFunction {
protected:
    const SampledWavelengths *_swl{};

public:
    virtual void init(Float g, const SampledWavelengths &swl) noexcept = 0;
    [[nodiscard]] virtual Bool valid() const noexcept = 0;

    [[nodiscard]] virtual ScatterEval evaluate(Float3 wo, Float3 wi) const noexcept {
        Float val = f(wo, wi);
        return {{_swl->dimension(), val}, val, 0};
    }
    [[nodiscard]] virtual PhaseSample sample(Float3 wo, Sampler *sampler) const noexcept = 0;
    [[nodiscard]] virtual Float f(Float3 wo, Float3 wi) const noexcept = 0;
};

class HenyeyGreenstein : public PhaseFunction {
private:
    static constexpr float InvalidG = 10;
    Float _g{InvalidG};

public:
    HenyeyGreenstein() = default;
    void init(Float g, const SampledWavelengths &swl) noexcept override {
        _g = g;
        _swl = &swl;
    }
    [[nodiscard]] Float f(Float3 wo, Float3 wi) const noexcept override;
    [[nodiscard]] PhaseSample sample(Float3 wo, Sampler *sampler) const noexcept override;
    [[nodiscard]] Bool valid() const noexcept override { return InvalidG != _g; }
};

struct Interaction {
public:
    Float3 pos;
    Float3 wo;
    Float3 time;
    Float3 ng;

    Float2 uv;
    Float2 lightmap_uv;
    PartialDerivative<Float3> shading;
    Float prim_area{0.f};
    Uint prim_id{InvalidUI32};
    Uint lightmap_id{InvalidUI32};
    Float du_dx;
    Float du_dy;
    Float dv_dx;
    Float dv_dy;

private:
    Uint _mat_id{InvalidUI32};
    Uint _light_id{InvalidUI32};

public:
    // todo optimize volpt and pt
    MediumInterface mi;
    HenyeyGreenstein phase;

public:
    Interaction();
    Interaction(Float3 pos, Float3 wo);
    void init_phase(Float g, const SampledWavelengths &swl);
    [[nodiscard]] Bool has_phase();
    void set_medium(const Uint &inside, const Uint &outside);
    void set_material(const Uint &mat) noexcept { _mat_id = mat; }
    void set_light(const Uint &light) noexcept { _light_id = light; }
    [[nodiscard]] Bool has_emission() const noexcept { return _light_id != InvalidUI32; }
    [[nodiscard]] Bool has_material() const noexcept { return _mat_id != InvalidUI32; }
    [[nodiscard]] Bool has_lightmap() const noexcept { return lightmap_id != InvalidUI32; }
    [[nodiscard]] Uint material_inst_id() const noexcept;
    [[nodiscard]] Uint material_type_id() const noexcept;
    [[nodiscard]] Uint material_id() const noexcept { return _mat_id; }
    [[nodiscard]] Uint light_inst_id() const noexcept;
    [[nodiscard]] Uint light_type_id() const noexcept;
    [[nodiscard]] Uint light_id() const noexcept { return _light_id; }
    [[nodiscard]] Bool valid() const noexcept { return prim_id != InvalidUI32; }
    [[nodiscard]] OCRay spawn_ray(const Float3 &dir) const noexcept;
    [[nodiscard]] OCRay spawn_ray(const Float3 &dir, const Float &t) const noexcept;
    [[nodiscard]] RayState spawn_ray_state(const Float3 &dir) const noexcept;
    [[nodiscard]] RayState spawn_ray_state_to(const Float3 &p) const noexcept;
    [[nodiscard]] OCRay spawn_ray_to(const Float3 &p) const noexcept {
        return vision::spawn_ray_to(pos, ng, p);
    }
};

struct SpacePoint {
    Float3 pos;
    Float3 ng;
    SpacePoint() = default;
    explicit SpacePoint(const Float3 &p) : pos(p) {}
    SpacePoint(const Float3 &p, const Float3 &n)
        : pos(p), ng(n) {}
    explicit SpacePoint(const Interaction &it)
        : pos(it.pos), ng(it.ng) {}

    [[nodiscard]] Float3 robust_pos(const Float3 &dir) const noexcept {
        Float factor = select(dot(ng, dir) > 0, 1.f, -1.f);
        return offset_ray_origin(pos, ng * factor);
    }

    [[nodiscard]] OCRay spawn_ray(const Float3 &dir) const noexcept {
        return vision::spawn_ray(pos, ng, dir);
    }
    [[nodiscard]] OCRay spawn_ray_to(const Float3 &p) const noexcept {
        return vision::spawn_ray_to(pos, ng, p);
    }
    [[nodiscard]] OCRay spawn_ray_to(const SpacePoint &lsc) const noexcept {
        return vision::spawn_ray_to(pos, ng, lsc.pos, lsc.ng);
    }
};

struct GeometrySurfacePoint : public SpacePoint {
    Float2 uv{};
    GeometrySurfacePoint() = default;
    explicit GeometrySurfacePoint(const Float3 &p) : SpacePoint(p) {}
    explicit GeometrySurfacePoint(const Interaction &it, Float2 uv)
        : SpacePoint(it), uv(uv) {}
    GeometrySurfacePoint(Float3 p, Float3 ng, Float2 uv)
        : SpacePoint{p, ng}, uv(uv) {}
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
    LightEvalContext(const Interaction &it)
        : GeometrySurfacePoint{it, it.uv}, PDF_pos(1.f / it.prim_area) {}
};

struct LightSampleContext : public SpacePoint {
    Float3 ns;
    LightSampleContext() = default;
    LightSampleContext(const Interaction &it)
        : SpacePoint(it), ns(it.shading.normal()) {}
    LightSampleContext(Float3 p, Float3 ng, Float3 ns)
        : SpacePoint{p, ng}, ns(ns) {}
};

struct AttrEvalContext {
    Float3 pos;
    Float2 uv;
    AttrEvalContext() = default;
    AttrEvalContext(const Float3 &pos)
        : pos(pos) {}
    AttrEvalContext(const Interaction &it)
        : pos(it.pos), uv(it.uv) {}
    AttrEvalContext(const Float2 &uv)
        : uv(uv) {}
};

struct MaterialEvalContext : public AttrEvalContext {
    Float3 wo;
    Float3 ng, ns;
    Float3 dp_dus;
    MaterialEvalContext() = default;
    MaterialEvalContext(const Interaction &it)
        : AttrEvalContext(it),
          wo(it.wo),
          ng(it.ng),
          ns(it.shading.normal()),
          dp_dus(it.shading.dp_du()) {}
};

}// namespace vision