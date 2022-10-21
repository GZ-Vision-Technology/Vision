//
// Created by Zero on 11/10/2022.
//

#pragma once

#include "core/basic_types.h"
#include "rhi/common.h"
#include "dsl/common.h"
#include "node.h"

namespace vision {
using namespace ocarina;
class Film : public Node {
public:
    using Desc = FilmDesc;
private:
    uint2 _resolution;

public:
    explicit Film(const FilmDesc *desc) : Node(desc), _resolution(desc->resolution) {}
    [[nodiscard]] uint pixel_num() const noexcept { return _resolution.x * _resolution.y; }
    [[nodiscard]] uint pixel_index(uint2 pixel) const noexcept { return pixel.y * _resolution.x + pixel.x; }
    [[nodiscard]] uint2 resolution() const noexcept { return _resolution; }
    virtual void add_sample(Uint2 pixel, Float4 val, Uint frame_index) noexcept = 0;
};
}// namespace vision