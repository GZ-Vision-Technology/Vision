//
// Created by Zero on 06/09/2022.
//

#pragma once

#include "descriptions/scene_desc.h"
#include "base/node.h"
#include "base/sensor/camera.h"
#include "base/sampler.h"
#include "base/shape.h"
#include "base/integrator.h"
#include "base/illumination/lightsampler.h"
#include "base/scattering/material.h"
#include "base/scattering/medium.h"
#include "base/warper.h"
#include "base/color/spectrum.h"

namespace vision {

using namespace ocarina;


#define MAKE_GETTER(member)                                          \
    [[nodiscard]] auto member() const noexcept { return _##member; } \
    [[nodiscard]] auto member() noexcept { return _##member; }

class Scene {
private:
    std::list<Node::Wrapper> _all_nodes;
    Box3f _aabb;
    Camera *_camera{nullptr};
    Sampler *_sampler{nullptr};
    Integrator *_integrator{nullptr};
    LightSampler *_light_sampler{nullptr};
    vector<Shape *> _shapes;
    Polymorphic<Material *> _materials;
    Polymorphic<Medium *> _mediums;
    WarperDesc _warper_desc;
    RenderSettingDesc _render_setting{};
    Spectrum *_spectrum{nullptr};
    Pipeline *_rp{nullptr};
    friend class Pipeline;

public:
    explicit Scene(Pipeline *rp);
    void init(const SceneDesc &scene_desc);
    void prepare() noexcept;
    [[nodiscard]] PolymorphicMode polymorphic_mode() const noexcept { return _render_setting.polymorphic_mode; }
    [[nodiscard]] Pipeline *pipeline() noexcept;
    MAKE_GETTER(integrator)
    MAKE_GETTER(camera)
    MAKE_GETTER(spectrum)
    MAKE_GETTER(sampler)
    MAKE_GETTER(light_sampler)
    [[nodiscard]] auto radiance_film() noexcept { return camera()->radiance_film(); }
    [[nodiscard]] auto radiance_film() const noexcept { return camera()->radiance_film(); }
    [[nodiscard]] const auto &materials() const noexcept { return _materials; }
    [[nodiscard]] auto &materials() noexcept { return _materials; }
    [[nodiscard]] const auto &mediums() const noexcept { return _mediums; }
    [[nodiscard]] Node *load_node(const NodeDesc &desc);

    [[nodiscard]] Slot create_slot(const SlotDesc &desc);
    template<typename T, typename desc_ty>
    [[nodiscard]] T *load(const desc_ty &desc) {
        desc.scene = this;
        auto ret = dynamic_cast<T *>(load_node(desc));
        OC_ERROR_IF(ret == nullptr, "error node load ", desc.name);
        return ret;
    }
    [[nodiscard]] uint light_num() const noexcept { return _light_sampler->light_num(); }
    void prepare_lights() noexcept;
    [[nodiscard]] Warper *load_warper() noexcept { return load<Warper>(_warper_desc); }
    [[nodiscard]] Warper2D *load_warper2d() noexcept {
        WarperDesc warper_desc = _warper_desc;
        warper_desc.sub_type += "2d";
        return load<Warper2D>(warper_desc);
    }
    [[nodiscard]] bool has_medium() const noexcept { return !_mediums.empty(); }
    void load_shapes(const vector<ShapeDesc> &descs);
    void load_mediums(const vector<MediumDesc> &descs);
    void load_materials(const vector<MaterialDesc> &material_descs);
    Light *load_light(const LightDesc &desc);
    ShaderNode *load_shader_node(const ShaderNodeDesc &desc);
    void prepare_materials();
    [[nodiscard]] float world_diameter() const noexcept { return _aabb.radius() * 2; }
    void upload_data() noexcept;
    [[nodiscard]] Shape *get_shape(uint id) noexcept { return _shapes[id]; }
};

#undef MAKE_GETTER

}// namespace vision