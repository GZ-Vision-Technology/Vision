//
// Created by Zero on 09/09/2022.
//

#pragma once

#include <utility>
#include "core/stl.h"

namespace vision {

class NodeDesc;

class Node {
private:
    ocarina::string _name;

public:
    using Creator = Node *(NodeDesc *);
    using Deleter = void(Node *);
    using Handle = ocarina::unique_ptr<Node, Node::Deleter *>;

public:
    Node() = default;
    explicit Node(ocarina::string name) : _name(std::move(name)) {}
    virtual ~Node() = default;
    [[nodiscard]] ocarina::string name() const noexcept { return _name; }
};
}// namespace vision

#define VS_MAKE_CLASS_CREATOR(Class)                                                  \
    VS_EXPORT_API Class *create(vision::NodeDesc *desc) {                             \
        return ocarina::new_with_allocator<Class>(dynamic_cast<Class::Desc *>(desc)); \
    }                                                                                 \
    OC_EXPORT_API void destroy(Class *obj) {                                          \
        ocarina::delete_with_allocator(obj);                                          \
    }
