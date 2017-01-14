
#pragma once

#define Min(a, b) (((a) < (b)) ? (a) : (b))
#define Max(a, b) (((a) > (b)) ? (a) : (b))
#define Clamp(x, a, b) Max((a), Min((x), (b)))

struct Vec2 {
	Vec2(float _x, float _y) : x(_x), y(_y) {}
	Vec2() : x(0), y(0) {}
	float x, y;
};

static inline Vec2 ClampVec2(const Vec2 &f, const Vec2 &min, Vec2 max) {
	return Vec2(Clamp(f.x, min.x, max.x), Clamp(f.y, min.y, max.y));
}
static inline Vec2 MinVec2(const Vec2 &lhs, const Vec2 &rhs) {
	return Vec2(Min(lhs.x, rhs.x), Min(lhs.y, rhs.y));
}
static inline Vec2 MaxVec2(const Vec2 &lhs, const Vec2 &rhs) {
	return Vec2(Max(lhs.x, rhs.x), Max(lhs.y, rhs.y));
}

static inline float DotProduct(const Vec2 &v1, const Vec2 &v2)     { return v1.x * v2.x + v1.y * v2.y; }

static inline Vec2 operator+(const Vec2& lhs, const float rhs)     { return Vec2(lhs.x+rhs, lhs.y+rhs); }
static inline Vec2 operator-(const Vec2& lhs, const float rhs)     { return Vec2(lhs.x-rhs, lhs.y-rhs); }
static inline Vec2 operator*(const Vec2& lhs, const float rhs)     { return Vec2(lhs.x*rhs, lhs.y*rhs); }
static inline Vec2 operator/(const Vec2& lhs, const float rhs)     { return Vec2(lhs.x/rhs, lhs.y/rhs); }

static inline Vec2 operator+(const Vec2& lhs, const Vec2& rhs)     { return Vec2(lhs.x+rhs.x, lhs.y+rhs.y); }
static inline Vec2 operator-(const Vec2& lhs, const Vec2& rhs)     { return Vec2(lhs.x-rhs.x, lhs.y-rhs.y); }
static inline Vec2 operator*(const Vec2& lhs, const Vec2& rhs)     { return Vec2(lhs.x*rhs.x, lhs.y*rhs.y); }
static inline Vec2 operator/(const Vec2& lhs, const Vec2& rhs)     { return Vec2(lhs.x/rhs.x, lhs.y/rhs.y); }

static inline Vec2 operator*(float s, Vec2 a) { return Vec2(s * a.x, s * a.y); }

static inline Vec2& operator+=(Vec2& lhs, const Vec2& rhs)         { lhs.x += rhs.x; lhs.y += rhs.y; return lhs; }
static inline Vec2& operator-=(Vec2& lhs, const Vec2& rhs)         { lhs.x -= rhs.x; lhs.y -= rhs.y; return lhs; }
static inline Vec2& operator*=(Vec2& lhs, const float rhs)         { lhs.x *= rhs; lhs.y *= rhs; return lhs; }
static inline Vec2& operator/=(Vec2& lhs, const float rhs)         { lhs.x /= rhs; lhs.y /= rhs; return lhs; }

static inline bool operator==(const Vec2& l, const Vec2& r)        { return l.x == r.x && l.y == r.y; }
static inline bool operator!=(const Vec2& l, const Vec2& r)        { return !(l.x == r.x && l.y == r.y); }

