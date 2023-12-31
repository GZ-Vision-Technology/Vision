//
// Created by Zero on 2023/6/17.
//

#include "bake_utlis.h"

namespace vision {

BakedShape::BakedShape(vision::ShapeInstance *shape)
    : _shape(shape) {}

UnwrapperResult BakedShape::load_uv_config_from_cache() const {
    DataWrap json = create_json_from_file(uv_config_fn());
    auto res = json["resolution"];
    UnwrapperResult spread_result;
    spread_result.width = res[0];
    spread_result.height = res[1];
    DataWrap elm = json["uv_result"][0];
    auto vertices = elm["vertices"];
    UnwrapperMesh u_mesh;
    for (auto vertex : vertices) {
        u_mesh.vertices.emplace_back(make_float2(vertex[0], vertex[1]), vertex[2]);
    }

    auto triangles = elm["triangles"];
    for (auto tri : triangles) {
        u_mesh.triangles.emplace_back(tri[0], tri[1], tri[2]);
    }
    spread_result.meshes.push_back(u_mesh);
    return spread_result;
}

void BakedShape::save_to_cache(const UnwrapperResult &result) {
    Context::create_directory_if_necessary(cache_directory());
    DataWrap data = DataWrap::object();
    data["resolution"] = {result.width, result.height};
    data["uv_result"] = DataWrap::array();
    const UnwrapperMesh &u_mesh = result.meshes[0];
    DataWrap elm = DataWrap::object();
    elm["vertices"] = DataWrap::array();
    for (auto vertex : u_mesh.vertices) {
        elm["vertices"].push_back({vertex.uv.x, vertex.uv.y, vertex.xref});
    }
    elm["triangles"] = DataWrap::array();
    for (Triangle tri : u_mesh.triangles) {
        elm["triangles"].push_back({tri.i, tri.j, tri.k});
    }
    data["uv_result"].push_back(elm);
    string data_str = data.dump(4);
    fs::path uv_config = uv_config_fn();
    Context::write_file(uv_config, data_str);
}

CommandList BakedShape::save_lightmap_to_cache() const {
    Context::create_directory_if_necessary(instance_cache_directory());
    CommandList ret;
    float4 *ptr = ocarina::allocate<float4>(pixel_num());
    ret << _lightmap_tex.download(ptr);
    ret << [&, ptr] {
        ImageIO::save_image(lightmap_cache_path(), PixelStorage::FLOAT4, resolution(), ptr);
        ocarina::deallocate(ptr);
    };
    return ret;
}

CommandList BakedShape::save_rasterize_map_to_cache() const {
    CommandList ret;
    Context::create_directory_if_necessary(instance_cache_directory());
    float4 *ptr = ocarina::allocate<float4>(pixel_num());
    ret << _pixels.download(ptr);
    ret << [&, ptr] {
        ImageIO::save_image(rasterize_cache_path(), PixelStorage::FLOAT4, resolution(), ptr);
        ocarina::deallocate(ptr);
    };
    return ret;
}

void BakedShape::merge_meshes() noexcept {
    uint vert_offset = 0;
    float4x4 o2w = shape()->o2w();
    for (const Vertex &vertex : _shape->mesh()->vertices()) {
        float3 world_pos = transform_point<H>(o2w, vertex.position());
        float3 world_norm = transform_normal<H>(o2w, vertex.normal());
        world_norm = select(nonzero(world_norm), normalize(world_norm), world_norm);
        _merged_mesh.vertices.emplace_back(world_pos, world_norm,
                                           vertex.tex_coord(),
                                           vertex.lightmap_uv());
    }
    for (const Triangle &tri : _shape->mesh()->triangles()) {
        _merged_mesh.triangles.emplace_back(tri.i + vert_offset,
                                            tri.j + vert_offset,
                                            tri.k + vert_offset);
    }
}

void BakedShape::prepare_to_rasterize() noexcept {
    merge_meshes();
    _pixels = device().create_buffer<uint4>(pixel_num());
    _pixels.reset_immediately();
}

void BakedShape::allocate_lightmap_texture() noexcept {
    _lightmap_tex = device().create_texture(resolution(), ocarina::PixelStorage::FLOAT4);
}

void BakedShape::normalize_lightmap_uv() {
    _shape->mesh()->normalize_lightmap_uv();
}

uint64_t BakedShape::instance_hash() const noexcept {
    return hash64(_shape->mesh()->hash(), _shape->o2w());
}

fs::path BakedShape::uv_config_fn() const noexcept {
    return cache_directory() / "uv_config.json";
}

bool BakedShape::has_uv_cache() const noexcept {
    return fs::exists(uv_config_fn());
}

fs::path BakedShape::lightmap_cache_path() const noexcept {
    return instance_cache_directory() / "lightmap.exr";
}

fs::path BakedShape::rasterize_cache_path() const noexcept {
    return instance_cache_directory() / "rasterize.exr";
}

}// namespace vision