/**
 * OMEGA ENGINE - Math Utilities
 * 
 * Common math types and functions used across all Omega games.
 * Includes vectors, colors, rectangles, and math helpers.
 */

#ifndef OMEGA_MATH_H
#define OMEGA_MATH_H

#include <cmath>
#include <algorithm>

namespace Omega {

constexpr double PI = 3.14159265358979323846;
constexpr double DEG_TO_RAD = PI / 180.0;
constexpr double RAD_TO_DEG = 180.0 / PI;

// ============================================================
// UTILITY FUNCTIONS
// ============================================================

inline double clamp(double v, double lo, double hi) {
    return v < lo ? lo : (v > hi ? hi : v);
}

inline float clampf(float v, float lo, float hi) {
    return v < lo ? lo : (v > hi ? hi : v);
}

inline int clampi(int v, int lo, int hi) {
    return v < lo ? lo : (v > hi ? hi : v);
}

inline double lerp(double a, double b, double t) {
    return a + (b - a) * t;
}

inline float lerpf(float a, float b, float t) {
    return a + (b - a) * t;
}

inline double smoothstep(double t) {
    t = clamp(t, 0.0, 1.0);
    return t * t * (3.0 - 2.0 * t);
}

inline double sign(double v) {
    return v < 0 ? -1.0 : (v > 0 ? 1.0 : 0.0);
}

// ============================================================
// 2D VECTOR
// ============================================================

struct Vec2 {
    double x, y;
    
    Vec2() : x(0), y(0) {}
    Vec2(double x, double y) : x(x), y(y) {}
    
    Vec2 operator+(const Vec2& o) const { return Vec2(x + o.x, y + o.y); }
    Vec2 operator-(const Vec2& o) const { return Vec2(x - o.x, y - o.y); }
    Vec2 operator*(double s) const { return Vec2(x * s, y * s); }
    Vec2 operator/(double s) const { return Vec2(x / s, y / s); }
    Vec2 operator-() const { return Vec2(-x, -y); }
    
    Vec2& operator+=(const Vec2& o) { x += o.x; y += o.y; return *this; }
    Vec2& operator-=(const Vec2& o) { x -= o.x; y -= o.y; return *this; }
    Vec2& operator*=(double s) { x *= s; y *= s; return *this; }
    Vec2& operator/=(double s) { x /= s; y /= s; return *this; }
    
    bool operator==(const Vec2& o) const { return x == o.x && y == o.y; }
    bool operator!=(const Vec2& o) const { return !(*this == o); }
    
    double length() const { return std::sqrt(x * x + y * y); }
    double lengthSquared() const { return x * x + y * y; }
    
    Vec2 normalized() const {
        double len = length();
        return len > 0.0001 ? Vec2(x / len, y / len) : Vec2();
    }
    
    double dot(const Vec2& o) const { return x * o.x + y * o.y; }
    double cross(const Vec2& o) const { return x * o.y - y * o.x; }
    
    Vec2 rotate(double angle) const {
        double c = std::cos(angle), s = std::sin(angle);
        return Vec2(x * c - y * s, x * s + y * c);
    }
    
    Vec2 perpendicular() const { return Vec2(-y, x); }
    
    double angle() const { return std::atan2(y, x); }
    double angleTo(const Vec2& o) const { return std::atan2(o.y - y, o.x - x); }
    double distanceTo(const Vec2& o) const { return (*this - o).length(); }
    
    Vec2 lerp(const Vec2& o, double t) const {
        return Vec2(x + (o.x - x) * t, y + (o.y - y) * t);
    }
    
    static Vec2 fromAngle(double angle, double length = 1.0) {
        return Vec2(std::cos(angle) * length, std::sin(angle) * length);
    }
};

// ============================================================
// 2D INTEGER VECTOR (for screen coordinates)
// ============================================================

struct Vec2i {
    int x, y;
    
    Vec2i() : x(0), y(0) {}
    Vec2i(int x, int y) : x(x), y(y) {}
    Vec2i(const Vec2& v) : x((int)v.x), y((int)v.y) {}
    
    Vec2i operator+(const Vec2i& o) const { return Vec2i(x + o.x, y + o.y); }
    Vec2i operator-(const Vec2i& o) const { return Vec2i(x - o.x, y - o.y); }
    Vec2i operator*(int s) const { return Vec2i(x * s, y * s); }
    
    bool operator==(const Vec2i& o) const { return x == o.x && y == o.y; }
    bool operator!=(const Vec2i& o) const { return !(*this == o); }
    
    Vec2 toVec2() const { return Vec2((double)x, (double)y); }
};

// ============================================================
// 3D VECTOR (for future 3D support)
// ============================================================

struct Vec3 {
    double x, y, z;
    
    Vec3() : x(0), y(0), z(0) {}
    Vec3(double x, double y, double z) : x(x), y(y), z(z) {}
    Vec3(const Vec2& v, double z = 0) : x(v.x), y(v.y), z(z) {}
    
    Vec3 operator+(const Vec3& o) const { return Vec3(x + o.x, y + o.y, z + o.z); }
    Vec3 operator-(const Vec3& o) const { return Vec3(x - o.x, y - o.y, z - o.z); }
    Vec3 operator*(double s) const { return Vec3(x * s, y * s, z * s); }
    Vec3 operator/(double s) const { return Vec3(x / s, y / s, z / s); }
    
