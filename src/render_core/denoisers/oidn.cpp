//
// Created by Zero on 2023/5/30.
//

#include "base/denoiser.h"
#include "OpenImageDenoise/oidn.hpp"

namespace vision {

using namespace ocarina;

class OidnDenoiser : public Denoiser {
private:
    oidn::DeviceRef _device{};

private:
    [[nodiscard]] oidn::DeviceRef create_device() const noexcept {
        oidn::DeviceType device_type{};
        switch (_backend) {
            case CPU:
                device_type = oidn::DeviceType::CPU;
                break;
            case GPU:
                device_type = oidn::DeviceType::CUDA;
                break;
            default:
                break;
        }
        return oidn::DeviceRef(oidn::newDevice(device_type));
    }
    [[nodiscard]] oidn::FilterRef create_filter() const noexcept {
        switch (_mode) {
            case RT:
                return _device.newFilter("RT");
            case RTLightmap:
                return _device.newFilter("RTLightmap");
            default:
                break;
        }
        return nullptr;
    }

public:
    explicit OidnDenoiser(const DenoiserDesc &desc)
        : Denoiser(desc),
          _device{create_device()} {
        _device.commit();
    }

    void apply(ocarina::uint2 res, RegistrableManaged<ocarina::float4> *output,
               RegistrableManaged<ocarina::float4> *color,
               RegistrableManaged<ocarina::float4> *normal,
               RegistrableManaged<ocarina::float4> *albedo) noexcept override {
        float4 *output_ptr = nullptr;
        float4 *color_ptr = nullptr;
        float4 *normal_ptr = nullptr;
        float4 *albedo_ptr = nullptr;

        auto before_denoise = [&]() {
#define VS_DENOISE_ATTR_GPU(attr_name) attr_name##_ptr = attr_name->ptr<float4 *>();
#define VS_DENOISE_ATTR_CPU(attr_name) \
    attr_name->download_immediately(); \
    attr_name##_ptr = attr_name->data();
            switch (_backend) {
                case CPU: {
                    output_ptr = output->data();
                    VS_DENOISE_ATTR_CPU(color)
                    if (normal && albedo) {
                        VS_DENOISE_ATTR_CPU(normal)
                        VS_DENOISE_ATTR_CPU(albedo)
                    }
                    break;
                }
                case GPU: {
                    VS_DENOISE_ATTR_GPU(output)
                    VS_DENOISE_ATTR_GPU(color)
                    if (normal && albedo) {
                        VS_DENOISE_ATTR_GPU(normal)
                        VS_DENOISE_ATTR_GPU(albedo)
                    }
                    break;
                }
            }
#undef VS_DENOISE_ATTR_GPU
#undef VS_DENOISE_ATTR_CPU
        };

        auto after_denoise = [&]() {
            switch (_backend) {
                case CPU: break;
                case GPU: {
                    output->download_immediately();
                    break;
                }
                default: break;
            }
        };

        before_denoise();
        apply(res, output_ptr, color_ptr, normal_ptr, albedo_ptr);
        after_denoise();
    }

    void apply(uint2 res, float4 *output, float4 *color,
               float4 *normal, float4 *albedo) noexcept override {
        TIMER(oidn_denoise)
        oidn::FilterRef filter = create_filter();
        filter.setImage("output", output, oidn::Format::Float3, res.x, res.y, 0, sizeof(float4));
        filter.setImage("color", color, oidn::Format::Float3, res.x, res.y, 0, sizeof(float4));
        if (normal && albedo) {
            filter.setImage("normal", normal, oidn::Format::Float3, res.x, res.y, 0, sizeof(float4));
            filter.setImage("albedo", albedo, oidn::Format::Float3, res.x, res.y, 0, sizeof(float4));
        }
        // color image is HDR
        filter.set("hdr", true);
        filter.commit();
        filter.execute();
        _device.sync();

        const char *errorMessage;
        if (_device.getError(errorMessage) != oidn::Error::None) {
            OC_ERROR_FORMAT("oidn error: {}", errorMessage)
        }
    }
};

}// namespace vision

VS_MAKE_CLASS_CREATOR(vision::OidnDenoiser)