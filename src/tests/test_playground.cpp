//
// Created by Zero on 15/09/2022.
//

#include <math/box.h>
#include "core/stl.h"
#include "rhi/common.h"
#include "core/string_util.h"
#include "math/transform.h"
#include "math/geometry.h"
#include "math/constants.h"
#include "plugins/spectra/srgb2spec.h"
#include "base/color/spd.h"

using namespace vision;
using namespace ocarina;

float eta(float lambda) {
    auto f = 2.00029547f * sqr(lambda) / (sqr(lambda) - 0.0121426017f) +
        0.298926886f * sqr(lambda) / (sqr(lambda) - 0.0538736236f) +
        1.80691843f * sqr(lambda) / (sqr(lambda) - 156.530829f);
    return sqrt(f + 1);
}

int main(int argc, char *argv[]) {

    float e = eta(0.83);
    float e2 = eta(0.36);
    return 0;
}