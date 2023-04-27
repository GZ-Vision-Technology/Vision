//
// Created by Zero on 07/03/2023.
//

#include "base/shader_graph/shader_node.h"

namespace vision {
class NumberInput : public ShaderNode {
private:
    Serialize<vector<float>> _value;

public:
    explicit NumberInput(const ShaderNodeDesc &desc)
        : ShaderNode(desc), _value(desc["value"].as_vector<float>()) {}
    OC_SERIALIZABLE_FUNC(_value)
    [[nodiscard]] bool is_zero() const noexcept override { return false; }
    [[nodiscard]] bool is_constant() const noexcept override { return false; }
    [[nodiscard]] uint dim() const noexcept override { return _value.element_num(); }
    [[nodiscard]] bool is_uniform() const noexcept override { return true; }
    [[nodiscard]] uint64_t _compute_hash() const noexcept override {
        return hash64_list(_value.hv());
    }
    [[nodiscard]] Array<float> evaluate(const AttrEvalContext &ctx,
                                        const SampledWavelengths &swl,
                                        const DataAccessor<float> *da) const noexcept override {
        return da->read_dynamic_array<float>(_value.element_num());
    }
    [[nodiscard]] Array<float> evaluate(const AttrEvalContext &ctx,
                                        const SampledWavelengths &swl) const noexcept override {
        return _value.has_device_value() ?
                   _value.dv() :
                   Array<float>(_value.hv());
    }
};
}// namespace vision

VS_MAKE_CLASS_CREATOR(vision::NumberInput)