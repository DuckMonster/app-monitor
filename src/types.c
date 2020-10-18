#include "types.h"
const Point POINT_ZERO = {0, 0};
const Point POINT_X = {1, 0};
const Point POINT_Y = {0, 1};
const Point POINT_ONE = {1, 1};

bool point_in_rect(Point point, Rect rect)
{
	return (point.x >= rect.min.x && point.x <= rect.max.x &&
			point.y >= rect.min.y && point.y <= rect.max.y);
}

bool rect_rect_intersect(Rect a, Rect b)
{
	return (a.max.x >= b.min.x &&
		b.max.x >= a.min.x && 
		a.max.y >= b.min.y &&
		b.max.y >= a.min.y);
}

f32 point_len(const Point a) { return sqrt(a.x * a.x + a.y * a.y); }
f32 point_dist(const Point from, const Point to) { return point_len(point_sub(to, from)); }
i32 point_dist_manhattan(const Point from, const Point to)
{
	Point delta = point_sub(to, from);
	return abs(delta.x) + abs(delta.y);
}
Point point_lerp(const Point a, const Point b, f32 alpha)
{
	return point(
		a.x + round((b.x - a.x) * alpha),
		a.y + round((b.y - a.y) * alpha)
	);
}