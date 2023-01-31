//
// Created by Zero on 21/12/2022.
//

#include "srgb2spec.h"
#include "base/color/spectrum.h"
#include "base/color/spd.h"
#include "base/mgr/render_pipeline.h"

namespace vision {

template<EPort p = EPort::D>
[[nodiscard]] oc_float<p> sample_visible_wavelength_impl(const oc_float<p> &u) noexcept {
    return 538 - 138.888889f * atanh(0.85691062f - 1.82750197f * u);
}
VS_MAKE_CALLABLE(sample_visible_wavelength)

template<EPort p = EPort::D>
[[nodiscard]] oc_float<p> visible_wavelength_PDF_impl(const oc_float<p> &lambda) noexcept {
    return 0.0039398042f / sqr(cosh(0.0072f * (lambda - 538)));
}
VS_MAKE_CALLABLE(visible_wavelength_PDF)

class RGBSigmoidPolynomial {
private:
    array<Float, 3> _c;

private:
    [[nodiscard]] static Float _s(Float x) noexcept {
        return select(isinf(x), cast<float>(x > 0.0f),
                      0.5f * fma(x, rsqrt(fma(x, x, 1.f)), 1.f));
    }

    [[nodiscard]] Float _f(Float lambda) const noexcept {
        return fma(fma(_c[0], lambda, _c[1]), lambda, _c[2]);
    }

public:
    RGBSigmoidPolynomial() noexcept = default;
    RGBSigmoidPolynomial(Float c0, Float c1, Float c2) noexcept
        : _c{c0, c1, c2} {}
    explicit RGBSigmoidPolynomial(Float3 c) noexcept : _c{c[0], c[1], c[2]} {}
    [[nodiscard]] Float operator()(Float lambda) const noexcept {
        return _s(_f(lambda));// c0 * x * x + c1 * x + c2
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
                Float3 coord = make_float3(x, y, zz);
                coord = dsl::fma(
                    coord,
                    make_float3((res - 1.0f) / res),
                    make_float3(0.5f / res));
                c = array.tex(base_index + maxc).sample<float4>(coord).xyz();
            };
            return c;
        };
        return make_float4(decode(_rp->bindless_array().var(), _base_index, rgb),
                           cie::linear_srgb_to_y(rgb));
    }

    [[nodiscard]] Float4 decode_unbound(const Float3 &rgb_in) const noexcept {
        Float3 rgb = max(rgb_in, make_float3(0.f));
        Float m = max_comp(rgb);
        Float scale = 2.f * m;
        Float4 c = decode_albedo(select(scale == 0.f, make_float3(0.f), rgb / scale));
        return make_float4(c.xyz(), scale);
    }
};

class RGBAlbedoSpectrum {
private:
    RGBSigmoidPolynomial _rsp;

public:
    explicit RGBAlbedoSpectrum(RGBSigmoidPolynomial rsp) noexcept : _rsp{move(rsp)} {}
    [[nodiscard]] Float sample(const Float &lambda) const noexcept { return _rsp(lambda); }
};

class RGBUnboundSpectrum {
private:
    RGBSigmoidPolynomial _rsp;
    Float _scale;

public:
    explicit RGBUnboundSpectrum(RGBSigmoidPolynomial rsp, Float scale) noexcept
        : _rsp{move(rsp)}, _scale(scale) {}
    explicit RGBUnboundSpectrum(const Float4 &c) noexcept
        : RGBUnboundSpectrum(RGBSigmoidPolynomial(c.xyz()), c.w) {}
    [[nodiscard]] Float sample(const Float &lambda) const noexcept {
        return _rsp(lambda) * _scale;
    }
};

class RGBIlluminationSpectrum {
private:
    RGBSigmoidPolynomial _rsp;
    Float _scale;
    const SPD &_illuminant;

public:
    explicit RGBIlluminationSpectrum(RGBSigmoidPolynomial rsp, Float scale, const SPD &wp) noexcept
        : _rsp{move(rsp)}, _scale(scale), _illuminant(wp) {}
    explicit RGBIlluminationSpectrum(const Float4 &c, const SPD &wp) noexcept
        : RGBIlluminationSpectrum(RGBSigmoidPolynomial(c.xyz()), c.w, wp) {}
    [[nodiscard]] Float sample(const Float &lambda) const noexcept {
        return _rsp(lambda) * _scale * _illuminant.sample(lambda);
    }
};

