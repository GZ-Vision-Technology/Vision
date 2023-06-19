//
// Created by Zero on 2023/6/14.
//

#pragma once

#include "base/mgr/pipeline.h"
#include "base/bake_utlis.h"
#include "expander.h"

namespace vision {

class BakerPipeline : public Pipeline {
private:
    UVSpreader *_uv_spreader{};
    Rasterizer *_rasterizer{};
    UP<Expander> _expander;
    vector<BakedShape> _baked_shapes;
    using transform_signature = void(Buffer<float4>,
                                     Buffer<float4>,
                                     float4x4 o2w);
    Shader<transform_signature> _transform_shader;
    using bake_signature = void(uint, Buffer<float4>,
                                Buffer<float4>, Buffer<float4>);
    Shader<bake_signature> _bake_shader;

public:
    explicit BakerPipeline(const PipelineDesc &desc);
    static void create_cache_directory_if_necessary() {
        Context::create_directory_if_necessary(Global::instance().scene_cache_path());
    }
    void compile_shaders() noexcept override;
    void init_postprocessor(const vision::SceneDesc &scene_desc) override;
    void compile_transform_shader() noexcept;
    void init_scene(const vision::SceneDesc &scene_desc) override;
    void prepare() noexcept override;
    void display(double dt) noexcept override;
    void render(double dt) noexcept override;
    void bake_all() noexcept;
    void bake(BakedShape &baked_shape) noexcept;
    [[nodiscard]] RayState generate_ray(const Float4& position, const Float4 &normal) const noexcept;
    void preprocess() noexcept override;
    template<typename Func>
    void for_each_need_bake(Func &&func) {
        auto &meshes = _scene.shapes();
        std::for_each(meshes.begin(), meshes.end(), [&](vision::Shape *item) {
            if (item->has_emission()) {
                return;
            }
            func(item);
        });
    }
};

}// namespace vision
