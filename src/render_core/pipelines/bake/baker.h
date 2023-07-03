//
// Created by Zero on 2023/7/3.
//

#pragma once

#include "base/bake_utlis.h"

namespace vision {

class Baker {
private:
    Buffer<float4> _positions;
    Buffer<float4> _normals;
    Buffer<float4> _radiance;
    vector<uint> _pixel_num;
    Rasterizer *_rasterizer{};
    Shader<void(uint, Buffer<float4>, Buffer<float4>, Buffer<float4>)> _bake_shader;
    Shader<void(Buffer<float4>, Buffer<float4>, float4x4, uint, uint2)> _transform_shader;

private:
    void _compile_transform() noexcept;
    void _compile_bake() noexcept;
    void _prepare(ocarina::span<BakedShape> baked_shapes) noexcept;
    void _baking() noexcept;
    void _save_result(ocarina::span<BakedShape> baked_shapes) noexcept;

public:
    explicit Baker(Rasterizer *rasterizer)
        : _rasterizer(rasterizer) {}
    void compile() noexcept;
    void allocate(uint buffer_size, Device &device) noexcept;
    void baking(ocarina::span<BakedShape> baked_shapes) noexcept;
    [[nodiscard]] CommandList append_buffer(const Buffer<float4> &normals,
                                            const Buffer<float4> &positions) noexcept;
    [[nodiscard]] CommandList clear() noexcept;
    [[nodiscard]] static Device &device() noexcept;
    [[nodiscard]] static Stream &stream() noexcept;
    [[nodiscard]] static Pipeline *pipeline() noexcept;
    [[nodiscard]] static Scene &scene() noexcept;
    [[nodiscard]] uint pixel_num() const noexcept;
    [[nodiscard]] BufferDownloadCommand *download_radiance(void *ptr, uint offset) const noexcept;
};

}// namespace vision