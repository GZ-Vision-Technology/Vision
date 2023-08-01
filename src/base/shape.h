//
// Created by Zero on 14/09/2022.
//

#pragma once

#include "core/stl.h"
#include "core/basic_types.h"
#include "dsl/rtx_type.h"
#include "node.h"
#include "scattering/medium.h"
#include "math/box.h"
#include "core/vs_header.h"
#include "base/scattering/material.h"
#include "base/illumination/light.h"

namespace vision {

struct Geometry;
struct Mesh;

struct Handle {
    // todo compress unsigned int data
    uint light_id{InvalidUI32};
    uint mat_id{InvalidUI32};
    uint lightmap_id{InvalidUI32};
    uint mesh_id{InvalidUI32};
    uint inside_medium{InvalidUI32};
    uint outside_medium{InvalidUI32};
    float4x4 o2w;
};

struct Shape : public Node {
public:
    using Desc = ShapeDesc;

public:
    Box3f aabb;

protected:
    Handle _handle;
    float _factor{};
    uint _index{};
    SP<Mesh> _mesh{};
    Wrap<IAreaLight> _emission{};
    Wrap<Material> _material{};
    Wrap<Medium> _inside{};
    Wrap<Medium> _outside{};

protected:
    void init_medium(const ShapeDesc &desc) noexcept;

public:
    explicit Shape(const ShapeDesc &desc);
    Shape() = default;
    virtual void fill_geometry(Geometry &data) const noexcept {
        OC_ASSERT(false);
        OC_ERROR("fill_geometry can not called by model");
    }
    OC_MAKE_MEMBER_GETTER_SETTER(index, )

#define VS_MAKE_ATTR_SETTER_GETTER(attr)                     \
    void set_##attr(decltype(_##attr.object) val) noexcept { \
        _##attr.object = val;                                \
    }                                                        \
    void set_##attr##_name(const string &name) noexcept {    \
        _##attr.name = name;                                 \
    }                                                        \
    void set_##attr(decltype(_##attr) val) noexcept {        \
        _##attr = val;                                       \
    }                                                        \
    [[nodiscard]] auto attr() const noexcept {               \
        return _##attr.object.get();                         \
    }                                                        \
    [[nodiscard]] auto attr() noexcept {                     \
        return _##attr.object.get();                         \
    }                                                        \
    [[nodiscard]] string attr##_name() const noexcept {      \
        return _##attr.name;                                 \
    }

    void set_mesh(SP<Mesh> mesh) noexcept { _mesh = mesh; }
    VS_MAKE_ATTR_SETTER_GETTER(inside)
    VS_MAKE_ATTR_SETTER_GETTER(outside)
    VS_MAKE_ATTR_SETTER_GETTER(material)
    VS_MAKE_ATTR_SETTER_GETTER(emission)

#undef VS_MAKE_ATTR_SETTER_GETTER

    void set_o2w(float4x4 o2w) noexcept { _handle.o2w = o2w; }
    virtual void update_inside_medium_id(uint id) noexcept { _handle.inside_medium = id; }
    virtual void update_outside_medium_id(uint id) noexcept { _handle.outside_medium = id; }
    virtual void update_material_id(uint id) noexcept { _handle.mat_id = id; }
    virtual void update_light_id(uint id) noexcept { _handle.light_id = id; }
    [[nodiscard]] bool has_material() const noexcept { return _material.object.get(); }
    [[nodiscard]] bool has_inside_medium() const noexcept { return _inside.object.get(); }
    [[nodiscard]] bool has_outside_medium() const noexcept { return _outside.object.get(); }
    [[nodiscard]] bool has_lightmap() const noexcept { return _handle.lightmap_id != InvalidUI32; }
    [[nodiscard]] const Medium *inside_medium() const noexcept { return _inside.object.get(); }
    [[nodiscard]] const Medium *outside_medium() const noexcept { return _outside.object.get(); }
    [[nodiscard]] virtual vector<float> ref_surface_areas() const noexcept {
        OC_ASSERT(false);
        OC_ERROR("ref_surface_areas can not called by model");
    }
    [[nodiscard]] virtual uint lightmap_size() const noexcept;
    OC_MAKE_MEMBER_GETTER_SETTER(handle, &)
    void set_lightmap_id(uint id) noexcept {
        _handle.lightmap_id = id;
    }
    [[nodiscard]] bool has_emission() const noexcept { return _emission.object.get(); }
    [[nodiscard]] virtual vector<float> surface_areas() const noexcept {
        OC_ASSERT(false);
        OC_ERROR("surface_areas can not called by model");
    }
    virtual void for_each_mesh(const std::function<void(SP<Mesh>, uint)> &func) noexcept {
        OC_ASSERT(false);
        OC_ERROR("surface_areas can not called by model");
    }
    virtual void for_each_mesh(const std::function<void(SP<const Mesh>, uint)> &func) const noexcept {
        OC_ASSERT(false);
        OC_ERROR("surface_areas can not called by model");
    }
    [[nodiscard]] virtual float4x4 o2w() const noexcept { return _handle.o2w; }
};

}// namespace vision

OC_STRUCT(vision::Handle, light_id, mat_id, lightmap_id,
          mesh_id, inside_medium, outside_medium, o2w){};

namespace vision {
struct Mesh : public Shape {
public:
    struct Handle {
        uint vertex_offset;
        uint triangle_offset;
    };

public:
    vector<Vertex> vertices;
    vector<Triangle> triangles;

protected:
    [[nodiscard]] uint64_t _compute_hash() const noexcept override;

public:
    explicit Mesh(const ShapeDesc &desc);
    Mesh(vector<Vertex> vert, vector<Triangle> tri)
        : vertices(std::move(vert)), triangles(std::move(tri)) {}
    Mesh() = default;
    [[nodiscard]] Box3f compute_aabb() const noexcept;
    void init_aabb() noexcept { aabb = compute_aabb(); }
    void fill_geometry(Geometry &data) const noexcept override;
    [[nodiscard]] vector<float> surface_areas() const noexcept override;
    [[nodiscard]] vector<float> ref_surface_areas() const noexcept override;
};

}// namespace vision

OC_STRUCT(vision::Mesh::Handle, vertex_offset, triangle_offset){};

namespace vision {

class Group : public Node {
public:
    using Desc = ShapeDesc;

protected:
    vector<SP<Mesh>> _meshes;
    Box3f _aabb;
    float4x4 _o2w;

public:
    explicit Group(const ShapeDesc &desc)
        : Node(desc), _o2w(desc.o2w.mat) {}
    OC_MAKE_MEMBER_GETTER(o2w, )
    [[nodiscard]] Mesh &mesh_at(ocarina::uint i) noexcept {
        return *_meshes[i];
    }

    [[nodiscard]] const Mesh &mesh_at(ocarina::uint i) const noexcept {
        return *_meshes[i];
    }

    void for_each_mesh(const std::function<void(SP<Mesh>, uint)> &func) noexcept {
        for (uint i = 0; i < _meshes.size(); ++i) {
            func(_meshes[i], i);
        }
    }

    void for_each_mesh(const std::function<void(SP<const Mesh>, uint)> &func) const noexcept {
        for (uint i = 0; i < _meshes.size(); ++i) {
            func(_meshes[i], i);
        }
    }
};

}// namespace vision