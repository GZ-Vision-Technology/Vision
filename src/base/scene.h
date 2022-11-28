//
// Created by Zero on 06/09/2022.
//

#pragma once

#include "descriptions/scene_desc.h"
#include "base/node.h"
#include "base/sensor.h"
#include "base/sampler.h"
#include "base/shape.h"
#include "base/integrator.h"
#include "base/lightsampler.h"
#include "base/material.h"
#include "base/warper.h"

namespace vision {

using namespace ocarina;

class Context;

#define MAKE_GETTER(member)                                          \
    [[nodiscard]] auto member() const noexcept { return _##member; } \
    [[nodiscard]] auto member() noexcept { return _##member; }

class Scene {
private:
    vision::Context *_context{nullptr};
    vector<Node::Wrapper> _all_nodes;
    Box3f _aabb;
    Camera *_camera{nullptr};
    Sampler *_sampler{nullptr};
    Integrator *_integrator{nullptr};
    LightSampler *_light_sampler{nullptr};
    vector<Shape *> _shapes;
    Polymorphic<Material *> _materials;
    WarperDesc _warper_desc;
    RenderPipeline *_rp{nullptr};
    friend class RenderPipeline;

public:
    explicit Scene(vision::Context *ctx, RenderPipeline *rp);
    void init(const SceneDesc &scene_desc);
    void prepare(RenderPipeline *rp) noexcept;
    [[nodiscard]] RenderPipeline *render_pipeline() noexcept;
    MAKE_GETTER(integrator)
    MAKE_GETTER(camera)
    MAKE_GETTER(sampler)
    MAKE_GETTER(light_sampler)
    [[nodiscard]] auto film() noexcept { return camera()->film(); }
    [[nodiscard]] auto film() const noexcept { return camera()->film(); }
    [[nodiscard]] const auto& materials() const noexcept { return _materials; }
    [[nodiscard]] Node *load_node(const NodeDesc &desc);
    template<typename T, typename desc_ty>
    [[nodiscard]] T *load(const desc_ty &desc) noexcept {
        desc.scene = this;
        auto ret = dynamic_cast<T *>(load_node(desc));
        OC_ERROR_IF(ret == nullptr, "error node load ", desc.name);
        return ret;
    }
    [[nodiscard]] uint light_num() const noexcept { return _light_sampler->light_num(); }
    void build_warpers(RenderPipeline *rp) noexcept;
    [[nodiscard]] Warper *load_warper() noexcept { return load<Warper>(_warper_desc); }
    void load_shapes(const vector<ShapeDesc> &descs) noexcept;
    void load_materials(const vector<MaterialDesc> &material_descs) noexcept;
    Light *load_light(const LightDesc &desc) noexcept;
    void load_lights(const vector<LightDesc> &descs) noexcept;
    void upload_data() noexcept;
    [[nodiscard]] Shape *get_shape(uint id) noexcept { return _shapes[id]; }
};

}// namespace vision