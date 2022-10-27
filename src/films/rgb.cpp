//
// Created by Zero on 13/10/2022.
//

#include "base/film.h"
#include "core/render_pipeline.h"

namespace vision {
using namespace ocarina;

class RGBFilm : public Film {
private:
    Image _radiance;

public:
    explicit RGBFilm(const FilmDesc &desc) : Film(desc) {}
    void prepare(RenderPipeline *rp) noexcept override {
        _radiance = rp->device().create_image(resolution(), PixelStorage::FLOAT4);
    }
    void add_sample(Uint2 pixel, Float4 val, Uint frame_index) noexcept override {
        _radiance.write(pixel, val);
    }
    void copy_to(void *host_ptr) const noexcept override {
        _radiance.download_immediately(host_ptr);
    }
};

}// namespace vision

VS_MAKE_CLASS_CREATOR(vision::RGBFilm)