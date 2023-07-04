//
// Created by Zero on 2023/6/14.
//

#pragma once

#include "rhi/common.h"
#include "core/hash.h"

namespace vision {
using namespace ocarina;

class Expander : public Hashable {
    uint64_t _compute_hash() const noexcept override;
};

}// namespace vision