    Vec3& operator+=(const Vec3& o) { x += o.x; y += o.y; z += o.z; return *this; }
    Vec3& operator-=(const Vec3& o) { x -= o.x; y -= o.y; z -= o.z; return *this; }
    
    double length() const { return std::sqrt(x*x + y*y + z*z); }
    double lengthSquared() const { return x*x + y*y + z*z; }
    
    Vec3 normalized() const {
        double len = length();
        return len > 0.0001 ? Vec3(x/len, y/len, z/len) : Vec3();
    }
    
    double dot(const Vec3& o) const { return x*o.x + y*o.y + z*o.z; }
    
    Vec3 cross(const Vec3& o) const {
        return Vec3(y*o.z - z*o.y, z*o.x - x*o.z, x*o.y - y*o.x);
    }
    
    Vec2 xy() const { return Vec2(x, y); }
};

// ============================================================
// COLOR
// ============================================================

struct Color {
    uint8_t r, g, b, a;
    
    Color() : r(255), g(255), b(255), a(255) {}
    Color(uint8_t r, uint8_t g, uint8_t b, uint8_t a = 255) : r(r), g(g), b(b), a(a) {}
    
    // Construct from 0xRRGGBB or 0xRRGGBBAA
    static Color fromHex(uint32_t hex, bool hasAlpha = false) {
        if (hasAlpha) {
            return Color((hex >> 24) & 0xFF, (hex >> 16) & 0xFF, 
                        (hex >> 8) & 0xFF, hex & 0xFF);
        } else {
            return Color((hex >> 16) & 0xFF, (hex >> 8) & 0xFF, hex & 0xFF);
        }
    }
    
    // Construct from floats (0.0 - 1.0)
    static Color fromFloat(float r, float g, float b, float a = 1.0f) {
        return Color((uint8_t)(clampf(r, 0, 1) * 255),
                    (uint8_t)(clampf(g, 0, 1) * 255),
                    (uint8_t)(clampf(b, 0, 1) * 255),
                    (uint8_t)(clampf(a, 0, 1) * 255));
    }
    
    Color withAlpha(uint8_t newAlpha) const { return Color(r, g, b, newAlpha); }
    
    Color lerp(const Color& o, float t) const {
        return Color((uint8_t)lerpf(r, o.r, t), (uint8_t)lerpf(g, o.g, t),
                    (uint8_t)lerpf(b, o.b, t), (uint8_t)lerpf(a, o.a, t));
    }
    
    // Common colors
    static Color white()   { return Color(255, 255, 255); }
    static Color black()   { return Color(0, 0, 0); }
    static Color red()     { return Color(255, 0, 0); }
    static Color green()   { return Color(0, 255, 0); }
    static Color blue()    { return Color(0, 0, 255); }
    static Color yellow()  { return Color(255, 255, 0); }
    static Color cyan()    { return Color(0, 255, 255); }
    static Color magenta() { return Color(255, 0, 255); }
    static Color gray()    { return Color(128, 128, 128); }
    static Color orange()  { return Color(255, 165, 0); }
    static Color transparent() { return Color(0, 0, 0, 0); }
};

// ============================================================
// RECTANGLE
// ============================================================

struct Rect {
    double x, y, w, h;
    
    Rect() : x(0), y(0), w(0), h(0) {}
    Rect(double x, double y, double w, double h) : x(x), y(y), w(w), h(h) {}
    Rect(const Vec2& pos, const Vec2& size) : x(pos.x), y(pos.y), w(size.x), h(size.y) {}
    
    double left() const { return x; }
    double right() const { return x + w; }
    double top() const { return y; }
    double bottom() const { return y + h; }
    
    Vec2 topLeft() const { return Vec2(x, y); }
    Vec2 topRight() const { return Vec2(x + w, y); }
    Vec2 bottomLeft() const { return Vec2(x, y + h); }
    Vec2 bottomRight() const { return Vec2(x + w, y + h); }
    Vec2 center() const { return Vec2(x + w/2, y + h/2); }
    Vec2 size() const { return Vec2(w, h); }
    
    bool contains(const Vec2& p) const {
        return p.x >= x && p.x < x + w && p.y >= y && p.y < y + h;
    }
    
    bool contains(double px, double py) const {
        return px >= x && px < x + w && py >= y && py < y + h;
    }
    
    bool intersects(const Rect& o) const {
        return x < o.x + o.w && x + w > o.x && y < o.y + o.h && y + h > o.y;
    }
    
    Rect expanded(double amount) const {
        return Rect(x - amount, y - amount, w + amount*2, h + amount*2);
    }
    
    Rect moved(const Vec2& offset) const {
        return Rect(x + offset.x, y + offset.y, w, h);
    }
};

struct Recti {
    int x, y, w, h;
    
    Recti() : x(0), y(0), w(0), h(0) {}
    Recti(int x, int y, int w, int h) : x(x), y(y), w(w), h(h) {}
    Recti(const Rect& r) : x((int)r.x), y((int)r.y), w((int)r.w), h((int)r.h) {}
    
    Rect toRect() const { return Rect((double)x, (double)y, (double)w, (double)h); }
};

} // namespace Omega

#endif // OMEGA_MATH_H
