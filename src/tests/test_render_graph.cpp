//
// Created by Zero on 2023/6/27.
//

#include <math/box.h>
#include "core/stl.h"
#include "rhi/common.h"
#include "render_graph/graph.h"

using namespace vision;
using namespace ocarina;

namespace vision {

class RTPass : public RenderPass {
private:
    static constexpr auto kOutput = "radiance";
    static constexpr auto kNormal = "normal";
public:

};

class AccumulatePass : public RenderPass {
private:
    static constexpr auto kInput = "input";
    static constexpr auto kOutput = "output";

public:

};

class DenoisePass : public RenderPass {
private:
    static constexpr auto kRadiance = "radiance";
    static constexpr auto kNormal = "normal";
    static constexpr auto kOutput = "output";
};

class TonemappingPass : public RenderPass {
private:
    static constexpr auto kInput = "input";
    static constexpr auto kOutput = "output";
public:

};

}// namespace vision

int main(int argc, char *argv[]) {

    RenderGraph graph;

    auto rt = new RTPass();
    auto r_accum = new AccumulatePass();
    auto denoise = new DenoisePass();
    auto tonemapping = new TonemappingPass();

    graph.add_pass(rt, "rt");
    graph.add_pass(r_accum, "accum");
    graph.add_pass(denoise, "denoise");
    graph.add_pass(tonemapping, "tonemapping");

    graph.build_connection("rt.radiance", "accum.input");
    graph.build_connection("accum.output", "denoise.radiance");
    graph.build_connection("denoise.output", "tonemapping.input");
    graph.mark_output("tonemapping.output");

    return 0;
}
