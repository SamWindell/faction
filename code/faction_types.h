
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

struct Rect {
	Rect(Vec2 _pos, float _w, float _h) : pos(_pos), w(_w), h(_h) {}
	Rect(float x, float y, float _w, float _h) : pos(x, y), w(_w), h(_h) {}
	Rect() :x(0), y(0), w(0), h(0) {}

	Rect WithY(float _y) {
		pos.y = _y;
		return *this;
	}
	Rect WithLeftPulledIn(float xOffs) {
		auto r = *this;
		r.x += xOffs;
		r.w -= xOffs;
		return r;
	}
	void SetBottom(float b) {
		h = b - pos.y;
	}
	float Bottom() {
		return pos.y + h;
	}
	float Right() {
		return pos.x + w;
	}
	Vec2 Centre() {
		Vec2 p;
		p.x = pos.x + (w * 0.5f);
		p.y = pos.y + (h * 0.5f);
		return p;
	}
	bool Contains(Vec2 p) {
		return p.x >= pos.x && p.x < pos.x + w && p.y >= pos.y && p.y < pos.y + h;
	}
   	Vec2 MinPos() { return pos; }
   	Vec2 MaxPos() { return pos + size; }

	union {
		Vec2 pos;
		struct {
			float x;
			float y;
		};
	};
	union {
		Vec2 size;
		struct {
			float w;
			float h;
		};
	};
};

