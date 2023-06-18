//
// Created by Zero on 2023/6/15.
//

#include "base/bake_utlis.h"

namespace vision {

class SoftwareRasterizer : public Rasterizer {
private:
    using signature = void(Buffer<Vertex>, Buffer<Triangle>, Buffer<float4>, Buffer<float4>, uint2, uint);
    Shader<signature> _shader;

public:
    explicit SoftwareRasterizer(const RasterizerDesc &desc)
        : Rasterizer(desc) {}

    void compile_shader() noexcept override {
        Kernel kernel = [&](BufferVar<Vertex> vertices, BufferVar<Triangle> triangles,
                                   BufferVar<float4> positions, BufferVar<float4> normals, Uint2 res, Uint triangle_index) {
            Float2 coord = (make_float2(dispatch_idx().xy()) + 0.5f);
            Var tri = triangles.read(triangle_index);
            Var v0 = vertices.read(tri.i);
            Var v1 = vertices.read(tri.j);
            Var v2 = vertices.read(tri.k);

            Float2 p0 = v0->lightmap_uv() * make_float2(res);
            Float2 p1 = v1->lightmap_uv() * make_float2(res);
            Float2 p2 = v2->lightmap_uv() * make_float2(res);
            $if(in_triangle<D>(coord, p0, p1, p2)) {
                Float2 bary = barycentric(coord, p0, p1, p2);
                Float3 pos = triangle_lerp(bary, v0->position(), v1->position(), v2->position());
                positions.write(dispatch_id(), make_float4(pos,1.f));
                Float3 norm = triangle_lerp(bary, v0->normal(), v1->normal(), v2->normal());
                normals.write(dispatch_id(), make_float4(norm,1.f));
            };
        };
        _shader = device().compile(kernel);
    }

    void apply(vision::BakedShape &baked_shape) noexcept override {
        auto &stream = pipeline()->stream();
        baked_shape.for_each_device_mesh([&](DeviceMesh &device_mesh, uint index) {

            const vision::Mesh &mesh = baked_shape.shape()->mesh_at(index);
            for (int i = 0; i < mesh.triangles.size(); ++i) {
                stream << _shader(device_mesh.vertices, device_mesh.triangles,
                                  baked_shape.positions(),
                                  baked_shape.normals(),
                                  make_uint2(baked_shape.resolution()), i).dispatch(baked_shape.resolution());
            }
        });
    }
};

}// namespace vision

VS_MAKE_CLASS_CREATOR(vision::SoftwareRasterizer)