//
// Created by Zero on 2023/6/14.
//

#pragma once

#include "base/mgr/pipeline.h"
#include "base/bake_utlis.h"
#include "expander.h"
#include "baker.h"

namespace vision {

/**
 * Process of baking
 * 1. unwrap uv and cache
 * 2. rasterize position, normal map and cache
 * 3. transform instance position , normal map to world space
 * 4. bake_old (to increase parallelism, mesh's normal, position map can be merged) and save to cache
 * 5. postprocess eg. denoise, padding ...
 * 6. update geometry data
 * 7. display
 */
class BakePipeline : public Pipeline {
private:
    UVUnwrapper *_uv_unwrapper{};
    Rasterizer *_rasterizer{};
    UP<Expander> _expander;
    vector<BakedShape> _baked_shapes;

    using transform_signature = void(Buffer<float4>,
                                     Buffer<float4>,
                                     float4x4 o2w);
    Baker _baker;

    Shader<transform_signature> _transform_shader_old;

    Shader<void(Buffer<float4>, Buffer<float4>, float4x4, uint, uint2)> _transform_shader;

    using bake_signature = void(uint, Buffer<float4>,
                                Buffer<float4>, Buffer<float4>);
    Shader<bake_signature> _bake_shader_old;

    Shader<void(uint, Buffer<float4>, Buffer<float4>, Buffer<float4>)> _bake_shader;

    Shader<void(uint)> _display_shader;
    uint _lightmap_base_index{InvalidUI32};

public:
    explicit BakePipeline(const PipelineDesc &desc);
    static void create_cache_directory_if_necessary() {
        Context::create_directory_if_necessary(Global::instance().scene_cache_path());
    }
    void compile_shaders() noexcept override;
    void compile_baker() noexcept;
    void compile_displayer() noexcept;
    void init_postprocessor(const vision::SceneDesc &scene_desc) override;
    void compile_transform_shader() noexcept;
    void init_scene(const vision::SceneDesc &scene_desc) override;
    void prepare() noexcept override;
    void display(double dt) noexcept override;
    void render(double dt) noexcept override;
    void bake_all_old() noexcept;
    void upload_lightmap() noexcept;
    void bake_old(vision::BakedShape &baked_shape) noexcept;

    void bake_all() noexcept;
    [[nodiscard]] CommandList bake(Baker &bake_buffer) noexcept;
    [[nodiscard]] CommandList prepare_for_bake(uint index, uint num, Baker &bake_buffer) noexcept;

    [[nodiscard]] RayState generate_ray(const Float4 &position, const Float4 &normal, Float *pdf) const noexcept;
    [[nodiscard]] Float3 Li(RayState &rs) const noexcept;
    void preprocess() noexcept override;
    template<typename Func>
    void for_each_need_bake(Func &&func) {
        auto &meshes = _scene.shapes();
        std::for_each(meshes.begin(), meshes.end(), [&](vision::Shape *item) {
            //            if (item->has_emission()) {
            //                return;
            //            }
            func(item);
        });
    }
};

}// namespace vision