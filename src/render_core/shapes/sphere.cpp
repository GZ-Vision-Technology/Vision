//
// Created by Zero on 21/10/2022.
//

#include "base/shape.h"
#include "math/transform.h"

namespace vision {

class Sphere : public vision::Mesh {
public:
    using Super = vision::Mesh;

public:
    explicit Sphere(const ShapeDesc &desc) : Super(desc) {
        init(desc);
    }
    void init(const ShapeDesc &desc) noexcept {
        float radius = desc["radius"].as_float(1.f);
        uint theta_div = desc["sub_div"].as_uint(60u);
        uint phi_div = 2 * theta_div;
        vertices.emplace_back(make_float3(0, radius, 0),
                              make_float3(0, 1, 0),
                              make_float2(0, 0));
        for (uint i = 1; i < theta_div; ++i) {
            float v = float(i) / theta_div;
            float theta = Pi * v;
            float y = radius * cos(theta);
            float r = radius * sin(theta);
            float3 p0 = make_float3(r, y, 0.f);
            float2 t0 = make_float2(0, v);

            vertices.emplace_back(p0, normalize(p0), t0);

            for (uint j = 1; j < phi_div; ++j) {
                float u = float(j) / phi_div;
                float phi = u * _2Pi;
                float x = cos(phi) * r;
                float z = sin(phi) * r;
                float3 p = make_float3(x, y, z);
                aabb.extend(transform_point<H>(handle().o2w, p));
                float2 t = make_float2(u, v);

                vertices.emplace_back(p, normalize(p), t);
            }
        }

        vertices.emplace_back(make_float3(0, -radius, 0),
                              make_float3(0, -1, 0),
                              make_float2(0, 1));

        uint tri_count = phi_div * 2 + (theta_div - 2) * phi_div * 2;
        triangles.reserve(tri_count);

        for (uint i = 0; i < phi_div; ++i) {
            Triangle tri{0, (i + 1) % phi_div + 1, i + 1};
            triangles.push_back(tri);
        }
        for (uint i = 0; i < theta_div - 2; ++i) {
            uint vert_start = 1 + i * phi_div;
            for (int j = 0; j < phi_div; ++j) {
                if (j != phi_div - 1) {
                    Triangle tri{vert_start, vert_start + 1, vert_start + phi_div};
                    triangles.push_back(tri);
                    Triangle tri2{vert_start + 1, vert_start + phi_div + 1, vert_start + phi_div};
                    triangles.push_back(tri2);
                } else {
                    Triangle tri{vert_start, vert_start + 1 - phi_div, vert_start + phi_div};
                    triangles.push_back(tri);
                    Triangle tri2{vert_start + 1 - phi_div, vert_start + 1, vert_start + phi_div};
                    triangles.push_back(tri2);
                }
                vert_start ++;
            }
        }
        uint vert_start = (theta_div - 1) * phi_div + 2;
        uint vert_end = vertices.size() - 1;
        for (uint i = 0; i < phi_div; ++i) {
            uint idx1 = i + 1;
            uint idx2 = (1 + i) % phi_div + 1;
            Triangle tri{vert_end, vert_end - idx2, vert_end - idx1};
            triangles.push_back(tri);
        }
        for (auto & vertex : vertices) {
            aabb.extend(transform_point<H>(handle().o2w, vertex.position()));
        }
    }
};

}// namespace vision

VS_MAKE_CLASS_CREATOR(vision::Sphere)