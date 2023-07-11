//
// Created by Zero on 2023/7/6.
//

#include "batch_mesh.h"
#include "baker.h"
#include "util.h"

namespace vision {

CommandList BatchMesh::clear() noexcept {
    CommandList ret;
    ret << _triangles_old.device_buffer().clear();
    ret << _vertices_old.device_buffer().clear();
    ret << _triangles.device_buffer().clear();
    ret << _vertices.device_buffer().clear();
    ret << _pixels.device_buffer().clear();
    ret << [&] {
        _triangles_old.host_buffer().clear();
        _vertices_old.host_buffer().clear();
        _triangles.host_buffer().clear();
        _vertices.host_buffer().clear();
        _pixel_num = 0;
    };
    return ret;
}

void BatchMesh::allocate(ocarina::uint buffer_size) {
    _pixels.resize(buffer_size);
    for (int i = 0; i < buffer_size; ++i) {
        _pixels.at(i) = make_uint4(InvalidUI32);
    }
    _pixels.device_buffer() = device().create_buffer<uint4>(buffer_size);
}

Command *BatchMesh::reset_pixels() noexcept {
    return _pixels.upload();
}

void BatchMesh::setup(ocarina::span<BakedShape> baked_shapes) noexcept {
    uint vert_offset = 0;
    vector<std::pair<uint2, uint>> res_offset;
//    for (BakedShape &bs : baked_shapes) {
//        bs.shape()->for_each_mesh([&](const vision::Mesh &mesh, int index) {
//            float4x4 o2w = bs.shape()->o2w();
////            for (const Vertex &vertex : mesh.vertices) {
////                float3 world_pos = transform_point<H>(o2w, vertex.position());
////                float3 world_norm = transform_normal<H>(o2w, vertex.normal());
////                world_norm = select(nonzero(world_norm), normalize(world_norm), world_norm);
//////                _vertices_old.emplace_back(world_pos, world_norm, vertex.tex_coord(), vertex.lightmap_uv());
////            }
//            for (const Triangle &tri : mesh.triangles) {
////                _triangles_old.emplace_back(tri.i + vert_offset, tri.j + vert_offset, tri.k + vert_offset);
//                res_offset.emplace_back(bs.resolution(), _pixel_num);
//            }
//            vert_offset += mesh.vertices.size();
//        });
//        bs.normalize_lightmap_uv();
//        _pixel_num += bs.pixel_num();
//    }

//    _vertices_old.reset_device_buffer_immediately(device());
//    _triangles_old.reset_device_buffer_immediately(device());

    auto rasterize = [&]() {
        stream() << _vertices.upload()
                 << reset_pixels()
                 << _triangles.upload();

        for (uint i = 0; i < _triangles.host_buffer().size(); ++i) {
            auto [res, offset] = res_offset[i];
            stream() << _rasterize(_triangles, _vertices,
                                   _pixels, offset, i, res)
                            .dispatch(pixel_num());
            stream() << synchronize() << commit();
        }
    };
//
//    rasterize();
}

void BatchMesh::batch(ocarina::span<BakedShape> baked_shapes) noexcept {
    uint triangle_offset = 0;
    uint vert_offset = 0;
    uint pixel_offset = 0;
    CommandList cmd_lst;
    for (BakedShape &bs : baked_shapes) {
        MergedMesh &mesh = bs.merged_mesh();

        cmd_lst << _shader(bs.pixels(), triangle_offset,
                           pixel_offset, _pixels)
                       .dispatch(bs.resolution());
        for (Triangle tri : mesh.triangles) {
            _triangles.emplace_back(tri.i + vert_offset,
                                    tri.j + vert_offset,
                                    tri.k + vert_offset);
        }

        triangle_offset += mesh.triangles.host_buffer().size();
        vert_offset += mesh.vertices.host_buffer().size();
        pixel_offset += bs.pixel_num();
        append(_vertices, mesh.vertices);
        bs.normalize_lightmap_uv();
        _pixel_num += bs.pixel_num();
    }
    stream() << cmd_lst << synchronize() << commit();
    _pixels.download_immediately();
    _vertices.reset_device_buffer_immediately(device());
    _triangles.reset_device_buffer_immediately(device());
    stream() << _vertices.upload() << _triangles.upload() <<synchronize() << commit();
}

void BatchMesh::compile() noexcept {

    Kernel kernel = [&](BufferVar<uint4> src_pixels, Uint triangle_offset,
                        Uint pixel_offset, BufferVar<uint4> dst_pixels) {
        Uint2 res = dispatch_dim().xy();
        Uint4 pixel = src_pixels.read(dispatch_id());
        Bool valid = bit_cast<uint>(1.f) == pixel.w;
        $if(valid) {
            pixel.x += triangle_offset;
        };
        pixel.y = pixel_offset;
        pixel.z = detail::uint2_to_uint(res);
        dst_pixels.write(dispatch_id() + pixel_offset, pixel);
    };
    _shader = device().compile(kernel, "preprocess rasterize");

    Kernel kn = [&](BufferVar<Triangle> triangles, BufferVar<Vertex> vertices,
                    BufferVar<uint4> pixels, Uint pixel_offset, Uint triangle_index, Uint2 res) {
        Uint pixel_num = res.x * res.y;
        $if(dispatch_id() < pixel_offset || dispatch_id() >= pixel_offset + pixel_num) {
            $return();
        };

        Uint pixel_index = dispatch_id() - pixel_offset;

        Uint x = pixel_index % res.x;
        Uint y = pixel_index / res.x;

        Float2 coord = make_float2(x + 0.5f, y + 0.5f);
        Var tri = triangles.read(triangle_index);
        Var v0 = vertices.read(tri.i);
        Var v1 = vertices.read(tri.j);
        Var v2 = vertices.read(tri.k);

        Float2 p0 = v0->lightmap_uv();
        Float2 p1 = v1->lightmap_uv();
        Float2 p2 = v2->lightmap_uv();
        Uint4 pixel = pixels.read(dispatch_id());
        $if(in_triangle<D>(coord, p0, p1, p2)) {
            pixel.x = triangle_index;
        };
        pixel.y = pixel_offset;
        pixel.z = detail::uint2_to_uint(res);
        pixels.write(dispatch_id(), pixel);
    };
    _rasterize = device().compile(kn, "rasterize");
}

}// namespace vision