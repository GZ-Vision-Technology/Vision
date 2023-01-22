//
// Created by Zero on 21/12/2022.
//

#pragma once

#include "dsl/common.h"
#include "cie.h"
#include "base/node.h"

namespace vision {
using namespace ocarina;

class SampledWavelengths {
private:
    Array<float> _lambdas;
    Array<float> _pdfs;

public:
    explicit SampledWavelengths(uint dim) noexcept : _lambdas{dim}, _pdfs{dim} {}
    [[nodiscard]] auto lambda(const Uint &i) const noexcept { return _lambdas[i]; }
    [[nodiscard]] auto pdf(const Uint &i) const noexcept { return _pdfs[i]; }
    void set_lambda(const Uint &i, const Float &lambda) noexcept { _lambdas[i] = lambda; }
    void set_pdf(const Uint &i, const Float &pdf) noexcept { _pdfs[i] = pdf; }
    [[nodiscard]] uint dimension() const noexcept { return static_cast<uint>(_lambdas.size()); }
};

class SampledSpectrum {
private:
    Array<float> _values;

public:
    SampledSpectrum(uint n, const Float &value) noexcept
        : _values(n) {
        for (int i = 0; i < n; ++i) {
            _values[i] = value;
        }
    }
    explicit SampledSpectrum(uint n) noexcept : SampledSpectrum{n, 0.f} {}
    explicit SampledSpectrum(const Float3 value) noexcept : _values(3) {
        for (int i = 0; i < 3; ++i) {
            _values[i] = value[i];
        }
    }
    explicit SampledSpectrum(const Float4 value) noexcept : _values(4) {
        for (int i = 0; i < 4; ++i) {
            _values[i] = value[i];
        }
    }
    explicit SampledSpectrum(const Float &value) noexcept : SampledSpectrum{1u, value} {}
    explicit SampledSpectrum(float value) noexcept : SampledSpectrum{1u, value} {}
    [[nodiscard]] uint dimension() const noexcept {
        return static_cast<uint>(_values.size());
    }
    [[nodiscard]] Array<float> &values() noexcept { return _values; }
    [[nodiscard]] const Array<float> &values() const noexcept { return _values; }
    [[nodiscard]] Float &operator[](const Uint &i) noexcept {
        return dimension() == 1u ? _values[0u] : _values[i];
    }
    [[nodiscard]] Float operator[](const Uint &i) const noexcept {
        return dimension() == 1u ? _values[0u] : _values[i];
    }
    template<typename F>
    [[nodiscard]] auto map(F &&f) const noexcept {
        SampledSpectrum s{dimension()};
        for (auto i = 0u; i < dimension(); i++) {
            if constexpr (std::invocable<F, Float>) {
                s[i] = f((*this)[i]);
            } else {
                s[i] = f(i, (*this)[i]);
            }
        }
        return s;
    }
    template<typename T, typename F>
    [[nodiscard]] auto reduce(T &&initial, F &&f) const noexcept {
        auto r = eval(OC_FORWARD(initial));
        for (auto i = 0u; i < dimension(); i++) {
            if constexpr (std::invocable<F, Var<expr_value_t<decltype(r)>>, Float>) {
                r = f(r, (*this)[i]);
            } else {
                r = f(r, i, (*this)[i]);
            }
        }
        return r;
    }
    [[nodiscard]] Float sum() const noexcept {
        return reduce(0.f, [](auto r, auto x) noexcept { return r + x; });
    }
    [[nodiscard]] Float max() const noexcept {
        return reduce(0.f, [](auto r, auto x) noexcept {
            return ocarina::max(r, x);
        });
    }
    [[nodiscard]] Float min() const noexcept {
        return reduce(std::numeric_limits<float>::max(), [](auto r, auto x) noexcept {
            return ocarina::min(r, x);
        });
    }

