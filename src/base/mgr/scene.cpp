//
// Created by Zero on 06/09/2022.
//

#include "scene.h"
#include "rhi/dynamic_module.h"
#include "pipeline.h"
#include "base/scattering/interaction.h"

namespace vision {

Pipeline *Scene::pipeline() noexcept { return Global::instance().pipeline(); }

void Scene::init(const SceneDesc &scene_desc) {
    TIMER(init_scene);
    _warper_desc = scene_desc.warper_desc;
    _render_setting = scene_desc.render_setting;
    _materials.set_mode(_render_setting.polymorphic_mode);
    OC_INFO_FORMAT("polymorphic mode is {}", _materials.mode());
    _light_sampler = load<LightSampler>(scene_desc.light_sampler_desc);
    _light_sampler->set_mode(_render_setting.polymorphic_mode);
    _camera = load<Camera>(scene_desc.sensor_desc);
    _spectrum = load<Spectrum>(scene_desc.spectrum_desc);
    load_materials(scene_desc.material_descs);
    load_shapes(scene_desc.shape_descs);
    load_mediums(scene_desc.mediums_desc.mediums);
    _integrator = load<Integrator>(scene_desc.integrator_desc);
    _sampler = load<Sampler>(scene_desc.sampler_desc);
}

Slot Scene::create_slot(const SlotDesc &desc) {
    return Global::node_mgr().create_slot(desc);
}

void Scene::prepare() noexcept {
    _camera->prepare();
    _sampler->prepare();
    _integrator->prepare();
    _camera->update_device_data();
    prepare_lights();
    prepare_materials();
    pipeline()->spectrum().prepare();
}

void Scene::prepare_lights() noexcept {
    _light_sampler->prepare();
    auto &light = _light_sampler->lights();
    OC_INFO_FORMAT("This scene contains {} light types with {} light instances",
                   light.type_num(),
                   light.all_instance_num());
}

void Scene::load_materials(const vector<MaterialDesc> &material_descs) {
    for (const MaterialDesc &desc : material_descs) {
        _materials.push_back(load<Material>(desc));
    }
    OC_INFO_FORMAT("This scene contains {} material types with {} material instances",
                   _materials.type_num(),
                   _materials.all_instance_num());
}

void Scene::load_shapes(const vector<ShapeDesc> &descs) {
    for (const auto &desc : descs) {
        if (desc.emission.valid()) {
            desc.emission.set_value("inst_id", _shapes.size());
        }
        Shape *shape = const_cast<Shape *>(load<Shape>(desc));
        shape->for_each_mesh([&](vision::Mesh &mesh, uint i) {
            if (mesh.has_material()) {
                const Material *material = _materials[mesh.handle().mat_id];
                mesh.update_material_id(_materials.encode_id(mesh.handle().mat_id, material));
            }
            if (mesh.has_emission()) {
                const Light *light = _light_sampler->lights()[mesh.handle().light_id];
                mesh.update_light_id(_light_sampler->lights().encode_id(mesh.handle().light_id, light));
            }
            _aabb.extend(mesh.aabb);
            _shapes.push_back(&mesh);
        });

//        if (shape->has_material()) {
//            const Material *material = _materials[shape->handle().mat_id];
//            shape->update_material_id(_materials.encode_id(shape->handle().mat_id, material));
//        }
//        if (shape->has_emission()) {
//            const Light *light = _light_sampler->lights()[shape->handle().light_id];
//            shape->update_light_id(_light_sampler->lights().encode_id(shape->handle().light_id, light));
//        }
//        _aabb.extend(shape->aabb);
//        _shapes.push_back(shape);
    }
}

void Scene::load_mediums(const vector<MediumDesc> &descs) {
    for (const MediumDesc &desc : descs) {
        auto medium = load<Medium>(desc);
        _mediums.push_back(medium);
    }
}

Light *Scene::load_light(const LightDesc &desc) {
    OC_ASSERT(_light_sampler != nullptr);
    auto ret = load<Light>(desc);
    _light_sampler->add_light(ret);
    return ret;
}

void Scene::prepare_materials() {
    _materials.for_each_instance([&](const Material *material) noexcept {
        const_cast<Material *>(material)->prepare();
    });
    auto rp = pipeline();
    _materials.prepare(rp->resource_array(), rp->device());
}

void Scene::upload_data() noexcept {
    _camera->update_device_data();
}

}// namespace vision