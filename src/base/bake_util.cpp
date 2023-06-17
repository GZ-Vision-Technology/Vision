//
// Created by Zero on 2023/6/17.
//

#include "bake_utlis.h"

namespace vision {

void BakedShape::prepare_for_rasterize() noexcept {
    _normal.reset_all(_shape->device(), pixel_num());
    _position.reset_all(_shape->device(), pixel_num());
    auto &stream = shape()->pipeline()->stream();
    stream << _normal.device_buffer().clear()
           << _position.device_buffer().clear();
    shape()->for_each_mesh([&](vision::Mesh &mesh, uint index) {
        DeviceMesh device_mesh;
        device_mesh.vertices = shape()->device().create_buffer<Vertex>(mesh.vertices.size());
        device_mesh.triangles = shape()->device().create_buffer<Triangle>(mesh.triangles.size());
        stream << device_mesh.vertices.upload(mesh.vertices.data())
               << device_mesh.triangles.upload(mesh.triangles.data());
        _device_meshes.push_back(ocarina::move(device_mesh));
    });
}

UVSpreadResult BakedShape::load_uv_config_from_cache() const {
    DataWrap json = create_json_from_file(uv_config_fn());
    auto res = json["resolution"];
    UVSpreadResult spread_result;
    spread_result.width = res[0];
    spread_result.height = res[1];
    _shape->for_each_mesh([&](vision::Mesh &mesh, uint i) {
        DataWrap elm = json["uv_result"][i];
        auto vertices = elm["vertices"];
        UVSpreadMesh u_mesh;
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

void BakedShape::save_to_cache(const UVSpreadResult &result) {
    Context::create_directory_if_necessary(cache_directory());
    DataWrap data = DataWrap::object();
    data["resolution"] = {result.width, result.height};
    data["uv_result"] = DataWrap::array();
    _shape->for_each_mesh([&](vision::Mesh &mesh, uint i) {
        const UVSpreadMesh &u_mesh = result.meshes[i];
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

void BakedShape::remedy_vertices(UVSpreadResult result) {
    _resolution = make_uint2(result.width, result.height);
    _shape->for_each_mesh([&](vision::Mesh &mesh, uint i) {
        UVSpreadMesh &u_mesh = result.meshes[i];
        vector<Vertex> vertices;
        vertices.reserve(u_mesh.vertices.size());
        for (auto &vert : u_mesh.vertices) {
            Vertex vertex = mesh.vertices[vert.xref];
            vertex.set_lightmap_uv(vert.uv / make_float2(result.width, result.height));
            vertices.push_back(vertex);
        }
        mesh.vertices = ocarina::move(vertices);
        mesh.triangles = ocarina::move(u_mesh.triangles);
    });
}
fs::path BakedShape::uv_config_fn() const noexcept {
    return cache_directory() / "uv_config.json";
}
bool BakedShape::has_uv_cache() const noexcept {
    return fs::exists(uv_config_fn());
}
fs::path BakedShape::rasterization_position() const noexcept {
    return cache_directory() / "position.exr";
}
fs::path BakedShape::rasterization_normal() const noexcept {
    return cache_directory() / "normal.exr";
}
bool BakedShape::has_rasterization_cache() const noexcept {
    return fs::exists(rasterization_normal()) && fs::exists(rasterization_position());
}

}// namespace vision