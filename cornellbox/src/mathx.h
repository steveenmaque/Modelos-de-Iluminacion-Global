// mathx.h - mini libreria vectorial (evita depender de GLM)
#pragma once
#include <cmath>

struct Vec3 {
    float x = 0, y = 0, z = 0;
    Vec3() {}
    Vec3(float a) : x(a), y(a), z(a) {}
    Vec3(float a, float b, float c) : x(a), y(b), z(c) {}
};

inline Vec3 operator+(Vec3 a, Vec3 b) { return {a.x + b.x, a.y + b.y, a.z + b.z}; }
inline Vec3 operator-(Vec3 a, Vec3 b) { return {a.x - b.x, a.y - b.y, a.z - b.z}; }
inline Vec3 operator*(Vec3 a, float s) { return {a.x * s, a.y * s, a.z * s}; }
inline Vec3 operator*(float s, Vec3 a) { return {a.x * s, a.y * s, a.z * s}; }
inline float dot(Vec3 a, Vec3 b) { return a.x * b.x + a.y * b.y + a.z * b.z; }
inline Vec3 cross(Vec3 a, Vec3 b) {
    return {a.y * b.z - a.z * b.y,
            a.z * b.x - a.x * b.z,
            a.x * b.y - a.y * b.x};
}
inline float length(Vec3 a) { return std::sqrt(dot(a, a)); }
inline Vec3 normalize(Vec3 a) {
    float l = length(a);
    return l > 0 ? a * (1.0f / l) : a;
}

inline float radians(float deg) { return deg * 3.14159265358979323846f / 180.0f; }
inline float clampf(float v, float lo, float hi) { return v < lo ? lo : (v > hi ? hi : v); }

// Camara orbital: gira alrededor de 'target' con yaw/pitch/distancia.
struct OrbitCamera {
    Vec3 target{0.0f, 0.0f, 0.0f};
    float yaw = 0.0f;       // grados, alrededor de Y
    float pitch = 0.0f;     // grados, elevacion
    float distance = 3.2f;  // distancia al objetivo
    float fovY = 40.0f;     // campo de vision vertical en grados

    Vec3 position() const {
        float cy = std::cos(radians(yaw)),  sy = std::sin(radians(yaw));
        float cp = std::cos(radians(pitch)), sp = std::sin(radians(pitch));
        Vec3 dir{cp * sy, sp, cp * cy};   // direccion desde target hacia la camara
        return target + dir * distance;
    }
    // Construye la base de la camara (forward, right, up) mirando hacia target.
    void basis(Vec3& fwd, Vec3& right, Vec3& up) const {
        Vec3 pos = position();
        fwd = normalize(target - pos);
        Vec3 worldUp{0.0f, 1.0f, 0.0f};
        right = normalize(cross(fwd, worldUp));
        up = cross(right, fwd);
    }
};
