#pragma once
typedef unsigned char u8;
typedef signed char i8;
typedef unsigned short u16;
typedef signed short i16;
typedef unsigned int u32;
typedef signed int i32;
typedef unsigned long long u64;
typedef long long i64;
typedef float f32;
typedef double f64;

typedef unsigned char bool;
enum { false, true };

#include <string.h>
#define memzero(ptr, size) memset(ptr, 0, size)

#include <math.h>
typedef struct
{
	i32 x;
	i32 y;
} Point;
inline Point point(i32 x, i32 y)
{
	Point point;
	point.x = x;
	point.y = y;
	return point;
}
extern const Point POINT_ZERO;
extern const Point POINT_X;
extern const Point POINT_Y;
extern const Point POINT_ONE;

inline bool point_eq(Point a, Point b) { return a.x == b.x && a.y == b.y; }
inline Point point_add(const Point a, const Point b) { return point(a.x + b.x, a.y + b.y); }
inline Point point_sub(const Point a, const Point b) { return point(a.x - b.x, a.y - b.y); }
inline Point point_inv(const Point pt) { return point(-pt.x, -pt.y); }
f32 point_len(const Point a);
f32 point_dist(const Point from, const Point to);
i32 point_dist_manhattan(const Point from, const Point to);
Point point_lerp(const Point a, const Point b, f32 alpha);

typedef struct
{
	Point min;
	Point max;
} Rect;
inline Rect rect(Point a, Point b)
{
	Rect rect;
	rect.min.x = min(a.x, b.x);
	rect.max.x = max(a.x, b.x);
	rect.min.y = min(a.y, b.y);
	rect.max.y = max(a.y, b.y);
	return rect;
}

bool point_in_rect(Point point, Rect rect);
bool rect_rect_intersect(Rect a, Rect b);

// Randomness
i32 rand_i32();
i32 randrange_i32(i32 from, i32 to);
f32 rand_f32();

// Mathy stuff
inline i32 sign_i32(i32 a) { return a > 0 ? 1 : (a < 0 ? -1 : 0); }