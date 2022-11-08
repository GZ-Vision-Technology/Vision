//
// Created by Zero on 07/11/2022.
//

#pragma once

#include "core/basic_types.h"
#include "dsl/common.h"
#include "math/base.h"
#include "math/geometry.h"

namespace vision {
using namespace ocarina;
enum MicrofacetType : uint8_t {
    GGX,
    Disney,
    Beckmann,
};

inline namespace microfacet {

template<EPort p = D>
[[nodiscard]] oc_float<p> roughness_to_alpha(oc_float<p> roughness) {
    roughness = max(roughness, oc_float<p>(1e-3));
    oc_float<p> x = log(roughness);
    return 1.62142f +
           0.819955f * x +
           0.1734f * Pow<2>(x) +
           0.0171201f * Pow<3>(x) +
           0.000640711f * Pow<4>(x);
}

/**
 *  beckmann
 *
 *             e^[-(tan_theta_h)^2 ((cos_theta_h)^2/ax^2 + (sin_theta_h)^2/ay^2)]
 * D(wh) = -------------------------------------------------------------------------
 *                                PI ax ay (cos_theta_h)^4
 *
 *  GGX
 *                                                    1
 * D(wh) = ---------------------------------------------------------------------------------------------------
 *             PI ax ay (cos_theta_h)^4 [1 + (tan_theta_h)^2 ((cos_theta_h)^2/ax^2 + (sin_theta_h)^2/ay^2)]^2
 *
 * from http://www.pbr-book.org/3ed-2018/Reflection_Models/Microfacet_Models.html
 *
 * @param wh
 * @return
 */
template<EPort p = D>
[[nodiscard]] oc_float<p> D_(const oc_float3<p> &wh, oc_float<p> alpha_x, oc_float<p> alpha_y, MicrofacetType type = GGX) {
    // When theta is close to 90, tan theta is infinity
    oc_float<p> tan_theta_2 = geometry::tan_theta_2(wh);
    oc_float<p> cos_theta_4 = sqr(geometry::cos_theta_2(wh));
    switch (type) {
        case Disney:
        case GGX: {
            oc_float<p> e = tan_theta_2 * (sqr(geometry::cos_phi(wh) / alpha_x) + sqr(geometry::sin_phi(wh) / alpha_y));
            oc_float<p> ret = 1.f / (Pi * alpha_x * alpha_y * cos_theta_4 * sqr(1 + e));
            return select(cos_theta_4 < 1e-16f || isinf(tan_theta_2), 0.f, ret);
        }
        case Beckmann: {
            oc_float<p> ret = exp(-tan_theta_2 * (geometry::cos_phi_2(wh) / sqr(alpha_x) +
                                                  geometry::sin_phi_2(wh) / sqr(alpha_y))) /
                              (Pi * alpha_x * alpha_y * cos_theta_4);
            return select(cos_theta_4 < 1e-16f || isinf(tan_theta_2), 0.f, ret);
        }
        default:
            break;
    }
    return 0;
}

/**
 * lambda(w) = A-(w) / (A+(w) - A-(w))
 * @param  w [description]
 * @return   [description]
 */
template<EPort p = EPort::D>
[[nodiscard]] oc_float<p> lambda(const oc_float3<p> &w, const oc_float<p> &alpha_x,
                                 const oc_float<p> &alpha_y, MicrofacetType type = GGX) {
    switch (type) {
        case Disney:
        case GGX: {
            oc_float<p> abs_tan_theta = abs(geometry::tan_theta(w));
            oc_float<p> cos_theta2 = geometry::cos_theta_2(w);
            oc_float<p> sin_theta2 = geometry::sin_theta_2(w);
            oc_float<p> alpha = sqrt(cos_theta2 * sqr(alpha_x) +
                                     sin_theta2 * sqr(alpha_y));
            oc_float<p> ret = (-1 + sqrt(1.f + sqr(alpha * abs_tan_theta))) / 2;
            return select(isinf(abs_tan_theta), 0.f, ret);
        }
        case Beckmann: {
            oc_float<p> abs_tan_theta = abs(geometry::tan_theta(w));

            oc_float<p> cos_theta2 = geometry::cos_theta_2(w);
            oc_float<p> sin_theta2 = geometry::sin_theta_2(w);

            oc_float<p> alpha = sqrt(cos_theta2 * sqr(alpha_x) +
                                     sin_theta2 * sqr(alpha_y));
            oc_float<p> a = 1.f / (alpha * abs_tan_theta);

            oc_float<p> ret = (1 - 1.259f * a + 0.396f * sqr(a)) / (3.535f * a + 2.181f * sqr(a));
            return select(a >= 1.6f || isinf(abs_tan_theta), 0.f, ret);
        }
        default:
            break;
    }
    return 0;
}

/**
 * smith occlusion function
 * G1(w) = 1 / (lambda(w) + 1)
 * @param  w [description]
 * @return   [description]
 */
template<EPort p = EPort::D>
[[nodiscard]] oc_float<p> G1(const oc_float3<p> &w, oc_float<p> alpha_x, oc_float<p> alpha_y, MicrofacetType type = GGX) {
    oc_float<p> ret = 1 / (1 + lambda(w, alpha_x, alpha_y, type));
    return ret;
}

/**
 * G(wo, wi) = 1 / (lambda(wo) + lambda(wi) + 1)
 * @return   [description]
 */
template<EPort p = EPort::D>
[[nodiscard]] oc_float<p> G_(const oc_float3<p> &wo, const oc_float3<p> &wi, oc_float<p> alpha_x, oc_float<p> alpha_y, MicrofacetType type = GGX) {
    oc_float<p> ret = 0.f;
    switch (type) {
        case Disney: {
            ret = G1<p>(wi, alpha_x, alpha_y, type) * G1<p>(wo, alpha_x, alpha_y, type);
            return ret;
        }
        case GGX:
        case Beckmann: {
            ret = 1 / (1 + lambda<p>(wo, alpha_x, alpha_y, type) + lambda<p>(wi, alpha_x, alpha_y, type));
            return ret;
        }
        default:
            break;
    }
    return ret;
}

template<EPort p = EPort::D>
[[nodiscard]] oc_float3<p> sample_wh(const oc_float3<p> &wo, const oc_float2<p> &u,
                                     oc_float<p> alpha_x, oc_float<p> alpha_y, MicrofacetType type = GGX) {
    switch (type) {
        case Disney:
        case GGX: {
            oc_float<p> cos_theta = 0, phi = _2Pi * u[1];
            phi = atan(alpha_y / alpha_x * tan(_2Pi * u[1] + PiOver2));
            phi = select(u[1] > .5f, phi + Pi, phi);
            oc_float<p> sin_phi = sin(phi), cos_phi = cos(phi);
            oc_float<p> alpha2 = 1.f / (sqr(cos_phi / alpha_x) + sqr(sin_phi / alpha_y));
            oc_float<p> tan_theta_2 = alpha2 * u[0] / (1 - u[0]);
            cos_theta = 1 / sqrt(1 + tan_theta_2);
            oc_float<p> sin_theta = safe_sqrt(1 - sqr(cos_theta));
            oc_float3<p> wh = spherical_direction<p>(sin_theta, cos_theta, phi);
            wh = select(same_hemisphere(wo, wh), wh, -wh);
            return wh;
        }
        case Beckmann: {
            oc_float<p> tan_theta_2, phi;
            oc_float<p> log_sample = log(1 - u[0]);
            oc_assert(!isinf(log_sample));
            phi = atan(alpha_y / alpha_x *
                       tan(_2Pi * u[1] + PiOver2));
            phi = select(u[1] > .5f, phi + Pi, phi);
            oc_float<p> sin_phi = sin(phi), cos_phi = cos(phi);
            tan_theta_2 = -log_sample / (sqr(cos_phi / alpha_x) + sqr(sin_phi / alpha_y));
            oc_float<p> cos_theta = 1 / sqrt(1 + tan_theta_2);
            oc_float<p> sin_theta = safe_sqrt(1 - sqr(cos_theta));
            oc_float3<p> wh = spherical_direction<p>(sin_theta, cos_theta, phi);
            wh = select(same_hemisphere(wo, wh), wh, -wh);
            return wh;
        }
        default:
            break;
    }
    return {};
}

template<EPort p = EPort::D>
[[nodiscard]] oc_float<p> PDF_wh(const oc_float3<p> &wo, const oc_float3<p> &wh,
                                 const oc_float<p> &alpha_x, const oc_float<p> &alpha_y,
                                 MicrofacetType type = GGX) {
    return D_<p>(wh, alpha_x, alpha_y, type) * abs_cos_theta(wh);
}

/**
 * pwi(wi) = dwh / dwi * pwh(wh) = pwh(wh) / 4cos_theta_h
 * @param PDF_wh
 * @param wo
 * @param wh
 * @return
 */
template<EPort p = EPort::D>
[[nodiscard]] oc_float<p> PDF_wi_reflection(const oc_float<p> &PDF_wh,
                                            const oc_float3<p> &wo,
                                            const oc_float3<p> &wh) {
    oc_float<p> ret = PDF_wh / (4 * abs_dot(wo, wh));
    oc_assert(!invalid(ret), "invalid pdf reflection");
    return ret;
}

template<EPort p = EPort::D>
[[nodiscard]] oc_float<p> PDF_wi_reflection(const oc_float3<p> &wo, const oc_float3<p> &wh, const oc_float<p> &alpha_x,
                                            const oc_float<p> &alpha_y, MicrofacetType type = GGX) {
    return PDF_wi_reflection<p>(PDF_wh<p>(wo, wh, alpha_x, alpha_y, type), wo, wh);
}

/**
 * dwh  dwi
 *                   eta_i^2 |wi dot wh|
 * dwh/dwi = -----------------------------------------
 *            [eta_o(wh dot wo) + eta_i(wi dot wh)]^2
 * @tparam T
 * @param PDF_wh
 * @param eta eta_i / eta_o
 * @return
 */
template<EPort p = EPort::D>
[[nodiscard]] oc_float<p> PDF_wi_transmission(const oc_float<p> &PDF_wh, const oc_float3<p> &wo, const oc_float3<p> &wh,
                                              const oc_float3<p> &wi, const oc_float<p> &eta) {
    oc_float<p> denom = sqr(dot(wi, wh) * eta + dot(wo, wh));
    oc_float<p> dwh_dwi = abs_dot(wi, wh) / denom;
    oc_float<p> ret = PDF_wh * dwh_dwi;
    oc_assert(!invalid(ret), "invalid pdf transmission");
    return ret;
}

template<EPort p = EPort::D>
[[nodiscard]] oc_float3<p> BRDF(const oc_float3<p> &wo, const oc_float3<p> &wh, const oc_float3<p> &wi, oc_float3<p> Fr,
                                const oc_float<p> &cos_theta_i, const oc_float<p> &cos_theta_o,
                                const oc_float<p> &alpha_x, const oc_float<p> &alpha_y, MicrofacetType type = GGX) {
    auto ret = D_<p>(wh, alpha_x, alpha_y, type) * Fr * G_<p>(wo, wi, alpha_x, alpha_y, type) / abs(4 * cos_theta_o * cos_theta_i);
    oc_assert(!invalid(ret) && all(ret > 0.f), "invalid brdf !");
    return ret;
}

template<EPort p = EPort::D>
[[nodiscard]] oc_float3<p> BRDF(const oc_float3<p> &wo, const oc_float3<p> &wi, oc_float3<p> Fr,
                                const oc_float<p> &cos_theta_i, const oc_float<p> &cos_theta_o,
                                const oc_float<p> &alpha_x, const oc_float<p> &alpha_y, MicrofacetType type = GGX) {
    oc_float3<p> wh = normalize(wo + wi);
    return BRDF<p>(wo, wh, wi, Fr, cos_theta_i, cos_theta_o, alpha_x, alpha_y, type);
}

/**
 *
 * @param eta : eta_i / eta_o
 * @param mode
 * @return
 */
template<EPort p = EPort::D>
[[nodiscard]] oc_float3<p> BTDF(const oc_float3<p> &wo, const oc_float3<p> &wh, const oc_float3<p> &wi, const oc_float3<p> &Ft,
                                const oc_float<p> &cos_theta_i, const oc_float<p> &cos_theta_o, const oc_float<p> &eta,
                                const oc_float<p> &alpha_x, const oc_float<p> &alpha_y, MicrofacetType type = GGX) {
    oc_float3<p> numerator = D_<p>(wh, alpha_x, alpha_y, type) * Ft * G_<p>(wo, wi, alpha_x, alpha_y, type) *
                             abs(dot(wi, wh) * dot(wo, wh));
    oc_float<p> denom = sqr(dot(wi, wh) * eta + dot(wo, wh)) * abs(cos_theta_i * cos_theta_o);
    oc_float3<p> ft = numerator / denom;
    oc_float<p> factor = rcp(sqr(eta));
    oc_assert(!invalid(ft) && all(ft > 0.f), "invalid btdf !");
    return ft * factor;
}

/**
 *
 * @param eta : eta_i / eta_o
 * @param mode
 * @return
 */
template<EPort p = EPort::D>
[[nodiscard]] oc_float3<p> BTDF(const oc_float3<p> &wo, const oc_float3<p> &wi, const oc_float3<p> &Ft,
                                const oc_float<p> &cos_theta_i, const oc_float<p> &cos_theta_o, const oc_float<p> &eta,
                                const oc_float<p> &alpha_x, const oc_float<p> &alpha_y, MicrofacetType type = GGX) {
    oc_float3<p> wh = normalize(wo + wi * eta);
    return BTDF<p>(wo, wh, wi, Ft, cos_theta_i, cos_theta_o, eta, alpha_x, alpha_y, type);
}

}// namespace microfacet

}// namespace vision