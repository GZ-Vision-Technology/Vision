//
// Created by Zero on 04/09/2022.
//

#include "render_pipeline.h"
#include "base/sensor.h"
#include "base/scene.h"
#include "core/context.h"

namespace vision {

RenderPipeline::RenderPipeline(Device *device, vision::Context *context)
    : _device(device),
      _context(context),
      _scene(context, this),
      _geometry(device),
      _stream(device->create_stream()) {}

void RenderPipeline::download_result() {
    _scene.film()->copy_to(_render_image.pixel_ptr());
}

void RenderPipeline::prepare_device_data() noexcept {
    for (const Shape *shape : _scene._shapes) {
        shape->fill_geometry(_geometry);
    }
    _geometry.reset_device_buffer();
    _geometry.build_meshes();
    _geometry.upload();
    _geometry.build_accel();
}

void RenderPipeline::compile_shaders() noexcept {
    _scene.integrator()->compile_shader(this);
}

void RenderPipeline::prepare() noexcept {
    _scene.prepare(this);
    prepare_device_data();
    compile_shaders();
    _render_image = ImageIO::pure_color(make_float4(0, 0, 0, 1), ColorSpace::LINEAR, resolution());
}

void RenderPipeline::render(double dt) noexcept {
    Clock clk;
    _scene.integrator()->render(this);
    double ms = clk.elapse_ms();
    _total_time += ms;
    ++_frame_index;
    cout << ms << "  " << _total_time / _frame_index << "  " << _frame_index << endl;
}

OCHit RenderPipeline::trace_closest(const OCRay &ray) const noexcept {
    return geometry().accel.trace_closest(ray);
}

Bool RenderPipeline::trace_any(const OCRay &ray) const noexcept {
    return geometry().accel.trace_any(ray);
}

SurfaceInteraction RenderPipeline::compute_surface_interaction(const OCHit &hit) const noexcept {
    return geometry().compute_surface_interaction(hit, true);
}

LightEvalContext RenderPipeline::compute_light_eval_context(const Uint &inst_id, const Uint &prim_id, const Float2 &bary) const noexcept {
    return geometry().compute_light_eval_context(inst_id, prim_id, bary);
}
}// namespace vision