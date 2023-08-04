//
// Created by Zero on 2023/7/17.
//

#include "vision.h"
#include "base/node.h"
#include "core/stl.h"
#include "base/shape.h"
#include "base/mgr/global.h"
#include "base/mgr/pipeline.h"

namespace vision::sdk {

class VisionRendererImpl : public VisionRenderer {
private:
    UP<Device> _device;
    SP<Pipeline> _pipeline{};

public:
    void init_pipeline(const char *rpath) override;
    void init_scene() override;
    void clear_geometries() override;
    void add_instance(const Instance &instance) override;
    void build_accel() override;
    void update_camera(vision::sdk::Camera camera) override;
    void update_resolution(uint32_t width, uint32_t height) override;
};

void VisionRendererImpl::init_pipeline(const char *rpath) {
    PipelineDesc desc;
    Context::instance().init(rpath);
    _device = make_unique<Device>(Context::instance().create_device("cuda"));
    desc.device = _device.get();
    desc.sub_type = "offline";
    _pipeline = Global::node_mgr().load<Pipeline>(desc);
    Global::instance().set_pipeline(_pipeline.get());
}

void VisionRendererImpl::clear_geometries() {
    std::cout << "clear_geometries" << std::endl;
    _pipeline->clear_geometry();
}

void VisionRendererImpl::init_scene() {
    SceneDesc scene_desc;
    scene_desc.init(DataWrap::object());
    Scene &scene = _pipeline->scene();
    scene.init(scene_desc);
}

vision::Vertex from_sdk_vertex(const sdk::Vertex &vert) {
    vision::Vertex vertex;
    vertex.pos = vert.pos;
    vertex.n = vert.n;
    vertex.uv = vert.uv;
    return vertex;
}

Triangle from_triple(const sdk::Triple &triple) {
    return bit_cast<Triangle>(triple);
}

float4x4 from_array(std::array<float, 16> arr) {
    float4x4 ret;
    for (int i = 0; i < 16; ++i) {
        int x = i / 4;
        int y = i % 4;
        ret[x][y] = arr[i];
    }
    return ret;
}

ShapeInstance from_sdk_instance(const sdk::Instance &inst) {

    SP<Mesh> mesh = std::make_shared<Mesh>();
    for (int i = 0; i < inst.vert_num; ++i) {
        mesh->vertices.push_back(from_sdk_vertex(inst.vertices.get()[i]));
    }

    for (int i = 0; i < inst.tri_num; ++i) {
        mesh->triangles.push_back(from_triple(inst.triangles.get()[i]));
    }

    ShapeInstance ret{mesh};

    ret.set_o2w(from_array(inst.mat4.m));

    return ret;
}

void VisionRendererImpl::add_instance(const vision::sdk::Instance &instance) {
    SP<ShapeGroup> group = std::make_shared<ShapeGroup>(ShapeDesc{});
    ShapeInstance inst = from_sdk_instance(instance);
    group->add_instance(inst);
}

void VisionRendererImpl::build_accel() {
}

void VisionRendererImpl::update_camera(vision::sdk::Camera camera) {
}

void VisionRendererImpl::update_resolution(uint32_t width, uint32_t height) {
}

}// namespace vision::sdk

VS_EXPORT_API vision::sdk::VisionRenderer *create() {
    return ocarina::new_with_allocator<vision::sdk::VisionRendererImpl>();
}
OC_EXPORT_API void destroy(vision::sdk::VisionRenderer *obj) {
    ocarina::delete_with_allocator(obj);
}