    [[nodiscard]] Float average() const noexcept {
        return sum() * static_cast<float>(1.0 / dimension());
    }
    template<typename F>
    [[nodiscard]] Bool any(F &&f) const noexcept {
        return reduce(false, [&f](auto ans, auto value) noexcept { return ans || f(value); });
    }
    template<typename F>
    [[nodiscard]] Bool all(F &&f) const noexcept {
        return reduce(true, [&f](auto ans, auto value) noexcept { return ans && f(value); });
    }
    [[nodiscard]] Bool is_zero() const noexcept {
        return all([](auto x) noexcept { return x == 0.f; });
    }
    template<typename F>
    [[nodiscard]] Bool none(F &&f) const noexcept { return !any(std::forward<F>(f)); }
    [[nodiscard]] SampledSpectrum operator+() const noexcept {
        return map([](auto s) noexcept { return s; });
    }
    [[nodiscard]] SampledSpectrum operator-() const noexcept {
        return map([](auto s) noexcept { return -s; });
    }

#define VS_MAKE_SPECTRUM_OPERATOR(op)                                                                         \
    [[nodiscard]] SampledSpectrum operator op(const Float &rhs) const noexcept {                              \
        return map([rhs](const auto &lvalue) { return lvalue op rhs; });                                      \
    }                                                                                                         \
    [[nodiscard]] SampledSpectrum operator op(const SampledSpectrum &rhs) const noexcept {                    \
        OC_ERROR_IF_NOT(dimension() == 1 || rhs.dimension() == 1 || dimension() == rhs.dimension(),           \
                        "Invalid sampled spectrum");                                                          \
        SampledSpectrum s(ocarina::max(dimension(), rhs.dimension()));                                        \
        for (int i = 0; i < s.dimension(); ++i) {                                                             \
            s[i] = (*this)[i] op rhs[i];                                                                      \
        }                                                                                                     \
        return s;                                                                                             \
    }                                                                                                         \
    [[nodiscard]] friend SampledSpectrum operator op(const Float &lhs, const SampledSpectrum &rhs) noexcept { \
        return rhs.map([lhs](const Float &rvalue) { return lhs op rvalue; });                                 \
    }                                                                                                         \
    SampledSpectrum &operator op##=(const Float &rhs) noexcept {                                              \
        for (int i = 0; i < dimension(); ++i) {                                                               \
            (*this)[i] op## = rhs;                                                                            \
        }                                                                                                     \
        return *this;                                                                                         \
    }                                                                                                         \
    SampledSpectrum &operator op##=(const SampledSpectrum &rhs) noexcept {                                    \
        OC_ERROR_IF_NOT(dimension() == 1 || rhs.dimension() == 1 || dimension() == rhs.dimension(),           \
                        "Invalid sampled spectrum");                                                          \
        if (rhs.dimension() == 1u) {                                                                          \
            return *this op## = rhs[0u];                                                                      \
        }                                                                                                     \
        for (uint i = 0; i < dimension(); ++i) {                                                              \
            (*this)[i] op## = rhs[i];                                                                         \
        }                                                                                                     \
        return *this;                                                                                         \
    }

    VS_MAKE_SPECTRUM_OPERATOR(+)
    VS_MAKE_SPECTRUM_OPERATOR(-)
    VS_MAKE_SPECTRUM_OPERATOR(*)
    VS_MAKE_SPECTRUM_OPERATOR(/)
#undef VS_MAKE_SPECTRUM_OPERATOR
};

template<typename A, typename B, typename T>
requires std::disjunction_v<
    std::is_same<std::remove_cvref_t<A>, SampledSpectrum>,
    std::is_same<std::remove_cvref_t<B>, SampledSpectrum>,
    std::is_same<std::remove_cvref_t<T>, SampledSpectrum>>
[[nodiscard]] auto lerp(T &&t, A &&a, B &&b) noexcept {
    return t * (b - a) + a;
}

[[nodiscard]] SampledSpectrum select(const SampledSpectrum &p, const SampledSpectrum &t, const SampledSpectrum &f) noexcept;
[[nodiscard]] SampledSpectrum select(const SampledSpectrum &p, const Float &t, const SampledSpectrum &f) noexcept;
[[nodiscard]] SampledSpectrum select(const SampledSpectrum &p, const SampledSpectrum &t, const Float &f) noexcept;
[[nodiscard]] SampledSpectrum select(const Bool &p, const SampledSpectrum &t, const SampledSpectrum &f) noexcept;
[[nodiscard]] SampledSpectrum select(const Bool &p, const Float &t, const SampledSpectrum &f) noexcept;
[[nodiscard]] SampledSpectrum select(const Bool &p, const SampledSpectrum &t, const Float &f) noexcept;

[[nodiscard]] SampledSpectrum zero_if_any_nan(const SampledSpectrum &t) noexcept;

// some math functions
[[nodiscard]] SampledSpectrum saturate(const SampledSpectrum &t) noexcept;
[[nodiscard]] SampledSpectrum abs(const SampledSpectrum &t) noexcept;
[[nodiscard]] SampledSpectrum sqrt(const SampledSpectrum &t) noexcept;

class Sampler;
class Spectrum : public Node {
public:
    using Desc = SpectrumDesc;
    struct Decode {
        SampledSpectrum value;
        Float strength;
        [[nodiscard]] static auto constant(uint dim, float value) noexcept {
            return Decode{.value = {dim, value}, .strength = value};
        }
        [[nodiscard]] static auto one(uint dim) noexcept { return constant(dim, 1.f); }
        [[nodiscard]] static auto zero(uint dim) noexcept { return constant(dim, 0.f); }
    };

public:
    explicit Spectrum(const SpectrumDesc &desc) : Node(desc) {}
    [[nodiscard]] virtual SampledWavelengths sample_wavelength(Sampler *sampler) const noexcept = 0;
    [[nodiscard]] virtual uint dimension() const noexcept { return 3; }
    [[nodiscard]] virtual Float4 srgb(const SampledSpectrum &sp, const SampledWavelengths &swl) const noexcept = 0;
    [[nodiscard]] virtual Float4 decode_to_albedo(Float3 rgb) const noexcept = 0;
    [[nodiscard]] virtual Float4 decode_to_illumination(Float3 rgb) const noexcept = 0;
};

}// namespace vision