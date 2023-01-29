//
// Created by Zero on 21/12/2022.
//

#include "srgb2spec.h"
#include "base/color/spectrum.h"
#include "base/color/spd.h"
#include "base/mgr/render_pipeline.h"

namespace vision {

class RGBSigmoidPolynomial {
private:
    array<Float, 3> _c;

private:
    [[nodiscard]] static Float _s(Float x) noexcept {
        return select(isinf(x), cast<float>(x > 0.0f),
                      0.5f * fma(x, rsqrt(fma(x, x, 1.f)), 1.f));
    }

public:
    RGBSigmoidPolynomial() noexcept = default;
    RGBSigmoidPolynomial(Float c0, Float c1, Float c2) noexcept
        : _c{c0, c1, c2} {}
    explicit RGBSigmoidPolynomial(Float3 c) noexcept : _c{c[0], c[1], c[2]} {}
    [[nodiscard]] Float operator()(Float lambda) const noexcept {
        return _s(fma(lambda, fma(lambda, _c[0], _c[1]), _c[2]));// c0 * x * x + c1 * x + c2
    }
};

class RGBToSpectrumTable {
public:
    static constexpr auto res = 64u;
    using coefficient_table_type = const float[3][res][res][res][4];

private:
    const coefficient_table_type &_coefficients;
    RenderPipeline *_rp{};
    uint _base_index{InvalidUI32};
    RHITexture _coefficient0;
    RHITexture _coefficient1;
    RHITexture _coefficient2;

private:
    [[nodiscard]] inline static auto _inverse_smooth_step(auto x) noexcept {
        return 0.5f - sin(asin(1.0f - 2.0f * x) * (1.0f / 3.0f));
    }

public:
    explicit RGBToSpectrumTable(const coefficient_table_type &coefficients, RenderPipeline *rp) noexcept
        : _coefficients{coefficients}, _rp(rp) {}

    void init() noexcept {
        _coefficient0 = _rp->device().create_texture(make_uint3(res), PixelStorage::FLOAT4);
        _coefficient1 = _rp->device().create_texture(make_uint3(res), PixelStorage::FLOAT4);
        _coefficient2 = _rp->device().create_texture(make_uint3(res), PixelStorage::FLOAT4);
    }

    void prepare() noexcept {
        _base_index = _rp->register_texture(_coefficient0);
        _rp->register_texture(_coefficient1);
        _rp->register_texture(_coefficient2);
        _coefficient0.upload_immediately(&_coefficients[0]);
        _coefficient1.upload_immediately(&_coefficients[1]);
        _coefficient2.upload_immediately(&_coefficients[2]);
    }

    [[nodiscard]] Float4 decode_albedo(const Float3 &rgb_in) const noexcept {
        Float3 rgb = clamp(rgb_in, make_float3(0.f), make_float3(1.f));
        static Callable decode = [](Var<BindlessArray> array, Uint base_index, Float3 rgb) noexcept -> Float3 {
            Float3 c = make_float3(0.0f, 0.0f, (rgb[0] - 0.5f) * rsqrt(rgb[0] * (1.0f - rgb[0])));
            $if(!(rgb[0] == rgb[1] & rgb[1] == rgb[2])) {
                Uint maxc = select(
                    rgb[0] > rgb[1],
                    select(rgb[0] > rgb[2], 0u, 2u),
                    select(rgb[1] > rgb[2], 1u, 2u));
                Float z = rgb[maxc];
                Float x = rgb[(maxc + 1u) % 3u] / z;
                Float y = rgb[(maxc + 2u) % 3u] / z;
                Float zz = _inverse_smooth_step(_inverse_smooth_step(z));

                Float3 coord = dsl::fma(
                    make_float3(x, y, zz),
                    make_float3((res - 1.0f) / res),
                    make_float3(0.5f / res));
                c = array.tex(base_index + maxc).sample<float4>(coord).xyz();
            };
            return c;
        };
        return make_float4(decode(_rp->bindless_array().var(), _base_index, rgb), 1.f);
    }
};

class RGBAlbedoSpectrum {
private:
    RGBSigmoidPolynomial _rsp;

public:
    explicit RGBAlbedoSpectrum(RGBSigmoidPolynomial rsp) noexcept : _rsp{move(rsp)} {}
    [[nodiscard]] Float sample(const Float &lambda) const noexcept { return _rsp(lambda); }
};

class RGBAIlluminationSpectrum {
private:
    RGBSigmoidPolynomial _rsp;
    Float _scale;

public:
    explicit RGBAIlluminationSpectrum(RGBSigmoidPolynomial rsp) noexcept : _rsp{move(rsp)} {}
    [[nodiscard]] Float sample(const Float &lambda) const noexcept { return _rsp(lambda); }
};

class HeroWavelengthSpectrum : public Spectrum {
private:
    uint _dimension{};
    SPD _white_point;
    SPD _cie_x;
    SPD _cie_y;
    SPD _cie_z;
    RGBToSpectrumTable _rgb_to_spectrum_table;

public:
    explicit HeroWavelengthSpectrum(const SpectrumDesc &desc)
        : Spectrum(desc), _dimension(3),
          _rgb_to_spectrum_table(sRGBToSpectrumTable_Data, render_pipeline()),
          _white_point(SPD::create_cie_d65(render_pipeline())),
          _cie_x(SPD::create_cie_x(render_pipeline())),
          _cie_y(SPD::create_cie_y(render_pipeline())),
          _cie_z(SPD::create_cie_z(render_pipeline())) {
        _rgb_to_spectrum_table.init();
    }

