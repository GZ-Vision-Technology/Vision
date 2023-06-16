//
// Created by Zero on 2023/6/14.
//

#pragma once

#include "base/mgr/pipeline.h"
#include "base/bake.h"
#include "expander.h"

namespace vision {

class BakerPipeline : public Pipeline {
private:
    UVSpreader *_uv_spreader{};
    Rasterizer *_rasterizer{};
    UP<Expander> _expander;
    vector<BakedShape> _baked_shapes;

public:
    explicit BakerPipeline(const PipelineDesc &desc)
        : Pipeline(desc),
          _uv_spreader(Global::node_mgr().load<UVSpreader>(desc.uv_spreader_desc)),
          _rasterizer(Global::node_mgr().load<Rasterizer>(desc.rasterizer_desc)) {
        create_cache_directory_if_necessary();
    }

    static void create_cache_directory_if_necessary()  {
        Context::create_directory_if_necessary(Global::instance().scene_cache_path());
    }

    template<typename Func>
    void for_each_need_bake(Func &&func) {
        auto &meshes = _scene.shapes();
        std::for_each(meshes.begin(), meshes.end(), [&](vision::Shape *item) {
            if (!item->has_emission()) {
                func(item);
            }
        });
    }

    void preprocess() noexcept override {
        // uv spread
        for_each_need_bake([&](auto &item) {
            BakedShape baked_shape = _uv_spreader->apply(item);
            _baked_shapes.push_back(ocarina::move(baked_shape));
        });

        std::for_each(_baked_shapes.begin(), _baked_shapes.end(), [&](BakedShape &baked_shape) {
            baked_shape.save_uv_spread_result_to_cache();
        });

        // raster
        std::for_each(_baked_shapes.begin(), _baked_shapes.end(), [&](BakedShape &baked_shape) {

        });
    }
};

}// namespace vision
