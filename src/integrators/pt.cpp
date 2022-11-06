//
// Created by Zero on 12/09/2022.
//

#include "base/integrator.h"
#include "core/render_pipeline.h"
#include "math/warp.h"

namespace vision {
using namespace ocarina;
class PathTracingIntegrator : public Integrator {
public:
    explicit PathTracingIntegrator(const IntegratorDesc &desc)
        : Integrator(desc) {
        _mis_weight = mis_weight<D>;
    }

    void compile_shader(RenderPipeline *rp) noexcept override {
        Camera *camera = rp->scene().camera();
        Sampler *sampler = rp->scene().sampler();
        LightSampler *light_sampler = rp->scene().light_sampler();

        _kernel = [&](Uint frame_index) -> void {
            Uint2 pixel = dispatch_idx().xy();
            Bool debug = all(pixel == make_uint2(508, 66));
            sampler->start_pixel_sample(pixel, frame_index, 0);
            SensorSample ss = sampler->sensor_sample(pixel);
            ss.p_film = make_float2(pixel);
            RaySample rs = camera->generate_ray(ss);
            Var ray = rs.ray;
            Float bsdf_pdf = eval(1e16f);
            Float3 Li = make_float3(0.f);
            Float3 throughput = make_float3(1.f);

            $for(bounces, 0, _max_depth) {
                Var hit = rp->trace_closest(ray);
                comment("miss");
                $if(hit->is_miss()) {
                    $break;
                };

                auto si = rp->compute_surface_interaction(hit);
                si.wo = normalize(-ray->direction());

                comment("hit light");
                $if(si.has_emission()) {
                    LightSampleContext p_ref;
                    p_ref.pos = ray->origin();
                    p_ref.ng = ray->direction();
                    LightEval eval = light_sampler->evaluate_hit(p_ref, si);
                    Float weight = _mis_weight(bsdf_pdf, eval.pdf);
                    Li += eval.L * throughput * weight;
                };

                comment("estimate direct lighting");
                comment("sample light");
                LightSample light_sample = light_sampler->sample(si, sampler->next_1d(), sampler->next_2d());
                OCRay shadow_ray = si.spawn_ray_to(light_sample.p_light);
                Bool occluded = rp->trace_any(shadow_ray);

                comment("sample bsdf");
                BSDFSample bsdf_sample;
                rp->dispatch<Material>(si.mat_id, rp->scene().materials(),
                                       [&](const Material *material) {
                    UP<BSDF> bsdf = material->get_BSDF(si);

                    BSDFEval bsdf_eval;
                    Float3 wi = normalize(light_sample.p_light - si.pos);
                    bsdf_eval = bsdf->evaluate(si.wo, wi, BxDFFlag::All);
                    bsdf_sample = bsdf->sample(si.wo, sampler->next_1d(), sampler->next_2d(), BxDFFlag::All);
                    Float3 w = bsdf_sample.wi;
                    Float3 f = bsdf_sample.eval.f;
                    Float weight = _mis_weight(light_sample.eval.pdf, bsdf_eval.pdf);
                    $if(!occluded && bsdf_eval.valid() && light_sample.valid()) {
                        Float3 Ld = light_sample.eval.L * bsdf_eval.f * weight / light_sample.eval.pdf;
                        Li += throughput * Ld;
                    };

                });
                Float lum = luminance(throughput);
                $if(!bsdf_sample.valid() || lum == 0.f) {
                    $break;
                };
                throughput *= bsdf_sample.eval.f / bsdf_sample.eval.pdf;
                $if(lum < _rr_threshold && bounces >= _min_depth) {
                    Float q = min(0.95f, lum);
                    Float rr = sampler->next_1d();
                    $if(q < rr) {
                        $break;
                    };
                    throughput /= q;
                };
                bsdf_pdf = bsdf_sample.eval.pdf;
                ray = si.spawn_ray(bsdf_sample.wi);
            };
            camera->film()->add_sample(pixel, Li, frame_index);
        };
        _shader = rp->device().compile(_kernel);
    }

    void render(RenderPipeline *rp) const noexcept override {
        Stream &stream = rp->stream();
        stream << _shader(rp->frame_index()).dispatch(rp->resolution());
        stream << synchronize();
        stream << commit();
    }
};
}// namespace vision

VS_MAKE_CLASS_CREATOR(vision::PathTracingIntegrator)