//
// Created by Zero on 26/10/2022.
//

#pragma once

#include <iostream>
#include "core/cli_parser.h"
#include "descriptions/scene_desc.h"
#include "core/stl.h"
#include "core/context.h"
#include "util/image_io.h"
#include "core/logging.h"

namespace vision {
class App {
public:
    vision::Context context;
    Device device;
    Window::Wrapper window{nullptr, nullptr};
    SceneDesc scene_desc;
    RenderPipeline rp;

public:
    App(int argc, char *argv[])
        : context(argc, argv),
          device(context.create_device("cuda")),
          rp(context.create_pipeline(&device)) {
        device.init_rtx();
        context.clear_cache();
    }
    void prepare() noexcept;
    void update(double dt) noexcept;
    void register_event() noexcept;
    void on_key_event(int key, int action) noexcept;
    void on_mouse_event(int button, int action, float2 pos) noexcept;
    void on_scroll_event(float2 scroll) noexcept;
    void on_cursor_move(float2 pos) noexcept;
    int run() noexcept;
};
}// namespace vision