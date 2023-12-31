//
// Created by Zero on 2023/7/17.
//

#pragma once

#include "base/shape.h"
#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
#include <assimp/Subdivision.h>
#include <assimp/scene.h>
#include "base/sensor/camera.h"

namespace vision {

namespace assimp {

[[nodiscard]] float2 from_vec2(aiVector2D vec) noexcept {
    return make_float2(vec.x, vec.y);
}

[[nodiscard]] float3 from_vec3(aiVector3D vec) noexcept {
    return make_float3(vec.x, vec.y, vec.z);
}

[[nodiscard]] float4 from_color4(aiColor4D vec) noexcept {
    return make_float4(vec.r, vec.g, vec.b, vec.a);
}

[[nodiscard]] float3 from_color3(aiColor3D vec) noexcept {
    return make_float3(vec.r, vec.g, vec.b);
}

}// namespace assimp

class AssimpParser {
private:
    Assimp::Importer _ai_importer;
    const aiScene *_ai_scene{};
    fs::path _directory;

public:
    const aiScene *load_scene(const fs::path &fn,
                              bool swap_handed = false, bool smooth = true,
                              bool flip_uv = false);
    [[nodiscard]] vector<ShapeInstance> parse_meshes(bool parse_material,
                                                     uint32_t subdiv_level = 0u);
    [[nodiscard]] vector<float4x4> parse_cameras() noexcept;
    [[nodiscard]] float4x4 parse_camera(aiCamera *ai_camera) noexcept;
    [[nodiscard]] SP<vision::Light> point_light(aiLight *ai_light) noexcept;
    [[nodiscard]] SP<vision::Light> spot_light(aiLight *ai_light) noexcept;
    [[nodiscard]] SP<vision::Light> area_light(aiLight *ai_light) noexcept;
    [[nodiscard]] SP<vision::Light> environment(aiLight *ai_light) noexcept;
    [[nodiscard]] SP<vision::Light> directional_light(aiLight *ai_light) noexcept;
    [[nodiscard]] SP<vision::Light> parse_light(aiLight *ai_light) noexcept;
    [[nodiscard]] vector<SP<vision::Light>> parse_lights() noexcept;
    [[nodiscard]] static std::pair<string, float4> parse_texture(const aiMaterial *mat, aiTextureType type);
    [[nodiscard]] vector<vision::MaterialDesc> parse_materials() noexcept;
    [[nodiscard]] vision::MaterialDesc parse_material(aiMaterial *ai_material) noexcept;
};

}// namespace vision