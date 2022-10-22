//
// Created by Zero on 04/09/2022.
//

#pragma once

#include "rhi/common.h"
#include "base/scene.h"
#include "rhi/window.h"

namespace vision {
using namespace ocarina;
class RenderPipeline {
private:
    Device *_device;
    vision::Context *_context;
    Scene _scene;
    unique_ptr<float4[]> _view_buffer;

public:
    RenderPipeline(Device *device, vision::Context *context);
    void init_scene(const SceneDesc &scene_desc) { _scene.init(scene_desc); }
    [[nodiscard]] const Device &device() const noexcept { return *_device; }
    [[nodiscard]] Device &device() noexcept { return *_device; }
    [[nodiscard]] vision::Context &context() noexcept { return *_context; }
    void prepare() noexcept;
    [[nodiscard]] uint2 resolution() const noexcept { return _scene.camera()->resolution(); }
    void download_result(void *host_ptr);
    void build_accel();
    void render();
};

}// namespace vision