//
// Created by Zero on 2023/9/3.
//

#pragma once

#include "util.h"
#include "base/serial_object.h"
#include "base/mgr/global.h"

namespace vision {
/**
 * generate initial candidates
 * evaluate visibility for initial candidates
 * temporal reuse
 * spatial reuse and iterate
 */
class ReSTIRDI : public SerialObject, public Ctx {
private:
    uint M{};
    Buffer<Reservoir> _reservoirs;
    Buffer<Reservoir> _prev_reservoirs;

    /**
     * generate initial candidates
     * check visibility
     * temporal reuse
     */
    Shader<void(uint)> _pass1;
    /**
     * spatial reuse and shading
     */
    Shader<void(uint)> _pass2;

public:
    explicit ReSTIRDI(uint M) : M(M) {}
    void compile() noexcept;
};

}// namespace vision