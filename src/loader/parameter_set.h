//
// Created by Zero on 09/09/2022.
//

#pragma once

#include "ocarina/src/core/basic_types.h"
#include "core/logging.h"

using namespace ocarina;

namespace vision {

class ParameterSet {
private:
    std::string _key;
    mutable DataWrap _data;

private:
#define VISION_MAKE_AS_TYPE_FUNC(type)     \
    OC_NODISCARD type _as_##type() const { \
        return static_cast<type>(_data);   \
    }

#define VISION_MAKE_AS_TYPE_VEC2(type)                  \
    OC_NODISCARD type##2 _as_##type##2() const {        \
        return make_##type##2(this->at(0).as_##type(),  \
                              this->at(1).as_##type()); \
    }

#define VISION_MAKE_AS_TYPE_VEC3(type)                  \
    OC_NODISCARD type##3 _as_##type##3() const {        \
        return make_##type##3(this->at(0).as_##type(),  \
                              this->at(1).as_##type(),  \
                              this->at(2).as_##type()); \
    }
#define VISION_MAKE_AS_TYPE_VEC4(type)                                          \
    OC_NODISCARD type##4 _as_##type##4() const {                                \
        return make_##type##4(this->at(0).as_##type(),                          \
                              this->at(1).as_##type(),                          \
                              this->at(2).as_##type(),                          \
                              this->at(3).as_##type());                         \
    }                                                                           \
    template<typename T, std::enable_if_t<std::is_same_v<T, type##4>, int> = 0> \
    T _as() const {                                                             \
        return _as_##type##4();                                                 \
    }
#define VISION_MAKE_AS_TYPE_VEC(type) \
    VISION_MAKE_AS_TYPE_VEC2(type)    \
    VISION_MAKE_AS_TYPE_VEC3(type)    \
    VISION_MAKE_AS_TYPE_VEC4(type)

#define VISION_MAKE_AS_TYPE_MAT(type) \
    VISION_MAKE_AS_TYPE_MAT2X2(type)  \
    VISION_MAKE_AS_TYPE_MAT3X3(type)  \
    VISION_MAKE_AS_TYPE_MAT4X4(type)

#define VISION_MAKE_AS_TYPE_MAT2X2(type)                       \
    OC_NODISCARD float2x2 _as_##type##2x2() const {            \
        if (_data.size() == 2) {                               \
            return make_##type##2x2(                           \
                this->at(0)._as_##type##2(),                   \
                this->at(1)._as_##type##2());                  \
        } else {                                               \
            return make_##type##2x2(this->at(0)._as_##type(),  \
                                    this->at(1)._as_##type(),  \
                                    this->at(2)._as_##type(),  \
                                    this->at(3)._as_##type()); \
        }                                                      \
    }

#define VISION_MAKE_AS_TYPE_MAT3X3(type)                       \
    OC_NODISCARD float3x3 _as_##type##3x3() const {            \
        if (_data.size() == 3) {                               \
            return make_##type##3x3(                           \
                this->at(0)._as_##type##3(),                   \
                this->at(1)._as_##type##3(),                   \
                this->at(2)._as_##type##3());                  \
        } else {                                               \
            return make_##type##3x3(this->at(0)._as_##type(),  \
                                    this->at(1)._as_##type(),  \
                                    this->at(2)._as_##type(),  \
                                    this->at(3)._as_##type(),  \
                                    this->at(4)._as_##type(),  \
                                    this->at(5)._as_##type(),  \
                                    this->at(6)._as_##type(),  \
                                    this->at(7)._as_##type(),  \
                                    this->at(8)._as_##type()); \
        }                                                      \
    }
#define VISION_MAKE_AS_TYPE_MAT4X4(type)                        \
    OC_NODISCARD float4x4 _as_##type##4x4() const {             \
        if (_data.size() == 4) {                                \
            return make_##type##4x4(                            \
                this->at(0)._as_##type##4(),                    \
                this->at(1)._as_##type##4(),                    \
                this->at(2)._as_##type##4(),                    \
                this->at(3)._as_##type##4());                   \
        } else {                                                \
            return make_##type##4x4(this->at(0)._as_##type(),   \
                                    this->at(1)._as_##type(),   \
                                    this->at(2)._as_##type(),   \
                                    this->at(3)._as_##type(),   \
                                    this->at(4)._as_##type(),   \
                                    this->at(5)._as_##type(),   \
                                    this->at(6)._as_##type(),   \
                                    this->at(7)._as_##type(),   \
                                    this->at(8)._as_##type(),   \
                                    this->at(9)._as_##type(),   \
                                    this->at(10)._as_##type(),  \
                                    this->at(11)._as_##type(),  \
                                    this->at(12)._as_##type(),  \
                                    this->at(13)._as_##type(),  \
                                    this->at(14)._as_##type(),  \
                                    this->at(15)._as_##type()); \
        }                                                       \
    }

    VISION_MAKE_AS_TYPE_FUNC(int)
    VISION_MAKE_AS_TYPE_FUNC(uint)
    VISION_MAKE_AS_TYPE_FUNC(bool)
    VISION_MAKE_AS_TYPE_FUNC(float)
    VISION_MAKE_AS_TYPE_FUNC(string)
    VISION_MAKE_AS_TYPE_VEC(uint)
    VISION_MAKE_AS_TYPE_VEC(int)
    VISION_MAKE_AS_TYPE_VEC(float)
    VISION_MAKE_AS_TYPE_MAT(float)

