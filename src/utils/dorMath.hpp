#pragma once
#include "raylib.h"
#include "raymath.h"
#include <cmath>        // std::sqrt, std::cos, std::sin
#include <algorithm>    

// helpers
static inline float random_ab(float a, float b)
{
    return float(static_cast<float>(rand()) / (RAND_MAX + 1.0f) * (b - a) + a);
}

// Small vector helpers ---------------------------------------------------------
static inline Vector2 vlimit(Vector2 v, float maxLen)
{
    const float len = Vector2Length(v);
    if (len > maxLen && len > 0.0f)
        return Vector2Scale(v, maxLen / len);
    return v;
}
static inline Vector2 vsafe_normalize(Vector2 v)
{
    const float len = Vector2Length(v);
    if (len > 0.00001f)
        return Vector2Scale(v, 1.0f / len);
    return Vector2{0, 0};
}

struct Vec2 {
    float x{0}, y{0};

    // --- ctors ---
    constexpr Vec2() = default;
    constexpr Vec2(float x_, float y_) : x{x_}, y{y_} {}
    // from raylib
    static constexpr Vec2 from(Vector2 v) { return Vec2{v.x, v.y}; }
    // to raylib
    constexpr operator Vector2() const { return Vector2{ x, y }; }

    // --- unary ---
    constexpr Vec2 operator+() const { return *this; }
    constexpr Vec2 operator-() const { return Vec2{-x, -y}; }
    constexpr Vec2 operator++() const {return Vec2{x + 1.0f, y + 1.0f}; }
    constexpr Vec2 operator--() const {return Vec2{x - 1.0f, y - 1.0f}; }

    // --- component-wise vector ops ---
    constexpr Vec2 operator+(const Vec2& v) const { return {x + v.x, y + v.y}; }
    constexpr Vec2 operator-(const Vec2& v) const { return {x - v.x, y - v.y}; }
    constexpr Vec2 operator*(const Vec2& v) const { return {x * v.x, y * v.y}; } // Hadamard
    constexpr Vec2 operator/(const Vec2& v) const { return {x / v.x, y / v.y}; }

    // --- scalar ops ---
    constexpr Vec2 operator*(float s) const { return {x * s, y * s}; }
    constexpr Vec2 operator/(float s) const { return {x / s, y / s}; }

    // --- compound assignments ---
    constexpr Vec2& operator+=(const Vec2& v) { x += v.x; y += v.y; return *this; }
    constexpr Vec2& operator-=(const Vec2& v) { x -= v.x; y -= v.y; return *this; }
    constexpr Vec2& operator*=(const Vec2& v) { x *= v.x; y *= v.y; return *this; }
    constexpr Vec2& operator/=(const Vec2& v) { x /= v.x; y /= v.y; return *this; }

    constexpr Vec2& operator*=(float s) { x *= s; y *= s; return *this; }
    constexpr Vec2& operator/=(float s) { x /= s; y /= s; return *this; }

    // --- comparisons (exact) ---
    constexpr bool operator==(const Vec2& v) const { return x == v.x && y == v.y; }
    constexpr bool operator!=(const Vec2& v) const { return !(*this == v); }

    // --- indexing (0->x, 1->y) ---
    float& operator[](int i)       { return (i == 0) ? x : y; }
    float  operator[](int i) const { return (i == 0) ? x : y; }

    // --- metrics ---
    constexpr float lengthSquared() const { return x*x + y*y; }
    float length() const { return std::sqrt(lengthSquared()); }

    // normalized (safe: returns {0,0} for zero-length)
    Vec2 normalized() const {
        float len = length();
        return (len > 0.0f) ? (*this / len) : Vec2{0,0};
    }

    // dot/cross (2D cross as scalar z-component)
    static constexpr float dot(const Vec2& a, const Vec2& b) { return a.x*b.x + a.y*b.y; }
    static constexpr float crossZ(const Vec2& a, const Vec2& b) { return a.x*b.y - a.y*b.x; }

    // angles & rotation (radians)
    static float angleBetween(const Vec2& a, const Vec2& b) {
        float d = dot(a, b);
        float l = std::sqrt(a.lengthSquared() * b.lengthSquared());
        if (l <= 0.0f) return 0.0f;
        float c = Clamp(d / l, -1.0f, 1.0f);
        return std::acos(c);
    }
    Vec2 rotated(float radians) const {
        float c = std::cos(radians), s = std::sin(radians);
        return { x*c - y*s, x*s + y*c };
    }

    // projection & reflection
    static Vec2 project(const Vec2& a, const Vec2& onto) { // proj of a onto onto
        float d2 = onto.lengthSquared();
        if (d2 <= 0.0f) return {0,0};
        return onto * (dot(a, onto) / d2);
    }
    static Vec2 reflect(const Vec2& v, const Vec2& normalUnit) {
        // assumes normalUnit is normalized
        return v - normalUnit * (2.0f * dot(v, normalUnit));
    }

    // lerp & clamp
    static constexpr Vec2 lerp(const Vec2& a, const Vec2& b, float t) {
        return a + (b - a) * t;
    }
    static Vec2 clamp(const Vec2& v, const Vec2& minV, const Vec2& maxV) {
        return { Clamp(v.x, minV.x, maxV.x), Clamp(v.y, minV.y, maxV.y) };
    }

    // approx equality (tolerance)
    static constexpr bool nearEqual(const Vec2& a, const Vec2& b, float eps = 1e-5f) {
        return (std::abs(a.x - b.x) <= eps) && (std::abs(a.y - b.y) <= eps);
    }

