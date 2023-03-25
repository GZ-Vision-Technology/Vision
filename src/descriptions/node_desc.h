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

struct NameID {
public:
    using map_ty = map<string, uint>;

public:
    string name;
    uint id{InvalidUI32};
    bool valid() { return id != InvalidUI32; }
    void fill_id(const map_ty &name_to_id) {
        if (name_to_id.contains(name)) {
            id = name_to_id.at(name);
        } else {
            id = InvalidUI32;
        }
    }
};

enum ShaderNodeType {
    Number,
    Albedo,
    Unbound,
    Illumination,
    Calculate
};

struct NodeDesc : public Hashable {
protected:
    string_view _type;
    ParameterSet _parameter{DataWrap::object()};

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
    [[nodiscard]] string parameter_string() const noexcept;
    [[nodiscard]] ParameterSet operator[](const string &key) const noexcept { return _parameter[key]; }
    template<typename... Args>
    void set_value(Args &&...args) noexcept {
        _parameter.set_value(OC_FORWARD(args)...);
    }
    void set_parameter(const ParameterSet &ps) noexcept;
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

struct ShaderNodeDesc : public NodeDesc {
public:
    ShaderNodeType type{};

protected:
    [[nodiscard]] uint64_t _compute_hash() const noexcept override {
        return hash64(NodeDesc::_compute_hash(), parameter_string());
    }

public:
    ShaderNodeDesc() = default;
    explicit ShaderNodeDesc(ShaderNodeType type)
        : NodeDesc("ShaderNode"), type(type) {
        sub_type = "constant";
        _parameter.set_json(DataWrap::object());
    }
    explicit ShaderNodeDesc(string name, ShaderNodeType type)
        : NodeDesc("ShaderNode", std::move(name)), type(type) {
        sub_type = "constant";
        _parameter.set_json(DataWrap::object());
    }
    explicit ShaderNodeDesc(float v, ShaderNodeType type)
        : NodeDesc("ShaderNode"), type(type) {
        sub_type = "constant";
        _parameter.set_json(DataWrap::object());
        _parameter.set_value("value", {v, v, v, v});
    }
    explicit ShaderNodeDesc(float2 v, ShaderNodeType type)
        : NodeDesc("ShaderNode"), type(type) {
        sub_type = "constant";
        _parameter.set_json(DataWrap::object());
        _parameter.set_value("value", {v.x, v.y, 0, 0});
    }
    explicit ShaderNodeDesc(float3 v, ShaderNodeType type)
        : NodeDesc("ShaderNode"), type(type) {
        sub_type = "constant";
        _parameter.set_json(DataWrap::object());
        _parameter.set_value("value", {v.x, v.y, v.z, 0});
    }
    explicit ShaderNodeDesc(float4 v, ShaderNodeType type)
        : NodeDesc("ShaderNode"), type(type) {
        sub_type = "constant";
        _parameter.set_json(DataWrap::object());
        _parameter.set_value("value", {v.x, v.y, v.z, v.w});
    }
    void init(const ParameterSet &ps) noexcept override;
    void init(const ParameterSet &ps, fs::path scene_path) noexcept {
        this->scene_path = scene_path;
        init(ps);
    }
};

template<uint Dim>
requires(Dim <= 4) struct TSlotDesc : public NodeDesc {
public:
    static constexpr auto default_channels() noexcept {
        if constexpr (Dim == 1) {
            return "x";
        } else if constexpr (Dim == 2) {
            return "xy";
        } else if constexpr (Dim == 3) {
            return "xyz";
        } else {
            return "xyzw";
        }
    }

public:
    string channels;
    ShaderNodeDesc node;
    VISION_DESC_COMMON(TSlot)
    explicit TSlotDesc(ShaderNodeDesc node, string channels = default_channels())
        : node(node), channels(channels) {}

    explicit TSlotDesc(ShaderNodeType type, string channels = default_channels())
        : node(type), channels(channels) {}

    void init(const ParameterSet &ps) noexcept override {
        DataWrap data = ps.data();
        if (data.contains("channels")) {
            channels = ps["channels"].as_string();
            node.init(ps["node"], scene_path);
        } else {
            node.init(ps, scene_path);
        }
    }
    void init(const ParameterSet &ps, fs::path scene_path) noexcept {
        this->scene_path = scene_path;
        init(ps);
    }
};

struct SlotDesc : public NodeDesc {
public:
    [[nodiscard]] static string default_channels(uint dim) noexcept {
        switch (dim) {
            case 1: return "x";
            case 2: return "xy";
            case 3: return "xyz";
            case 4: return "xyzw";
        }
        OC_ASSERT(0);
        return "";
    }
    string channels;
    ShaderNodeDesc node;
    VISION_DESC_COMMON(Slot)
    SlotDesc(ShaderNodeDesc node, uint dim)
        : node(node), channels(default_channels(dim)) {}