#undef VISION_MAKE_AS_TYPE_FUNC

#undef VISION_MAKE_AS_TYPE_VEC
#undef VISION_MAKE_AS_TYPE_VEC2
#undef VISION_MAKE_AS_TYPE_VEC3
#undef VISION_MAKE_AS_TYPE_VEC4

#undef VISION_MAKE_AS_TYPE_MAT
#undef VISION_MAKE_AS_TYPE_MAT3X3
#undef VISION_MAKE_AS_TYPE_MAT4X4

public:
    ParameterSet() = default;

    ParameterSet(const DataWrap &json, const string &key = "")
        : _data(json), _key(key) {}

    void setJson(const DataWrap &json) { _data = json; }

    OC_NODISCARD DataWrap data() const { return _data; }

    OC_NODISCARD ParameterSet get(const std::string &key) const {
        return ParameterSet(_data[key], key);
    }

    OC_NODISCARD ParameterSet at(uint idx) const {
        return ParameterSet(_data.at(idx));
    }

    OC_NODISCARD ParameterSet operator[](const std::string &key) const {
        return ParameterSet(_data.value(key, DataWrap()), key);
    }

    OC_NODISCARD bool contains(const std::string &key) const {
        return _data.contains(key);
    }

    OC_NODISCARD ParameterSet operator[](uint i) const {
        return ParameterSet(_data[i]);
    }

    template<typename T>
    OC_NODISCARD std::vector<T> as_vector() const {
        OC_ERROR_IF(!_data.is_array(), "data is not array!");
        std::vector<T> ret;
        for (const auto &elm : _data) {
            ParameterSet ps{elm};
            ret.push_back(ps.template as<T>());
        }
        return ret;
    }

#define VISION_MAKE_AS_TYPE_SCALAR(type)                                     \
    OC_NODISCARD type as_##type(type val = type()) const {                   \
        try {                                                                \
            if (_data.is_null()) _data = val;                                \
            return _as_##type();                                             \
        } catch (const std::exception &e) {                                  \
            return val;                                                      \
        }                                                                    \
    }                                                                        \
    template<typename T, std::enable_if_t<std::is_same_v<T, type>, int> = 0> \
    [[nodiscard]] T as(T t = T{}) const {                                    \
        return as_##type(t);                                                 \
    }

#define VISION_MAKE_AS_TYPE_VEC_DIM(type, dim)                                               \
    OC_NODISCARD type##dim as_##type##dim(type##dim val = make_##type##dim()) const noexcept { \
        try {                                                                                \
            return _as_##type##dim();                                                        \
        } catch (const std::exception &e) {                                                  \
            return val;                                                                      \
        }                                                                                    \
    }                                                                                        \
    template<typename T, std::enable_if_t<std::is_same_v<T, type##dim>, int> = 0>            \
    [[nodiscard]] T as(T t = T{}) const {                                                    \
        return as_##type##dim(t);                                                            \
    }

#define VISION_MAKE_AS_TYPE_MAT_DIM(type, dim)                                                                                 \
    OC_NODISCARD type##dim##x##dim as_##type##dim##x##dim(type##dim##x##dim val = make_##type##dim##x##dim()) const noexcept { \
        try {                                                                                                                  \
            return _as_##type##dim##x##dim();                                                                                  \
        } catch (const std::exception &e) {                                                                                    \
            return val;                                                                                                        \
        }                                                                                                                      \
    }                                                                                                                          \
    template<typename T, std::enable_if_t<std::is_same_v<T, type##dim##x##dim>, int> = 0>                                      \
    [[nodiscard]] T as(T t = T{}) const {                                                                                      \
        return as_##type##dim##x##dim(t);                                                                                      \
    }

#define VISION_MAKE_AS_TYPE_MAT(type)    \
    VISION_MAKE_AS_TYPE_MAT_DIM(type, 2) \
    VISION_MAKE_AS_TYPE_MAT_DIM(type, 3) \
    VISION_MAKE_AS_TYPE_MAT_DIM(type, 4)

    VISION_MAKE_AS_TYPE_MAT(float)
    VISION_MAKE_AS_TYPE_SCALAR(float)
    VISION_MAKE_AS_TYPE_SCALAR(uint)
    VISION_MAKE_AS_TYPE_SCALAR(bool)
    VISION_MAKE_AS_TYPE_SCALAR(int)
    VISION_MAKE_AS_TYPE_SCALAR(string)

#define VISION_MAKE_AS_TYPE_VEC(type)    \
    VISION_MAKE_AS_TYPE_VEC_DIM(type, 2) \
    VISION_MAKE_AS_TYPE_VEC_DIM(type, 3) \
    VISION_MAKE_AS_TYPE_VEC_DIM(type, 4)
    VISION_MAKE_AS_TYPE_VEC(int)
    VISION_MAKE_AS_TYPE_VEC(uint)
    VISION_MAKE_AS_TYPE_VEC(float)

#undef VISION_MAKE_AS_TYPE_SCALAR
#undef VISION_MAKE_AS_TYPE_VEC
#undef VISION_MAKE_AS_TYPE_VEC_DIM
#undef VISION_MAKE_AS_TYPE_MAT
#undef VISION_MAKE_AS_TYPE_MAT_DIM
};

}// namespace vision