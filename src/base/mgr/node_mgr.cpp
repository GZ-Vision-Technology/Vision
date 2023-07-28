//
// Created by Zero on 2023/6/14.
//

#include "node_mgr.h"
#include "pipeline.h"

namespace vision {

NodeMgr *NodeMgr::s_node_loader = nullptr;

NodeMgr &NodeMgr::instance() noexcept {
    if (s_node_loader == nullptr) {
        s_node_loader = new vision::NodeMgr();
    }
    return *s_node_loader;
}

void NodeMgr::destroy_instance() noexcept {
    if (s_node_loader) {
        delete s_node_loader;
        s_node_loader = nullptr;
    }
}

SP<Node> NodeMgr::load_node(const vision::NodeDesc &desc) {
    const DynamicModule *module = Context::instance().obtain_module(desc.plugin_name());
    auto creator = reinterpret_cast<Node::Creator *>(module->function_ptr("create"));
    auto deleter = reinterpret_cast<Node::Deleter *>(module->function_ptr("destroy"));
    SP<Node> ret = SP<Node>(creator(desc), deleter);
    return ret;
}

SP<ShaderNode> NodeMgr::load_shader_node(const ShaderNodeDesc &desc) {
    auto ret = load<ShaderNode>(desc);
    return ret;
}

Slot NodeMgr::create_slot(const SlotDesc &desc) {
    SP<ShaderNode >shader_node = load_shader_node(desc.node);
    return Slot(shader_node, desc.channels);
}
}// namespace vision