//
// Created by Zero on 2023/7/10.
//

#pragma once

#include "base/bake_utlis.h"

namespace vision {

namespace detail {

template<EPort p = D>
[[nodiscard]] inline oc_uint<p> uint2_to_uint(oc_uint2<p> arg) noexcept {
    oc_uint<p> a = arg.x;
    oc_uint<p> b = arg.y;
    a = a << 16;
    return a | b;
}

template<EPort p = D>
[[nodiscard]] inline oc_uint2<p> uint_to_uint2(oc_uint<p> id) noexcept {
    oc_uint<p> a = (0xffff0000 & id) >> 16;
    oc_uint<p> b = 0x0000ffff & id;
    return make_uint2(a, b);
}

}// namespace detail

#define MAKE_ADD_TIME_FUNC(task) \
    void add_##task##_time(double t) noexcept { _##task##_time += t; }
#define VS_PLUS_TASK_TIME(task) +_##task##_time
#define VS_PLUS_TASK_TIME_STR(task) +get_##task##_stats_str()
#define VS_CLEAR_TIME(task) _##task##_time = 0;
#define VS_MAKE_GET_TIME_STR(task)                                                  \
    [[nodiscard]] string get_##task##_stats_str() const noexcept {                  \
        return ocarina::format(#task " time is {:.4f}s, proportion is {:2.2f}% \n", \
                               _##task##_time, task##_time_percent() * 100);        \
    }
#define VS_MAKE_GET_TIME_TOTAL_STR(...)                                      \
    [[nodiscard]] string get_total_time_stats() const noexcept {             \
        return get_total_time_str() MAP(VS_PLUS_TASK_TIME_STR, __VA_ARGS__); \
    }

#define VS_TIME_PERCENT(task) \
    [[nodiscard]] double task##_time_percent() const noexcept { return _##task##_time / total_time(); }

#define MAKE_TIME_FUNC(...)                            \
    MAP(MAKE_ADD_TIME_FUNC, __VA_ARGS__)               \
    MAP(VS_TIME_PERCENT, __VA_ARGS__)                  \
    MAP(VS_MAKE_GET_TIME_STR, __VA_ARGS__)             \
    VS_MAKE_GET_TIME_TOTAL_STR(__VA_ARGS__)            \
    [[nodiscard]] double total_time() const noexcept { \
        return 0 MAP(VS_PLUS_TASK_TIME, __VA_ARGS__);  \
    }                                                  \
    void clear_time() noexcept {                       \
        MAP(VS_CLEAR_TIME, __VA_ARGS__)                \
    }

struct BakerStats {
private:
    uint _model_num{};
    size_t _pixel_num{};
    uint _batch_index{};
    uint _spp{};
    double _bake_time{};
    double _raster_time{};
    double _filter_time{};
    double _package_time{};
    double _denoise_time{};
    double _uv_unwrap_time{};
    double _save_time{};

    /// batch stats
    Clock clock;
    uint _cur_batch_pixel_num{};
    uint _cur_batch_model_num{};
    uint _sample_index{};
    uint _model_counter{};
    uint _pixel_counter{};

public:
    MAKE_TIME_FUNC(uv_unwrap, raster,
                   bake, filter,
                   package, denoise,
                   save)

    [[nodiscard]] string get_scene_stats() const noexcept {
        return ocarina::format("\nmodel num is {}, \npixel num is {},\nbatch num is {},\n"
                               "{} sample per pixel, \ntotal sample num is {} \n \n",
                               _model_num, _pixel_num, _batch_index, _spp, _spp * _pixel_num);
    }
    void on_batch_start(ocarina::span<BakedShape> lst) noexcept {
        clock.start();
        _batch_index += 1;
        _sample_index = 0;
        _cur_batch_pixel_num = 0;
        _cur_batch_model_num = lst.size();
        std::for_each(lst.begin(), lst.end(), [&](const BakedShape &bs) {
            return _cur_batch_pixel_num += bs.pixel_num();
        });
        printf("batch start\n"
               "total model num is %u,\n"
               "batch model num is %u,\n"
               "cur model index is %u,\n"
               "model progress is %.2f%%,\n"
               "pixel progress is %.2f%%,\n\n",
               _model_num,
               _cur_batch_model_num,
               _model_counter,
               _model_counter * 100.f / _model_num,
               (_pixel_counter * 1.f / _pixel_num) * 100);
        _model_counter += _cur_batch_model_num;
        _pixel_counter += _cur_batch_pixel_num;
    }
    OC_MAKE_MEMBER_SETTER(sample_index)
    OC_MAKE_MEMBER_SETTER(spp)
    OC_MAKE_MEMBER_SETTER(model_num)
    OC_MAKE_MEMBER_SETTER(pixel_num)

    /**
     * report cur batch mesh num
     * spp index
     */
    void report_progress() const noexcept {
        printf("spp is %u,"
               "sample index is %u,"
               "progress is %.2f %% \r",
               _spp, _sample_index, _sample_index * 100.f / _spp);
    }
    void on_batch_end() noexcept {
        double t = clock.elapse_s();
        printf("batch end:\n"
               "elapse time %.3f s,\n"
               "total model num is %u,\n"
               "cur model progress is %u\n\n",
               t,
               _model_num,
               _model_counter);
    }
    [[nodiscard]] string get_total_time_str() const noexcept {
        return ocarina::format("total time is {:.4f}s, \n", total_time());
    }
    [[nodiscard]] string get_all_stats() const noexcept {
        return get_scene_stats() + get_total_time_stats();
    }
    void clear() noexcept { *this = BakerStats(); }
    [[nodiscard]] bool is_valid() const noexcept { return _model_num > 0; }
};

struct BakerGuard {
    Clock clock;
    std::function<void(double)> func;
    explicit BakerGuard(const std::function<void(double)> &f)
        : func(f) {}
    ~BakerGuard() {
        func(clock.elapse_s());
    }
};

#define VS_BAKER_STATS(stats, task) \
    BakerGuard __bg([&](double t) { \
        stats.add_##task##_time(t); \
    });

#undef MAKE_TIME_FUNC
#undef VS_CLEAR_TIME
#undef VS_PLUS_TASK_TIME_STR
#undef VS_TIME_PERCENT
#undef VS_MAKE_GET_TIME_TOTAL_STR
#undef VS_MAKE_GET_TIME_STR
#undef VS_PLUS_TASK_TIME
#undef MAKE_ADD_TIME_FUNC

}// namespace vision