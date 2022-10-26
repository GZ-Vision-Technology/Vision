//
// Created by Zero on 04/09/2022.
//

#pragma once

#include "rhi/common.h"
#include "base/scene.h"
#include "base/device_data.h"
#include "rhi/window.h"

namespace vision {
using namespace ocarina;

class RenderPipeline {
private:
    Device *_device;
    vision::Context *_context;
    Scene _scene;
    DeviceData _device_data{_device};
    unique_ptr<float4[]> _render_buffer;
    Stream _stream;

public:
    RenderPipeline(Device *device, vision::Context *context);
    void init_scene(const SceneDesc &scene_desc) { _scene.init(scene_desc); }
    [[nodiscard]] const Device &device() const noexcept { return *_device; }
    [[nodiscard]] Device &device() noexcept { return *_device; }
    [[nodiscard]] DeviceData &device_data() noexcept { return _device_data; }
    [[nodiscard]] const DeviceData &device_data() const noexcept { return _device_data; }
    [[nodiscard]] vision::Context &context() noexcept { return *_context; }
    void prepare() noexcept;
    [[nodiscard]] Stream &stream() noexcept { return _stream; }
    void prepare_device_data() noexcept;
    void compile_shaders() noexcept;
    [[nodiscard]] uint2 resolution() const noexcept { return _scene.camera()->resolution(); }
    void download_result();
    [[nodiscard]] const float4 *buffer() const { return _render_buffer.get(); }
    void render(double dt);
};

}// namespace vision