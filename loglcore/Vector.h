#pragma once

#include <xmmintrin.h>
#include <string.h>
#include <cmath>

namespace Game {
    //Don't call this function outside.
    template<typename T>
    inline __m128 _pack_vector_to_m128(T t) {
        return _pack_vector_to_m128<float>(static_cast<float>(t));
    }

    template<>
    inline __m128 _pack_vector_to_m128(float num) {
        __m128 temp = _mm_load_ss(&num);
        return _mm_shuffle_ps(temp, temp, _MM_SHUFFLE(0, 0, 0, 0));
    }

    


    //Don't invoke this function external.Call the dot function 
    //to get two vectors' dot product 
    float _packed_dot(__m128 v1, __m128 v2);

    struct Vector2 {
        union {
            float raw[2];
            struct {
                float x, y;
            };
        };
        Vector2() :x(0), y(0) {}
        Vector2(float x, float y) :x(x), y(y) {}
        Vector2(float* v) :x(v[0]), y(v[1]) { }

        //don't call this function to construct the vector.
        Vector2(__m128 m);

        template<typename T>
        Vector2(T vec) : Vector2(_pack_vector_to_m128(vec)) {}

        inline float& operator[](int index) { return raw[index]; }
        inline const float& operator[](int index) const { return raw[index]; }

        template<typename T>
        inline Vector2 operator+=(const T& other) { *this = *this + other; return *this; }

        template<typename T>
        inline Vector2 operator-=(const T& other) { *this = *this - other; return *this; }


        inline bool operator==(const Vector2& other) { return x == other.x && y == other.y; }
    };

    template<>
    inline __m128 _pack_vector_to_m128(Vector2 v) {
        float buffer[4] = { 0,0,0,0 };
        memcpy(buffer, v.raw, sizeof(float) * 2);
        return _mm_load_ps(buffer);
    }

    struct Vector3 {
        union {
            float raw[3];
            struct {
                float x, y, z;
            };
        };

        Vector3() :x(0), y(0), z(0) {}
        Vector3(float x, float y, float z) :x(x), y(y), z(z) {}
        Vector3(float* f) :x(f[0]), y(f[1]), z(f[2]) {}

        //don't call this function to construct the vector.
        Vector3(__m128 m);

        Vector3(Vector2 vec, float z) :x(vec.x), y(vec.y), z(z) {}

        template<typename T>
        Vector3(T vec) : Vector3(_pack_vector_to_m128(vec)) {}

        inline float& operator[](int index) { return raw[index]; }
        inline const float& operator[](int index) const { return raw[index]; }

        template<typename T>
        inline Vector3 operator+=(const T& other) { *this = *this + other; return *this; }
        
        template<typename T>
        inline Vector3 operator-=(const T& other) { *this = *this - other; return *this; }


        inline bool operator==(const Vector3& other) { return x == other.x && y == other.y && z == other.z; }
    };

    template<>
    inline __m128 _pack_vector_to_m128(Vector3 v) {
        float buffer[4] = { 0,0,0,0 };
        memcpy(buffer, v.raw, sizeof(float) * 3);
        return _mm_load_ps(buffer);
    }

    struct Vector4 {
        union {
            float raw[4];
            struct {
                float x, y, z, w;
            };
        };
        Vector4() :x(0), y(0), z(0), w(0) {}
        Vector4(float x, float y, float z, float w) :x(x), y(y), z(z), w(w) {}
        Vector4(float* f) :x(f[0]), y(f[1]), z(f[2]), w(f[3]) {}

        //don't call this function to construct the vector.
        Vector4(__m128 m);

        Vector4(Vector3 vec,float w):x(vec.x),y(vec.y),z(vec.z),w(w){}
        Vector4(Vector2 vec, float z, float w) :x(vec.x), y(vec.y),z(z),w(w) {}


        inline float& operator[](int index) { return raw[index]; }
        inline const float& operator[](int index) const { return raw[index]; }

        template<typename T>
        inline Vector4 operator+=(const T& other) { *this = *this + other; return *this; }

