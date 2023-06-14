//
// Created by Zero on 04/09/2022.
//

#include "pipeline.h"
#include "base/sensor/sensor.h"
#include "scene.h"
#include "base/color/spectrum.h"

namespace vision {

Pipeline::Pipeline(Device *device)
    : _device(device),
      _geometry(this),
      _stream(device->create_stream()),
      _resource_array(device->create_resource_array()) {
    Printer::instance().init(*device);
}

Pipeline::Pipeline(const vision::PipelineDesc &desc)
    : Node(desc),
      _device(desc.device),
      _geometry(this),
      _stream(device().create_stream()),
      _resource_array(device().create_resource_array()) {
    Printer::instance().init(device());
}

void Pipeline::init_postprocessor(const SceneDesc &scene_desc) {
    _postprocessor.set_denoiser(_scene.load<Denoiser>(scene_desc.denoiser_desc));
    _postprocessor.set_tone_mapper(_scene.camera()->radiance_film()->tone_mapper());
}

void Pipeline::change_resolution(uint2 res) noexcept {
    auto film = _scene.camera()->radiance_film();
    film->set_resolution(res);
    film->prepare();
}

void Pipeline::prepare_geometry() noexcept {
    for (const Shape *shape : _scene._shapes) {
        shape->fill_geometry(_geometry);
    }
    _geometry.reset_device_buffer();
    _geometry.build_meshes();
    _geometry.upload();
    _geometry.build_accel();
}

void Pipeline::prepare_resource_array() noexcept {
    _resource_array.prepare_slotSOA(device());
    _stream << _resource_array->upload_buffer_handles()
            << _resource_array->upload_texture_handles()
            << synchronize() << commit();
}

Spectrum &Pipeline::spectrum() noexcept {
    return *_scene.spectrum();
}

const Spectrum &Pipeline::spectrum() const noexcept {
    return *_scene.spectrum();
}

void Pipeline::deregister_buffer(handle_ty index) noexcept {
    _resource_array->remove_buffer(index);
}

void Pipeline::deregister_texture(handle_ty index) noexcept {
    _resource_array->remove_texture(index);
}

void Pipeline::compile_shaders() noexcept {
    _scene.integrator()->compile_shader();
}

void Pipeline::prepare() noexcept {
    auto pixel_num = resolution().x * resolution().y;
    _final_picture.reset_all(device(), pixel_num);
    _scene.prepare();
    image_pool().prepare();
    prepare_geometry();
    prepare_resource_array();
    compile_shaders();
}

void Pipeline::render(double dt) noexcept {
    Clock clk;
    _scene.integrator()->render();
    double ms = clk.elapse_ms();
    _total_time += ms;
    ++_frame_index;
    cerr << ms << "  " << _total_time / _frame_index << "  " << _frame_index << endl;
    Printer::instance().retrieve_immediately();
}

float4 *Pipeline::final_picture() noexcept {
    RegistrableManaged<float4> &original = _scene.radiance_film()->original_buffer();
    _postprocessor.denoise(resolution(), &_final_picture, &original, nullptr, nullptr);
    _postprocessor.tone_mapping(_final_picture, _final_picture);
    _final_picture.download_immediately();
    return _final_picture.data();
}

OCHit Pipeline::trace_closest(const OCRay &ray) const noexcept {
    return geometry().accel.trace_closest(ray);
}

Bool Pipeline::trace_any(const OCRay &ray) const noexcept {
    return geometry().accel.trace_any(ray);
}

Interaction Pipeline::compute_surface_interaction(const OCHit &hit, OCRay &ray) const noexcept {
    return geometry().compute_surface_interaction(hit, ray);
}

LightEvalContext Pipeline::compute_light_eval_context(const Uint &inst_id, const Uint &prim_id, const Float2 &bary) const noexcept {
    return geometry().compute_light_eval_context(inst_id, prim_id, bary);
}
}// namespace vision