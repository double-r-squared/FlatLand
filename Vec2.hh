#ifndef VEC2_HH
#define VEC2_HH

#include <cmath>

struct Vec2 {
    float x, y;
    Vec2(float x = 0, float y = 0) : x(x), y(y) {}
    Vec2 operator+(const Vec2& other) const { return Vec2(x + other.x, y + other.y); }
    Vec2 operator-(const Vec2& other) const { return Vec2(x - other.x, y - other.y); }
    Vec2 operator*(float s) const { return Vec2(x * s, y * s); }
    float length() const { return std::sqrt(x * x + y * y); }
    Vec2 normalized() const { float len = length(); return len > 0 ? Vec2(x/len, y/len) : Vec2(); }
};

#endif // VEC2_HH