        template<typename T>
        inline Vector4 operator-=(const T& other) { *this = *this - other; return *this; }

        inline bool operator==(const Vector4& other) { return x == other.x && y == other.y && z == other.z && w == other.w; }
    };

    template<>
    inline __m128 _pack_vector_to_m128(Vector4 v) {
        return _mm_load_ps(v.raw);
    }

    template<typename Vec>
    inline float dot(const Vec& lhs, const Vec& rhs) {
        return _packed_dot(
            _pack_vector_to_m128(lhs),
            _pack_vector_to_m128(rhs)
        );
    }

    template<typename T>
    inline Vector2 operator+ (const Vector2& vec, const T& t) {
        return
            Vector2(_mm_add_ps(
                _pack_vector_to_m128(vec),
                _pack_vector_to_m128(t)
            )
            );
    }

    template<typename T>
    inline Vector2 operator- (const Vector2& vec, const T& t) {
        return
            Vector2(_mm_sub_ps(
                _pack_vector_to_m128(vec),
                _pack_vector_to_m128(t)
            )
            );
    }

    template<typename T>
    inline Vector2 operator* (const Vector2& vec, const T& t) {
        return
            Vector2(_mm_mul_ps(
                _pack_vector_to_m128(vec),
                _pack_vector_to_m128(t)
            )
            );
    }

    inline Vector2 operator/ (const Vector2& vec, float t) {
        return
            Vector2(_mm_div_ps(
                _pack_vector_to_m128(vec),
                _pack_vector_to_m128(t)
            )
            );
    }


    template<typename T>
    inline Vector3 operator+ (const Vector3& vec, const T& t) {
        return
            Vector3(_mm_add_ps(
                _pack_vector_to_m128(vec),
                _pack_vector_to_m128(t)
            )
            );
    }

    template<typename T>
    inline Vector3 operator- (const Vector3& vec, const T& t) {
        return
            Vector3(_mm_sub_ps(
                _pack_vector_to_m128(vec),
                _pack_vector_to_m128(t)
            )
            );
    }

    template<typename T>
    inline Vector3 operator* (const Vector3& vec, const T& t) {
        return
            Vector3(_mm_mul_ps(
                _pack_vector_to_m128(vec),
                _pack_vector_to_m128(t)
            )
            );
    }

    inline Vector3 operator/ (const Vector3& vec, float t) {
        return
            Vector3(_mm_div_ps(
                _pack_vector_to_m128(vec),
                _pack_vector_to_m128(t)
            )
            );
    }

    template<typename T>
    inline Vector4 operator+ (const Vector4& vec, const T& t) {
        return
            Vector4(_mm_add_ps(
                _pack_vector_to_m128(vec),
                _pack_vector_to_m128(t)
            )
            );
    }

    template<typename T>
    inline Vector4 operator- (const Vector4& vec, const T& t) {
        return
            Vector4(_mm_sub_ps(
                _pack_vector_to_m128(vec),
                _pack_vector_to_m128(t)
            )
            );
    }

    template<typename T>
    inline Vector4 operator* (const Vector4& vec, const T& t) {
        return
            Vector4(_mm_mul_ps(
                _pack_vector_to_m128(vec),
                _pack_vector_to_m128(t)
            )
            );
    }

    inline Vector4 operator/ (const Vector4& vec, float t) {
        return
            Vector4(_mm_div_ps(
                _pack_vector_to_m128(vec),
                _pack_vector_to_m128(t)
            )
            );
    }

    Vector3 cross(const Vector3& lhs, const Vector3& rhs);

    template<typename Vec>
    inline float length(const Vec& v) {
        return sqrt(dot(v, v));
    }

    template<typename Vec>
    inline Vec normalize(const Vec& v) {
        return v / length(v);
    }

    //a * (1 - f) + b * f
    template<typename Vec>
    inline Vec lerp(Vec a, Vec b, float f) {
        return a * (1 - f) + b * f;
    }

    template<typename Vec,typename Vec2>
    inline Vec castVec(Vec2 source) {
        return Vec(_pack_vector_to_m128(source));
    }
}