//
// Created by Zero on 09/09/2022.
//

#pragma once

#include "base/node.h"
#include "util/image_io.h"
#include "base/color/spectrum.h"
#include "base/scattering/interaction.h"

namespace vision {

class ShaderNode : public Node {
protected:
    ShaderNodeType _type{};
    uint _dim{4};

public:
    using Desc = ShaderNodeDesc;

public:
    explicit ShaderNode(const ShaderNodeDesc &desc) : Node(desc), _type(desc.type) {}
    [[nodiscard]] virtual uint dim() const noexcept { return _dim; }
    [[nodiscard]] virtual bool is_zero() const noexcept { return false; }
    /**
     * if shader node is constant, the result will be inlined
     * @return
     */
    [[nodiscard]] virtual bool is_constant() const noexcept { return false; }
    /**
     * if shader node contain textures,the result is versatile
     * @return
     */
    [[nodiscard]] virtual bool is_uniform() const noexcept { return false; }
    [[nodiscard]] virtual Array<float> evaluate(const AttrEvalContext &ctx) const noexcept = 0;
    virtual void for_each_pixel(const function<ImageIO::foreach_signature> &func) const noexcept {
        OC_ERROR("call error");
    }
    [[nodiscard]] virtual uint2 resolution() const noexcept { return make_uint2(0); }
};

class Slot : public ocarina::Hashable {
private:
    const ShaderNode *_node{};
    uint _dim{4};
    uint _channel_mask{};

private:
    [[nodiscard]] uint _calculate_mask(string channels) noexcept;
    [[nodiscard]] uint64_t _compute_hash() const noexcept override { return hash64(_channel_mask, _node->hash()); }
    [[nodiscard]] uint64_t _compute_type_hash() const noexcept override { return hash64(_channel_mask, _node->type_hash()); }

public:
    Slot() = default;
    explicit Slot(const ShaderNode *input, string channels)
        : _node(input),
          _dim(channels.size()),
          _channel_mask(_calculate_mask(channels)) {
        OC_ASSERT(_dim <= 4);
    }

    [[nodiscard]] uint dim() const noexcept { return _dim; }
    [[nodiscard]] bool is_zero() const noexcept { return _node->is_zero(); }
    [[nodiscard]] bool is_constant() const noexcept { return _node->is_constant(); }
    [[nodiscard]] bool is_uniform() const noexcept { return _node->is_uniform(); }
    [[nodiscard]] Array<float> evaluate(const AttrEvalContext &ctx) const noexcept;
    [[nodiscard]] ColorDecode eval_albedo_spectrum(const AttrEvalContext &ctx,
                                                   const SampledWavelengths &swl) const noexcept;

    [[nodiscard]] ColorDecode eval_unbound_spectrum(const AttrEvalContext &ctx,
                                                    const SampledWavelengths &swl) const noexcept;

    [[nodiscard]] ColorDecode eval_illumination_spectrum(const AttrEvalContext &ctx,
                                                         const SampledWavelengths &swl) const noexcept;
    [[nodiscard]] auto node() const noexcept { return _node; }
    [[nodiscard]] auto node() noexcept { return _node; }
};

}// namespace vision