//
// Created by Zero on 21/10/2022.
//

#include "base/shape.h"
#include "base/mgr/scene.h"
#include "importers/assimp_parser.h"

namespace vision {

class Model : public ShapeGroup {
public:
    explicit Model(const ShapeDesc &desc)
        : ShapeGroup(desc) {
        load(desc);
        post_init(desc);
    }

    void load(const ShapeDesc &desc) noexcept {
        auto fn = scene_path() / desc["fn"].as_string();
        AssimpParser assimp_util;
        assimp_util.load_scene(fn, desc["swap_handed"].as_bool(false),
                               desc["smooth"].as_bool(false),
                               desc["flip_uv"].as_bool(true));
        string mat_name = desc["material"].as_string();
        _instances = assimp_util.parse_meshes(mat_name.empty(), desc["subdiv_level"].as_uint(0u));
    }
};

}// namespace vision

VS_MAKE_CLASS_CREATOR(vision::Model)