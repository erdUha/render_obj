#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <stdio.h>
#include "cfg.h"

void swap (void *p1, void *p2, size_t size)
{
	void *p3 = malloc(size);
	if (p3 != 0)
	{
		memmove(p3, p1, size);
		memmove(p1, p2, size);
		memmove(p2, p3, size);
		free(p3);
	}
}

short RoundF (float f)
{
	if (f >= 0)
	{
		return (short)(f + 0.5);
	} else
	{
		return (short)(f - 0.5);
	}
}

ushort RoundAbs (float f)
{
	if (f >= 0)
	{
		return (ushort)(f + 0.5);
	} else
	{
		return 0;
	}
}

struct Coord
{
	ushort x, y;
};

struct Point
{
	float x, y, z;
};

struct Normal
{
	float x, y, z;
};

struct Triangle2
{
	short x1, y1, x2, y2, x3, y3;
	float s1, s2, s3;
};

struct Triangle3
{
	float x1, y1, z1, x2, y2, z2, x3, y3, z3, s1, s2, s3;
	struct Normal n1, n2, n3;
};

struct Vec3
{
	float x, y, z, Ox, Oy, Oz;
};

void DrawLine (ushort WIDTH, ushort HEIGHT, u_char** coord, ushort x0, ushort y0, ushort x1, ushort y1) {
	int dx = x1 - x0;
	int dy = y1 - y0;
	if (dx == 0 && dy == 0)
	{
		if (x0 >= 0 && x0 < WIDTH && y0 >= 0 && y0 < HEIGHT)
		{
			coord[y0][x0] = 255;
		}
	} else if (abs(dx) < abs(dy))
	{
		// When line is vertical
		float rdx = (float)dx / (float)dy;
		struct Coord p;
		if (dy < 0)
		{
			y0 = y0 + dy;
			x0 = x0 + RoundF(rdx * (float)dy);
			dy = -dy;
		}
		for (ushort i = y0; i <= y0 + dy; i++)
		{
			p.y = i;
			p.x = RoundF(rdx * ((float)i - y0) + x0);
			if (p.x >= 0 && p.x < WIDTH && p.y >= 0 && p.y < HEIGHT)
			{
				coord[p.y][p.x] = 1;
			}
		}
	} else
	{
		// When line is horizontal
		float rdy = (float)dy / (float)dx;
		struct Coord p;
		if (dx < 0)
		{
			x0 = x0 + dx;
			y0 = y0 + RoundF(rdy * (float)dx);
			dx = -dx;
		}
		for (ushort i = x0; i <= x0 + dx; i++)
		{
			p.x = i;
			p.y = RoundF(rdy * ((float)i - x0) + y0);
			if (p.x >= 0 && p.x < WIDTH && p.y >= 0 && p.y < HEIGHT)
			{
				coord[p.y][p.x] = 1;
			}
		}
	}
}

void DrawHorizontalGradient (ushort WIDTH, ushort HEIGHT, u_char** coord, float** zbuffer, int x0, int x1, int y, float s0, float s1, float z1, float z2) {
	if (x1 - x0 == 0)
	{
		return;
	}
	if (y < 0 || y >= HEIGHT)
	{
		return;
	}
	if (x0 > x1)
	{
		swap(&x0, &x1, sizeof(x0));
	}
	for (int x = x0; x <= x1; x++)
	{
		float xf = (float)x, x0f = (float)x0, x1f = (float)x1;
		float k = (xf - x0f) / (x1f - x0f);
		float tmp_z = z1 * (1.0 - k) + z2 * k;
		if (x >= 0 && x < WIDTH && tmp_z >= 0 && tmp_z < zbuffer[y][x])
		{
			zbuffer[y][x] = tmp_z;
			coord[y][x] = RoundF((float)GRADIENT_SIZE + (255.0 - (float)GRADIENT_SIZE) * (s0 * (1.0 - k) + s1 * k));
		}
	}
}