    void prepare() noexcept override {
        _white_point.prepare();
        _rgb_to_spectrum_table.prepare();
        _cie_x.prepare();
        _cie_y.prepare();
        _cie_z.prepare();
    }
    [[nodiscard]] uint dimension() const noexcept override { return _dimension; }
    [[nodiscard]] Float3 linear_srgb(const SampledSpectrum &sp, const SampledWavelengths &swl) const noexcept override {
        return cie::xyz_to_linear_srgb(cie_xyz(sp, swl));
    }
    [[nodiscard]] Float cie_y(const SampledSpectrum &sp, const SampledWavelengths &swl) const noexcept override {
        Float sum = 0.f;

        constexpr auto safe_div = [](const Float &a, const Float &b) noexcept {
            return select(b == 0.0f, 0.0f, a / b);
        };

        for (uint i = 0; i < sp.dimension(); ++i) {
            sum += safe_div(_cie_y.sample(swl.lambda(i)) * sp[i], swl.pdf(i));
        }
        float factor = 1.f / (swl.dimension() * SPD::cie_y_integral());
        return sum * factor;
    }
    [[nodiscard]] Float3 cie_xyz(const SampledSpectrum &sp, const SampledWavelengths &swl) const noexcept override {
        Float3 sum = make_float3(0.f);
        constexpr auto safe_div = [](const Float &a, const Float &b) noexcept {
            return select(b == 0.0f, 0.0f, a / b);
        };
        for (uint i = 0; i < sp.dimension(); ++i) {
            sum += make_float3(safe_div(_cie_x.sample(swl.lambda(i)) * sp[i], swl.pdf(i)),
                               safe_div(_cie_y.sample(swl.lambda(i)) * sp[i], swl.pdf(i)),
                               safe_div(_cie_z.sample(swl.lambda(i)) * sp[i], swl.pdf(i)));
        }
        float factor = 1.f / (swl.dimension() * SPD::cie_y_integral());
        return sum * factor;
    }
    [[nodiscard]] SampledWavelengths sample_wavelength(Sampler *sampler) const noexcept override {
        uint n = dimension();
        SampledWavelengths swl{n};
        Float u = sampler->next_1d();
        for (uint i = 0; i < n; ++i) {
            float offset = static_cast<float>(i * (1.f / n));
            Float up = fract(u + offset);
            Float lambda = sample_visible_wavelength(up);
            swl.set_lambda(i, lambda);
            swl.set_pdf(i, visible_wavelength_PDF(lambda));
        }
        return swl;
    }
    [[nodiscard]] ColorDecode decode_to_albedo(Float3 rgb, const SampledWavelengths &swl) const noexcept override {
        // todo
        return {.sample = SampledSpectrum(rgb), .strength = luminance(rgb)};
    }
    [[nodiscard]] ColorDecode decode_to_illumination(Float3 rgb, const SampledWavelengths &swl) const noexcept override {
        // todo
        return {.sample = SampledSpectrum(rgb), .strength = luminance(rgb)};
    }
    [[nodiscard]] ColorDecode decode_to_unbound_spectrum(Float3 rgb, const SampledWavelengths &swl) const noexcept override {
        // todo
        return {.sample = SampledSpectrum(rgb), .strength = luminance(rgb)};
    }
};

}// namespace vision

VS_MAKE_CLASS_CREATOR(vision::HeroWavelengthSpectrum)