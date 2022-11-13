//
// Created by Zero on 10/09/2022.
//

#pragma once

#include <utility>
#include "core/stl.h"
#include "core/hash.h"
#include "core/basic_types.h"
#include "parameter_set.h"
#include "math/geometry.h"

namespace vision {

using namespace ocarina;
class Scene;
struct NodeDesc : public Hashable {
protected:
    string_view _type;

public:
    string sub_type;
    string name;
    mutable Scene *scene{nullptr};
    mutable fs::path scene_path;

protected:
    [[nodiscard]] uint64_t _compute_hash() const noexcept override {
        return hash64(_type, hash64(sub_type));
    }

public:
    NodeDesc() = default;
    NodeDesc(string_view type, string name)
        : _type(type), sub_type(std::move(name)) {}
    explicit NodeDesc(string_view type) : _type(type) {}
    virtual void init(const ParameterSet &ps) noexcept {
        if (ps.data().is_object())
            name = ps["name"].as_string();
    };
    [[nodiscard]] string plugin_name() const noexcept {
        return "vision-" + to_lower(string(_type)) + "-" + to_lower(sub_type);
    }
    [[nodiscard]] virtual bool operator==(const NodeDesc &other) const noexcept {
        return hash() == other.hash();
    }
};
#define VISION_DESC_COMMON(type)      \
    type##Desc() : NodeDesc(#type) {} \
    explicit type##Desc(string name) : NodeDesc(#type, std::move(name)) {}

struct TransformDesc : public NodeDesc {
public:
    float4x4 mat{make_float4x4(1.f)};

public:
    void init(const ParameterSet &ps) noexcept override;
};

struct TextureDesc : public NodeDesc {
public:
    float4 val;
    string fn;
    ColorSpace color_space;

public:
    VISION_DESC_COMMON(Texture)
    void init(const ParameterSet &ps) noexcept override;
    [[nodiscard]] bool valid_emission() const noexcept {
        return any(val != 0.f) || !fn.empty();
    }
    [[nodiscard]] uint64_t _compute_hash() const noexcept override {
        return hash64(NodeDesc::_compute_hash(), fn, val);
    }
};

struct LightDesc : public NodeDesc {
public:
    // common
    TextureDesc radiance;

    // area light
    bool two_sided{false};
    float scale{1.f};
    uint inst_id{InvalidUI32};

    // point light
    float3 position;
    float3 intensity{};

    VISION_DESC_COMMON(Light)
    void init(const ParameterSet &ps) noexcept override;
    [[nodiscard]] bool valid() const noexcept {
        return !sub_type.empty();
    }
};

struct ShapeDesc : public NodeDesc {
public:
    TransformDesc o2w;
    LightDesc emission;
    string material_name;
    uint mat_id{InvalidUI32};
    uint64_t mat_hash{InvalidUI32};
    uint index{InvalidUI32};
    fs::path fn;
    bool smooth{false};
    bool flip_uv{false};
    bool swap_handed{false};
    uint subdiv_level{0};

    // quad param
    float width{1};
    float height{1};

    // cube param
    float x{1}, y{1}, z{1};

    // sphere param
    float radius{1};
    uint sub_div{60};

    // mesh param
    mutable vector<Vertex> vertices;
    mutable vector<Triangle> triangles;

public:
    VISION_DESC_COMMON(Shape)
    void init(const ParameterSet &ps) noexcept override;
    [[nodiscard]] bool operator==(const ShapeDesc &other) const noexcept;
};

struct SamplerDesc : public NodeDesc {
public:
    uint spp{};

public:
    VISION_DESC_COMMON(Sampler)
    void init(const ParameterSet &ps) noexcept override;
};

struct FilterDesc : public NodeDesc {
public:
    float2 radius{make_float2(1.f)};
    // for gaussian filter
    float sigma{};
    // for sinc filter
    float tau{};
    // for mitchell filter
    float b{}, c{};

public:
    VISION_DESC_COMMON(Filter)
    void init(const ParameterSet &ps) noexcept override;
};

struct FilmDesc : public NodeDesc {
public:
    int state{0};
    int tone_map{0};
    uint2 resolution{make_uint2(500)};

public:
    VISION_DESC_COMMON(Film)
    void init(const ParameterSet &ps) noexcept override;
};

struct SensorDesc : public NodeDesc {
public:
    TransformDesc transform_desc;
    float fov_y{20};
    float velocity{5};
    float sensitivity{0.5};
    float focal_distance{5.f};
    float lens_radius{0.f};
    FilterDesc filter_desc;
    FilmDesc film_desc;

public:
    VISION_DESC_COMMON(Sensor)
    void init(const ParameterSet &ps) noexcept override;
};

struct IntegratorDesc : public NodeDesc {
public:
    uint max_depth{10};
    uint min_depth{5};
    float rr_threshold{1};

public:
    VISION_DESC_COMMON(Integrator)
    void init(const ParameterSet &ps) noexcept override;
};

struct MaterialDesc : public NodeDesc {
public:
    TextureDesc color;
    TextureDesc ior;
    TextureDesc roughness;

public:
    VISION_DESC_COMMON(Material)
    void init(const ParameterSet &ps) noexcept override;
    [[nodiscard]] uint64_t _compute_hash() const noexcept override {
        return hash64(NodeDesc::_compute_hash(), color, ior, roughness);
    }
};

struct LightSamplerDesc : public NodeDesc {
public:
    vector<LightDesc> light_descs;
    VISION_DESC_COMMON(LightSampler)
    void init(const ParameterSet &ps) noexcept override;
};

struct DistributionDesc : public NodeDesc {
public:
    VISION_DESC_COMMON(Distribution)
    void init(const ParameterSet &ps) noexcept override;
};

}// namespace vision