void SortTriangle2 (struct Triangle2* tr)
{
	if (tr->y1 > tr->y2) 
	{
		swap(&tr->x1, &tr->x2, sizeof(tr->x1));
		swap(&tr->y1, &tr->y2, sizeof(tr->y1));
		swap(&tr->s1, &tr->s2, sizeof(tr->s1));
	}
	if (tr->y2 > tr->y3) 
	{
		swap(&tr->x2, &tr->x3, sizeof(tr->x2));
		swap(&tr->y2, &tr->y3, sizeof(tr->y2));
		swap(&tr->s2, &tr->s3, sizeof(tr->s2));
	}
	if (tr->y1 > tr->y2) 
	{
		swap(&tr->x1, &tr->x2, sizeof(tr->x1));
		swap(&tr->y1, &tr->y2, sizeof(tr->y1));
		swap(&tr->s1, &tr->s2, sizeof(tr->s1));
	}
}

void DrawTriangle2 (ushort WIDTH, ushort HEIGHT, u_char** coord, float** zbuffer, struct Triangle2* tr)
{
	SortTriangle2(tr);
	int dx12 = tr->x2 - tr->x1;
	ushort dy12 = tr->y2 - tr->y1;
	int dx23 = tr->x3 - tr->x2;
	ushort dy23 = tr->y3 - tr->y2;
	int dx13 = tr->x3 - tr->x1;
	ushort dy13 = tr->y3 - tr->y1;
	if (dy12 == 0 && dy23 == 0)
	{
		return;
	}
	//interpolating
	float rdx12 = (float)dx12 / (float)dy12;
	float rdx23 = (float)dx23 / (float)dy23;
	float rdx13 = (float)dx13 / (float)dy13;
	if (!isfinite(rdx13))
		return;
	struct Coord p;
	for (ushort i = (tr->y1 >= 0) ? tr->y1 : 0; i <= tr->y3; i++)
	 {
		float k12, k13, k23;
		k12 = (float)(i - tr->y1) / (float)(tr->y2 - tr->y1);
		k13 = (float)(i - tr->y1) / (float)(tr->y3 - tr->y1);
		k23 = (float)(i - tr->y2) / (float)(tr->y3 - tr->y2);
		short x12, x13;
		float s12, s13;
		if (i < tr->y2)
		{
			if (!isfinite(rdx12))
				continue;
			x12 = RoundF(rdx12 * (float)(i - tr->y1) + tr->x1);
			x13 = RoundF(rdx13 * (float)(i - tr->y1) + tr->x1);
			s12 = tr->s1 * (1.0 - k12) + tr->s2 * k12;
			s13 = tr->s1 * (1.0 - k13) + tr->s3 * k13;
		} else
		{
			if (!isfinite(rdx23))
				continue;
			x12 = RoundF(rdx23 * (float)(i - tr->y2) + tr->x2);
			x13 = RoundF(rdx13 * (float)(i - tr->y1) + tr->x1);
			s12 = tr->s2 * (1.0 - k23) + tr->s3 * k23;
			s13 = tr->s1 * (1.0 - k13) + tr->s3 * k13;
		}
		if (dx12 >= 0 && dx23 <= 0 && dy23 != 0)
		{
			DrawHorizontalGradient(WIDTH, HEIGHT, coord, zbuffer, x12, x13, i, s13, s12, 1.0, 1.0);
		}
		else
		{
			DrawHorizontalGradient(WIDTH, HEIGHT, coord, zbuffer, x12, x13, i, s12, s13, 1.0, 1.0);
		}
	}
}

void Clear (ushort width, ushort height, u_char** coord, float** zbuffer)
{
	for (ushort y = 0; y < height; y++)
	{
		for (ushort x = 0; x < width; x++)
		{
			coord[y][x] = 0;
			zbuffer[y][x] = MAX_DISTANCE;
		}
	}
}

struct Point PointsSum (struct Point point1, struct Point point2)
{
	point1.x += point2.x;
	point1.y += point2.y;
	point1.z += point2.z;
	return point1;
}

struct Point PointsSub (struct Point point1, struct Point point2)
{
	point1.x -= point2.x;
	point1.y -= point2.y;
	point1.z -= point2.z;
	return point1;
}

struct Vec3 VecAdd (struct Vec3 vec1, struct Vec3 vec2)
{
	struct Vec3 res = {vec1.x + vec2.x, vec1.y + vec2.y, vec1.z + vec2.z, vec1.x, vec1.y, vec1.z};
	return res;
}

