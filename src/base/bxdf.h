//
// Created by Zero on 28/10/2022.
//

#pragma once

#include "dsl/common.h"
#include "sample.h"

namespace vision {
using namespace ocarina;
struct BxDFSample {
    Float3 val;
    Float pdf{-1.f};
    Float3 wi;
    Uchar flags;
    [[nodiscard]] Bool valid() const noexcept {
        return pdf >= 0.f;
    }
};

struct BxDFFlag {
    static constexpr uchar Unset = 1;
    static constexpr uchar Reflection = 1 << 1;
    static constexpr uchar Transmission = 1 << 2;
    static constexpr uchar Diffuse = 1 << 3;
    static constexpr uchar Glossy = 1 << 4;
    static constexpr uchar Specular = 1 << 5;
    static constexpr uchar NearSpec = 1 << 6;
    // Composite _BxDFFlags_ definitions
    static constexpr uchar DiffRefl = Diffuse | Reflection;
    static constexpr uchar DiffTrans = Diffuse | Transmission;
    static constexpr uchar GlossyRefl = Glossy | Reflection;
    static constexpr uchar GlossyTrans = Glossy | Transmission;
    static constexpr uchar SpecRefl = Specular | Reflection;
    static constexpr uchar SpecTrans = Specular | Transmission;
    static constexpr uchar All = Diffuse | Glossy | Specular | Reflection | Transmission | NearSpec;
};

class BxDF {
protected:
    Uchar _flag;

public:
    explicit BxDF(Uchar flag) : _flag(flag) {}
    [[nodiscard]] virtual Float PDF(Float3 wo, Float3 wi) const noexcept;
    [[nodiscard]] virtual Float3 f(Float3 wo, Float3 wi) const noexcept = 0;
    [[nodiscard]] virtual Bool safe(Float3 wo, Float3 wi) const noexcept;
    [[nodiscard]] virtual Evaluation evaluate(Float3 wo, Float3 wi) const noexcept;
    [[nodiscard]] virtual Evaluation safe_evaluate(Float3 wo, Float3 wi) const noexcept;
    [[nodiscard]] virtual BxDFSample sample(Float3 wo, Float2 u) const noexcept;
    [[nodiscard]] Uchar flag() const noexcept { return _flag; }
    [[nodiscard]] Bool match_flag(Uchar bxdf_flag) const noexcept {
        return ((_flag & bxdf_flag) == _flag);
    }
};

class LambertReflection : public BxDF {
private:
    Float3 Kr;

public:
    explicit LambertReflection(Float3 kr)
        : BxDF(select(is_zero(kr), BxDFFlag::Unset, BxDFFlag::DiffRefl)),
          Kr(kr) {}
    [[nodiscard]] Float3 f(Float3 wo, Float3 wi) const noexcept override;
};

}// namespace vision