    SlotDesc(ShaderNodeType type, uint dim)
        : node(type), channels(default_channels(dim)) {}

    void init(const ParameterSet &ps) noexcept override {
        DataWrap data = ps.data();
        if (data.contains("channels")) {
            channels = ps["channels"].as_string();
            node.init(ps["node"], scene_path);
        } else {
            node.init(ps, scene_path);
        }
    }
    void init(const ParameterSet &ps, fs::path scene_path) noexcept {
        this->scene_path = scene_path;
        init(ps);
    }
};

struct LightDesc : public NodeDesc {
public:
    TSlotDesc<3> color_slot{Illumination};
    SlotDesc color{Illumination, 3};
    TransformDesc o2w;

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
    NameID material;
    NameID inside_medium;
    NameID outside_medium;
    uint64_t mat_hash{InvalidUI32};
    uint index{InvalidUI32};

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
    VISION_DESC_COMMON(Sampler)
    void init(const ParameterSet &ps) noexcept override;
};

struct FilterDesc : public NodeDesc {
public:
    VISION_DESC_COMMON(Filter)
    void init(const ParameterSet &ps) noexcept override;
};

struct FilmDesc : public NodeDesc {
public:
    VISION_DESC_COMMON(Film)
    void init(const ParameterSet &ps) noexcept override;
};

struct SensorDesc : public NodeDesc {
public:
    TransformDesc transform_desc;
    FilterDesc filter_desc;
    FilmDesc film_desc;
    NameID medium;

public:
    VISION_DESC_COMMON(Sensor)
    void init(const ParameterSet &ps) noexcept override;
};

struct IntegratorDesc : public NodeDesc {
public:
    VISION_DESC_COMMON(Integrator)
    void init(const ParameterSet &ps) noexcept override;
};

struct MediumDesc : public NodeDesc {
public:
    ShaderNodeDesc sigma_a{Unbound};
    ShaderNodeDesc sigma_s{Unbound};
    ShaderNodeDesc g{Number};
    ShaderNodeDesc scale{Number};

public:
    VISION_DESC_COMMON(Medium)
    void init(const ParameterSet &ps) noexcept override;
};

struct MaterialDesc : public NodeDesc {
public:
    VISION_DESC_COMMON(Material)
    void init(const ParameterSet &ps) noexcept override;
    [[nodiscard]] uint64_t _compute_hash() const noexcept override;
    template<uint Dim>
    [[nodiscard]] TSlotDesc<Dim> tslot(const string &key, auto default_value,
                                       ShaderNodeType type = ShaderNodeType::Number) const noexcept {
        ShaderNodeDesc node{default_value, type};
        TSlotDesc<Dim> slot_desc{node};
        slot_desc.init(_parameter[key], scene_path);
        return slot_desc;
    }

    template<typename T>
    [[nodiscard]] SlotDesc slot(const string &key, T default_value,
                                ShaderNodeType type = ShaderNodeType::Number) const noexcept {
        ShaderNodeDesc node{default_value, type};
        SlotDesc slot_desc{node, type_dimension_v<T>};
        slot_desc.init(_parameter[key], scene_path);
        return slot_desc;
    }
};

struct LightSamplerDesc : public NodeDesc {
public:
    vector<LightDesc> light_descs;
    VISION_DESC_COMMON(LightSampler)
    void init(const ParameterSet &ps) noexcept override;
};

struct WarperDesc : public NodeDesc {
public:
    VISION_DESC_COMMON(Warper)
    void init(const ParameterSet &ps) noexcept override;
};

struct SpectrumDesc : public NodeDesc {
public:
    VISION_DESC_COMMON(Spectrum)
    void init(const ParameterSet &ps) noexcept override;
};

struct OutputDesc : public NodeDesc {
public:
    string fn;
    uint spp{0u};
    bool save_exit{false};
    VISION_DESC_COMMON(Output)
    void init(const ParameterSet &ps, fs::path scene_path) noexcept {
        this->scene_path = move(scene_path);
        init(ps);
    }
    void init(const ParameterSet &ps) noexcept override;
};

}// namespace vision