struct Vec3 PointsToVec (struct Point point1, struct Point point2)
{
	struct Vec3 res = {point2.x, point2.y, point2.z, point1.x, point1.y, point1.z};
	return res;
}

struct Vec3 VecPlusPoint (struct Vec3 vec, struct Point point)
{
	vec.x += point.x;
	vec.y += point.y;
	vec.z += point.z;
	vec.Ox += point.x;
	vec.Oy += point.y;
	vec.Oz += point.z;
	return vec;
}

struct Vec3 VecMinusPoint (struct Vec3 vec, struct Point point)
{
	vec.x -= point.x;
	vec.y -= point.y;
	vec.z -= point.z;
	vec.Ox -= point.x;
	vec.Oy -= point.y;
	vec.Oz -= point.z;
	return vec;
}

struct Triangle2 Rasterize (ushort w, ushort h, ushort s, float ar, ushort fov, struct Triangle3 tr)
{
	struct Triangle2 ans = {
		RoundF((tr.x1 / tr.z1) * (float)s * ASPECT_RATIO / tan((float)fov * M_PI / 360.0) + ((float)w/2.0)), RoundF(-(tr.y1 / tr.z1) * (float)s / tan((float)fov * M_PI / 360.0) + ((float)h/2.0)),
		RoundF((tr.x2 / tr.z2) * (float)s * ASPECT_RATIO / tan((float)fov * M_PI / 360.0) + ((float)w/2.0)), RoundF(-(tr.y2 / tr.z2) * (float)s / tan((float)fov * M_PI / 360.0) + ((float)h/2.0)),
		RoundF((tr.x3 / tr.z3) * (float)s * ASPECT_RATIO / tan((float)fov * M_PI / 360.0) + ((float)w/2.0)), RoundF(-(tr.y3 / tr.z3) * (float)s / tan((float)fov * M_PI / 360.0) + ((float)h/2.0)),
		tr.s1, tr.s2, tr.s3
	};
	return ans;
}

float VecLen (struct Vec3 vec)
{
	return (sqrt(pow(vec.x - vec.Ox, 2.0) + pow(vec.y - vec.Oy, 2.0) + pow(vec.z - vec.Oz, 2.0)));
}

float VecDot (struct Vec3 vec1, struct Vec3 vec2)
{
	return ((vec1.x - vec1.Ox) * (vec2.x - vec2.Ox)) + ((vec1.y - vec1.Oy) * (vec2.y - vec2.Oy)) + ((vec1.z - vec1.Oz) * (vec2.z - vec2.Oz));
}

float AngleBetweenVecs (struct Vec3 vec1, struct Vec3 vec2)
{
	float len1 = VecLen(vec1);
	float len2 = VecLen(vec2);
	float dot = VecDot(vec1, vec2);
	if (len1 == 0 || len2 == 0)
	{
		return 0.0;
	} else
	{
		return (
			acos(
				dot / (len1 * len2)
			)
		);
	}
}

float NormalLen (struct Normal normal)
{
	return sqrt(pow(normal.x, 2.0) + pow(normal.y, 2.0) + pow(normal.z, 2.0));
}

struct Normal VecNormalize (struct Vec3 vec)
{
	struct Normal res = {
		vec.x - vec.Ox,
		vec.y - vec.Oy,
		vec.z - vec.Oz
	};
	float len = NormalLen(res);
	if (len == 0.0)
	{
		res.x = 0.0;
		res.y = 0.0;
		res.z = 0.0;
		return res;
	}
	res.x /= len;
	res.y /= len;
	res.z /= len;
	return res;
}

struct Normal CrossProduct (struct Vec3 vec1, struct Vec3 vec2)
{
	struct Normal ans = {
		(vec1.y * vec2.z) - (vec1.z * vec2.y),
		(vec1.z * vec2.x) - (vec1.x * vec2.z),
		(vec1.x * vec2.y) - (vec1.y * vec2.x)
	};
	float len = NormalLen(ans);
	ans.x /= len;
	ans.y /= len;
	ans.z /= len;
	return ans;
}
