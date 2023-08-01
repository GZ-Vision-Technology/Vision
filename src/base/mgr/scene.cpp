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
    _spectrum = load<Spectrum>(scene_desc.spectrum_desc);
    load_materials(scene_desc.material_descs);
    load_mediums(scene_desc.mediums_desc);
    _camera = load<Camera>(scene_desc.sensor_desc);
    load_shapes(scene_desc.shape_descs);
    remove_unused_materials();
    tidy_up();
    fill_mesh_data();
    _integrator = load<Integrator>(scene_desc.integrator_desc);
    _sampler = load<Sampler>(scene_desc.sampler_desc);
}

void Scene::tidy_up() noexcept {
    _light_sampler->tidy_up();
    tidy_up_materials();
    tidy_up_mediums();
    tidy_up_meshes();
}

void Scene::tidy_up_materials() noexcept {
    _materials.for_each_instance([&](SP<Material> material, uint i) {
        material->set_index(i);
    });
}

void Scene::tidy_up_mediums() noexcept {
    _mediums.for_each_instance([&](SP<Medium> medium, uint i) {
        medium->set_index(i);
    });
}

void Scene::tidy_up_meshes() noexcept {
    for (uint i = 0; i < _meshes.size(); ++i) {
        _meshes[i]->set_index(i);
    }
}

Slot Scene::create_slot(const SlotDesc &desc) {
    return Global::node_mgr().create_slot(desc);
}

SP<Material> Scene::obtain_black_body() noexcept {
    if (!_black_body) {
        MaterialDesc md;
        md.sub_type = "black_body";
        _black_body = load<Material>(md);
        _materials.push_back(_black_body);
    }
    return _black_body;
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
        SP<Group> group = load<Group>(desc);
        _groups.push_back(group);
        group->for_each_mesh([&](SP<Mesh> mesh, uint i) {
            float4x4 o2w = group->o2w();

            auto iter = std::find_if(_materials.begin(), _materials.end(), [&](SP<Material> &material) {
                return material->name() == mesh->_material.name;
            });
            if (iter != _materials.end() && !mesh->has_material()) {
                mesh->_material.object = *iter;
            }
            if (desc.emission.valid()) {
                desc.emission.set_value("inst_id", _meshes.size());
                SP<IAreaLight> light = load_light<IAreaLight>(desc.emission);
                mesh->set_emission(light);
                light->set_mesh(mesh.get());
            }
            if (has_medium()) {
                auto iter = std::find_if(_mediums.begin(), _mediums.end(), [&](SP<Medium> &medium) {
                    return medium->name() == mesh->_inside.name;
                });
                if (iter != _mediums.end()) {
                    mesh->_inside.object = *iter;
                }
                iter = std::find_if(_mediums.begin(), _mediums.end(), [&](SP<Medium> &medium) {
                    return medium->name() == mesh->_outside.name;
                });
                if (iter != _mediums.end()) {
                    mesh->_outside.object = *iter;
                }
            }
            _aabb.extend(mesh->aabb);
            _meshes.push_back(mesh);
        });
    }
}

void Scene::fill_mesh_data() {
    for (SP<Mesh> mesh : _meshes) {
        if (mesh->has_material()) {
            uint mat_index = _materials.get_index([&](SP<Material> mat) {
                return mat.get() == mesh->_material.object.get();
            });
            const Material *material = _materials[mat_index].get();
            mesh->update_material_id(_materials.encode_id(mat_index, material));
        }
        if (mesh->has_emission()) {
            uint lit_index = _light_sampler->lights().get_index([&](SP<Light> light) {
                return light.get() == mesh->_emission.object.get();
            });
            mesh->update_light_id(_light_sampler->lights().encode_id(lit_index, mesh->_emission.object.get()));
        }
        if (has_medium()) {
            if (mesh->has_inside_medium()) {
                mesh->update_inside_medium_id(mesh->_inside.object->index());
            }
            if (mesh->has_outside_medium()) {
                mesh->update_outside_medium_id(mesh->_outside.object->index());
            }
        }
    }
}

void Scene::remove_unused_materials() {
    for (auto iter = _materials.begin(); iter != _materials.end();) {
        if (iter->use_count() == 1) {
            iter = _materials.erase(iter);
        } else {
            ++iter;
        }
    }
    tidy_up_materials();
}

void Scene::load_mediums(const MediumsDesc &md) {
    _global_medium.name = md.global;
    for (uint i = 0; i < md.mediums.size(); ++i) {
        const MediumDesc &desc = md.mediums[i];
        auto medium = load<Medium>(desc);
        medium->set_index(i);
        _mediums.push_back(medium);
    }
    uint index = _mediums.get_index([&](const SP<Medium> &medium) {
        return medium->name() == _global_medium.name;
    });
    if (index != InvalidUI32) {
        _global_medium.object = _mediums[index];
    }
}

void Scene::prepare_materials() {
    _materials.for_each_instance([&](const SP<Material> &material) noexcept {
        material->prepare();
    });
    auto rp = pipeline();
    _materials.prepare(rp->resource_array(), rp->device());
}

void Scene::upload_data() noexcept {
    _camera->update_device_data();
}

}// namespace vision