class HeroWavelengthSpectrum : public Spectrum {
private:
    uint _dimension{};
    SPD _illuminant_d65;
    SPD _cie_x;
    SPD _cie_y;
    SPD _cie_z;
    RGBToSpectrumTable _rgb_to_spectrum_table;

public:
    explicit HeroWavelengthSpectrum(const SpectrumDesc &desc)
        : Spectrum(desc), _dimension(desc.dimension),
          _rgb_to_spectrum_table(sRGBToSpectrumTable_Data, render_pipeline()),
          _illuminant_d65(SPD::create_cie_d65(render_pipeline())),
          _cie_x(SPD::create_cie_x(render_pipeline())),
          _cie_y(SPD::create_cie_y(render_pipeline())),
          _cie_z(SPD::create_cie_z(render_pipeline())) {
        _rgb_to_spectrum_table.init();
    }

    void prepare() noexcept override {
        _illuminant_d65.prepare();
        _rgb_to_spectrum_table.prepare();
        _cie_x.prepare();
        _cie_y.prepare();
        _cie_z.prepare();
    }
    [[nodiscard]] uint dimension() const noexcept override { return _dimension; }
    [[nodiscard]] Float3 linear_srgb(const SampledSpectrum &sp, const SampledWavelengths &swl) const noexcept override {
        return cie::xyz_to_linear_srgb(cie_xyz(sp, swl));
    }

    void test(const SampledWavelengths &) noexcept override {
        Sampler *sampler = _scene->sampler();
        Float3 rgb = make_float3(0.63, 0.065, 0.05);
        Float4 c = _rgb_to_spectrum_table.decode_albedo(rgb);
        RGBAlbedoSpectrum spec(RGBSigmoidPolynomial{c.xyz()});
        int n = 500;

        Float3 output = make_float3(0.f);
        SampledWavelengths swl{dimension()};
        $for(i, n) {
            swl = sample_wavelength(sampler);
            SampledSpectrum sp{1u, spec.sample(swl.lambda(0u))};
            Float3 cl = linear_srgb(sp, swl);
            output += cl;
        };
        output /= float(n);
        prints("rgb {}, {}, {} xyz {} {} {}  c {} {} {}", output, cie::linear_srgb_to_xyz(output), c.xyz());
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
        Float4 c = _rgb_to_spectrum_table.decode_albedo(rgb);
        RGBAlbedoSpectrum spec(RGBSigmoidPolynomial{c.xyz()});
        SampledSpectrum sp{dimension()};
        for (uint i = 0; i < dimension(); ++i) {
            sp[i] = spec.sample(swl.lambda(i));
        }
        return {.sample = sp, .strength = luminance(rgb)};
    }
    [[nodiscard]] ColorDecode decode_to_illumination(Float3 rgb, const SampledWavelengths &swl) const noexcept override {
        Float4 c = _rgb_to_spectrum_table.decode_unbound(rgb);
        RGBIlluminationSpectrum spec{c, _illuminant_d65};
        SampledSpectrum sp{dimension()};
        for (uint i = 0; i < dimension(); ++i) {
            sp[i] = spec.sample(swl.lambda(i));
        }
        return {.sample = sp, .strength = luminance(rgb)};
    }
    [[nodiscard]] ColorDecode decode_to_unbound_spectrum(Float3 rgb, const SampledWavelengths &swl) const noexcept override {
        Float4 c = _rgb_to_spectrum_table.decode_unbound(rgb);
        RGBUnboundSpectrum spec{c};
        SampledSpectrum sp{dimension()};
        for (uint i = 0; i < dimension(); ++i) {
            sp[i] = spec.sample(swl.lambda(i));
        }
        return {.sample = sp, .strength = luminance(rgb)};
    }
};

}// namespace vision

VS_MAKE_CLASS_CREATOR(vision::HeroWavelengthSpectrum)