    // handy constants
    static constexpr Vec2 zero() { return {0,0}; }
    static constexpr Vec2 one()  { return {1,1}; }
    static constexpr Vec2 unitX(){ return {1,0}; }
    static constexpr Vec2 unitY(){ return {0,1}; }
    
    static constexpr Vec2 fromAngle(float angle){ return {cos(angle), sin(angle)}; }
};

// scalar * Vec2
constexpr Vec2 operator*(float s, const Vec2& v) { return v * s; }

// Small vector helpers ---------------------------------------------------------
struct Vec3 {
    float x{0}, y{0}, z{0};

    // --- ctors ---
    constexpr Vec3() = default;
    constexpr Vec3(float x_, float y_, float _z) : x{x_}, y{y_} ,z{_z} {}
    // from raylib
    static constexpr Vec3 from(Vector3 v) { return Vec3{v.x, v.y, v.z}; }
    // to raylib
    constexpr operator Vector3() const { return Vector3{ x, y , z}; }

    // --- unary ---
    constexpr Vec3 operator+() const { return *this; }
    constexpr Vec3 operator-() const { return Vec3{-x, -y, -z}; }
    constexpr Vec3 operator++() const {return Vec3{x + 1.0f, y + 1.0f, z + 1.0f}; }
    constexpr Vec3 operator--() const {return Vec3{x - 1.0f, y - 1.0f, z - 1.0f}; }

    // --- component-wise vector ops ---
    constexpr Vec3 operator+(const Vec3& v) const { return {x + v.x, y + v.y, z + v.z}; }
    constexpr Vec3 operator-(const Vec3& v) const { return {x - v.x, y - v.y, z - v.z}; }
    constexpr Vec3 operator*(const Vec3& v) const { return {x * v.x, y * v.y, z * v.z}; } // Hadamard
    constexpr Vec3 operator/(const Vec3& v) const { return {x / v.x, y / v.y, z / v.z}; }

    // --- scalar ops ---
    constexpr Vec3 operator*(float s) const { return {x * s, y * s, z * s}; }
    constexpr Vec3 operator/(float s) const { return {x / s, y / s, z / s}; }

    // --- compound assignments ---
    constexpr Vec3& operator+=(const Vec3& v) { x += v.x; y += v.y; z += v.z; return *this; }
    constexpr Vec3& operator-=(const Vec3& v) { x -= v.x; y -= v.y; z -= v.z; return *this; }
    constexpr Vec3& operator*=(const Vec3& v) { x *= v.x; y *= v.y; z *= v.z; return *this; }
    constexpr Vec3& operator/=(const Vec3& v) { x /= v.x; y /= v.y; z /= v.z; return *this; }

    constexpr Vec3& operator*=(float s) { x *= s; y *= s; z *= s; return *this; }
    constexpr Vec3& operator/=(float s) { x /= s; y /= s; z /= s; return *this; }

    // --- comparisons (exact) ---
    constexpr bool operator==(const Vec3& v) const { return x == v.x && y == v.y && z == v.z; }
    constexpr bool operator!=(const Vec3& v) const { return !(*this == v); }

    // --- indexing (0->x, 1->y).. ---
    float& operator[](int i)       { return (i == 0) ? x : (i == 1) ? y : z; }
    float  operator[](int i) const { return (i == 0) ? x : (i == 1) ? y : z; }

    // --- metrics ---
    constexpr float lengthSquared() const { return x*x + y*y + z*z; }
    float length() const { return std::sqrt(lengthSquared()); }
    // normalized (safe: returns {0,0} for zero-length)
    Vec3 normalized() const {
        float len = length();
        return (len > 0.0f) ? (*this / len) : Vec3{0,0,0};
    }

    // dot/cross (2D cross as scalar z-component)
    static constexpr float dot(const Vec3& a, const Vec3& b) { return a.x * b.x + a.y * b.y + a.z * b.z; }

    // lerp & clamp
    static constexpr Vec3 lerp(const Vec3& a, const Vec3& b, float t) {
        return a + (b - a) * t;
    }
    static Vec3 clamp(const Vec3& v, const Vec3& minV, const Vec3& maxV) {
        return { Clamp(v.x, minV.x, maxV.x), Clamp(v.y, minV.y, maxV.y) , Clamp(v.z, minV.z, maxV.z)};
    }

    // approx equality (tolerance)
    static constexpr bool nearEqual(const Vec3& a, const Vec3& b, float eps = 1e-5f) {
        return (std::abs(a.x - b.x) <= eps) && (std::abs(a.y - b.y) <= eps) && (std::abs(a.z - b.z));
    }

    static Vec3 absV(const Vec3& v) { return {abs(v.x), abs(v.y), abs(v.z)}; }
    static float firstNonZero(const Vec3& v){ return v.x != 0 ? v.x : v.y != 0 ? v.y : v.z; }
    // handy constants
    static constexpr Vec3 zero() { return {0,0,0}; }
    static constexpr Vec3 one()  { return {1,1,1}; }
    static constexpr Vec3 unitX(){ return {1,0,0}; }
    static constexpr Vec3 unitY(){ return {0,1,0}; }
    static constexpr Vec3 unitZ(){ return {0,0,1}; }

};

// scalar * Vec2
constexpr Vec3 operator*(float s, const Vec3& v) { return v * s; }
