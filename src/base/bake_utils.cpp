//
// Created by Zero on 2023/6/17.
//

#include "bake_utlis.h"

namespace vision {

void BakedShape::prepare_for_rasterize_old() noexcept {
    _normals.super() = shape()->device().create_buffer<float4>(pixel_num());
    _positions.super() = shape()->device().create_buffer<float4>(pixel_num());
    auto &stream = shape()->pipeline()->stream();
    stream << _normals.clear()
           << _positions.clear();
    shape()->for_each_mesh([&](vision::Mesh &mesh, uint index) {
        DeviceMesh device_mesh;
        device_mesh.vertices = shape()->device().create_buffer<Vertex>(mesh.vertices.size());
        device_mesh.triangles = shape()->device().create_buffer<Triangle>(mesh.triangles.size());
        stream << device_mesh.vertices.upload(mesh.vertices.data())
               << device_mesh.triangles.upload(mesh.triangles.data());
        _device_meshes.push_back(ocarina::move(device_mesh));
    });
}

CommandList BakedShape::prepare_for_rasterize() noexcept {
    CommandList ret;
    _normals.super() = shape()->device().create_buffer<float4>(pixel_num());
    _positions.super() = shape()->device().create_buffer<float4>(pixel_num());
    ret << _normals.clear() << _positions.clear();
    shape()->for_each_mesh([&](vision::Mesh &mesh, uint index) {
        DeviceMesh device_mesh;
        device_mesh.vertices = shape()->device().create_buffer<Vertex>(mesh.vertices.size());
        device_mesh.triangles = shape()->device().create_buffer<Triangle>(mesh.triangles.size());
        ret << device_mesh.vertices.upload(mesh.vertices.data())
            << device_mesh.triangles.upload(mesh.triangles.data());
        _device_meshes.push_back(ocarina::move(device_mesh));
    });
    return ret;
}

void BakedShape::prepare_for_bake() noexcept {
    _lightmap.super() = shape()->device().create_buffer<float4>(pixel_num());
    auto &stream = shape()->pipeline()->stream();
    stream << _lightmap.clear();
}

void BakedShape::reallocate_lightmap() noexcept {
    _lightmap.super() = shape()->device().create_buffer<float4>(pixel_num());
    _lightmap.clear_immediately();
}

UnwrapperResult BakedShape::load_uv_config_from_cache() const {
    DataWrap json = create_json_from_file(uv_config_fn());
    auto res = json["resolution"];
    UnwrapperResult spread_result;
    spread_result.width = res[0];
    spread_result.height = res[1];
    _shape->for_each_mesh([&](vision::Mesh &mesh, uint i) {
        DataWrap elm = json["uv_result"][i];
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
    });
    return spread_result;
}

void BakedShape::save_to_cache(const UnwrapperResult &result) {
    Context::create_directory_if_necessary(cache_directory());
    DataWrap data = DataWrap::object();
    data["resolution"] = {result.width, result.height};
    data["uv_result"] = DataWrap::array();
    _shape->for_each_mesh([&](vision::Mesh &mesh, uint i) {
        const UnwrapperMesh &u_mesh = result.meshes[i];
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
    });

    string data_str = data.dump(4);
    fs::path uv_config = uv_config_fn();
    Context::write_file(uv_config, data_str);
}

CommandList BakedShape::load_rasterization_from_cache() const {
    CommandList ret;
    ImageIO &position = ImagePool::instance().obtain_image(position_cache_path(), ColorSpace::LINEAR).image();
    ImageIO &normal = ImagePool::instance().obtain_image(normal_cache_path(), ColorSpace::LINEAR).image();
    ret << _positions.upload(position.pixel_ptr())
        << _normals.upload(normal.pixel_ptr());
    return ret;
}

CommandList BakedShape::save_rasterization_to_cache() const {
    CommandList ret;
    float4 *ptr = ocarina::allocate<float4>(pixel_num());

    ret << _positions.download(ptr) << [&, ptr] {
        ImageIO::save_image(position_cache_path(), PixelStorage::FLOAT4, _resolution, ptr);
    };

    ret << _normals.download(ptr) << [&, ptr] {
        ImageIO::save_image(normal_cache_path(), PixelStorage::FLOAT4, _resolution, ptr);
        ocarina::deallocate(ptr);
    };
    return ret;
}

void BakedShape::save_lightmap_to_cache_old() const {
    vector<float4> map;
    map.resize(pixel_num());
    _lightmap.download_immediately(map.data());
    ImageIO::save_image(lightmap_cache_path(), PixelStorage::FLOAT4, _resolution, map.data());
}

CommandList BakedShape::save_lightmap_to_cache() const {
    CommandList ret;
    float4 *ptr = ocarina::allocate<float4>(pixel_num());
    ret << _lightmap.download(ptr);
    ret << [&, ptr] {
        ImageIO::save_image(lightmap_cache_path(), PixelStorage::FLOAT4, _resolution, ptr);
        ocarina::deallocate(ptr);
    };
    return ret;
}

void BakedShape::allocate_lightmap_texture() noexcept {
    _lightmap_tex = shape()->device().create_texture(resolution(), ocarina::PixelStorage::FLOAT4);
}

void BakedShape::setup_vertices(UnwrapperResult result) {
    _resolution = make_uint2(result.width, result.height);
    _shape->for_each_mesh([&](vision::Mesh &mesh, uint i) {
        UnwrapperMesh &u_mesh = result.meshes[i];
        vector<Vertex> vertices;
        vertices.reserve(u_mesh.vertices.size());
        for (auto &vert : u_mesh.vertices) {
            Vertex vertex = mesh.vertices[vert.xref];
            vertex.set_lightmap_uv(vert.uv);
            vertices.push_back(vertex);
        }
        mesh.vertices = ocarina::move(vertices);
        mesh.triangles = ocarina::move(u_mesh.triangles);
    });
}

void BakedShape::normalize_lightmap_uv() {
    _shape->for_each_mesh([&](vision::Mesh &mesh, uint i) {
        for (auto &vert : mesh.vertices) {
            vert.set_lightmap_uv(vert.lightmap_uv() / make_float2(resolution()));
        }
    });
}

uint64_t BakedShape::instance_hash() const noexcept {
    return hash64(_shape->hash(), _shape->o2w());
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

fs::path BakedShape::position_cache_path() const noexcept {
    return cache_directory() / "position.exr";
}

fs::path BakedShape::normal_cache_path() const noexcept {
    return cache_directory() / "normal.exr";
}

bool BakedShape::has_rasterization_cache() const noexcept {
    return fs::exists(normal_cache_path()) && fs::exists(position_cache_path());
}

}// namespace vision