//
// Created by Zero on 21/10/2022.
//

#include "base/shape.h"
#include "math/transform.h"

namespace vision {

class Quad : public vision::Group {
public:
    using Super = vision::Group;

public:
    explicit Quad(const ShapeDesc &desc) : Super(desc) {
        init(desc);
        post_init(desc);
    }

    void init(const ShapeDesc &desc) noexcept {
        auto mesh = make_shared<Mesh>(desc);
        float width = desc["width"].as_float(1.f) / 2;
        float height = desc["height"].as_float(1.f) / 2;
        vector<float3> P{make_float3(width, 0, height),
                         make_float3(width, 0, -height),
                         make_float3(-width, 0, height),
                         make_float3(-width, 0, -height)};

        for (auto p : P) { mesh->aabb.extend(p); }
        vector<float3> N(4, make_float3(0, 1, 0));
        vector<float2> UV{make_float2(1, 1),
                          make_float2(1, 0),
                          make_float2(0, 1),
                          make_float2(0, 0)};
        for (int i = 0; i < P.size(); ++i) {
            mesh->vertices.emplace_back(P[i], N[i], UV[i]);
            mesh->aabb.extend(transform_point<H>(_o2w, P[i]));
        }
        mesh->triangles = {Triangle{0, 1, 2}, Triangle{2, 1, 3}};
        _meshes.push_back(mesh);

        _instances.emplace_back(mesh);
    }
};

}// namespace vision

VS_MAKE_CLASS_CREATOR(vision::Quad)