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
	struct Normal n1, n2, n3;
	float c1, c2, c3;
	float z1, z2, z3; 
};

struct Triangle3
{
	float x1, y1, z1, x2, y2, z2, x3, y3, z3;
	float c1, c2, c3;
	struct Normal n1, n2, n3;
};

struct Vec3
{
	float x, y, z, Ox, Oy, Oz;
};

struct Sun
{
	float x, y, z, i;
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

void DrawHorizontalGradient (ushort WIDTH, ushort HEIGHT, u_char** coord, float** zbuffer, struct Normal** normalbuffer, int x0, int x1, int y, struct Normal n0, struct Normal n1, float z0, float z1, float c0, float c1) {
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
		float tmp_z = z0 * (1.0 - k) + z1 * k;
		struct Normal tmp_n = {
			n0.x * (1.0 - k) + n1.x * k,
			n0.y * (1.0 - k) + n1.y * k,
			n0.z * (1.0 - k) + n1.z * k,
		};

		if (x >= 0 && x < WIDTH && tmp_z >= 0 && tmp_z < zbuffer[y][x])
		{
			normalbuffer[y][x] = tmp_n;
			zbuffer[y][x] = tmp_z;
			coord[y][x] = RoundF((float)GRADIENT_SIZE + (255.0 - (float)GRADIENT_SIZE) * (c0 * (1.0 - k) + c1 * k));
		}
	}
}

void SortTriangle2 (struct Triangle2* tr)
{
	if (tr->y1 > tr->y2) 
	{
		swap(&tr->x1, &tr->x2, sizeof(tr->x1));
		swap(&tr->y1, &tr->y2, sizeof(tr->y1));
		swap(&tr->n1, &tr->n2, sizeof(tr->n1));
		swap(&tr->z1, &tr->z2, sizeof(tr->z1));
	}
	if (tr->y2 > tr->y3) 
	{
		swap(&tr->x2, &tr->x3, sizeof(tr->x2));
		swap(&tr->y2, &tr->y3, sizeof(tr->y2));
		swap(&tr->n2, &tr->n3, sizeof(tr->n2));
		swap(&tr->z2, &tr->z3, sizeof(tr->z2));
	}
	if (tr->y1 > tr->y2) 
	{
		swap(&tr->x1, &tr->x2, sizeof(tr->x1));
		swap(&tr->y1, &tr->y2, sizeof(tr->y1));
		swap(&tr->n1, &tr->n2, sizeof(tr->n1));
		swap(&tr->z1, &tr->z2, sizeof(tr->z1));
	}
}

void DrawTriangle2 (ushort WIDTH, ushort HEIGHT, u_char** coord, float** zbuffer, struct Normal** normalbuffer, struct Triangle2* tr)
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
		struct Normal n12, n13;
		float z12, z13;
		float c12, c13;
		if (i < tr->y2)
		{
			if (!isfinite(rdx12))
				continue;
			x12 = RoundF(rdx12 * (float)(i - tr->y1) + tr->x1);
			n12.x = tr->n1.x * (1.0 - k12) + tr->n2.x * k12;
			n12.y = tr->n1.y * (1.0 - k12) + tr->n2.y * k12;
			n12.z = tr->n1.z * (1.0 - k12) + tr->n2.z * k12;
			z12 = tr->z1 * (1.0 - k12) + tr->z2 * k12;
			c12 = tr->c1 * (1.0 - k12) + tr->c2 * k12;
		} else
		{
			if (!isfinite(rdx23))
				continue;
			x12 = RoundF(rdx23 * (float)(i - tr->y2) + tr->x2);
			n12.x = tr->n2.x * (1.0 - k23) + tr->n3.x * k23;
			n12.y = tr->n2.y * (1.0 - k23) + tr->n3.y * k23;
			n12.z = tr->n2.z * (1.0 - k23) + tr->n3.z * k23;
			z12 = tr->z2 * (1.0 - k23) + tr->z3 * k23;
			c12 = tr->c2 * (1.0 - k23) + tr->c3 * k23;
		}
		x13 = RoundF(rdx13 * (float)(i - tr->y1) + tr->x1);
		n13.x = tr->n1.x * (1.0 - k13) + tr->n3.x * k13;
		n13.y = tr->n1.y * (1.0 - k13) + tr->n3.y * k13;
		n13.z = tr->n1.z * (1.0 - k13) + tr->n3.z * k13;
		z13 = tr->z1 * (1.0 - k13) + tr->z3 * k13;
		c13 = tr->c1 * (1.0 - k13) + tr->c3 * k13;
		if ((dx12 >= 0 && rdx12 > rdx13) || (dx12 < 0 && rdx12 > rdx13))
		{
			DrawHorizontalGradient(WIDTH, HEIGHT, coord, zbuffer, normalbuffer, x12, x13, i, n13, n12, z13, z12, c13, c12);
		}
		else
		{
			DrawHorizontalGradient(WIDTH, HEIGHT, coord, zbuffer, normalbuffer, x12, x13, i, n12, n13, z12, z13, c12, c13);
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

struct Triangle2 Rasterize (ushort w, ushort h, ushort s, float ar, struct Triangle3 tr, struct Point offset)
{
	float kx = (float)s * ASPECT_RATIO / tan((float)FOV * M_PI / 360.0);
	float ky = (float)s / tan((float)FOV * M_PI / 360.0);
	float hw = (float)w / 2.0;
	float hh = (float)h / 2.0;
	struct Triangle2 ans = {
		RoundF(((tr.x1 + offset.x) / (tr.z1 + offset.z)) * kx + hw),
		RoundF(-((tr.y1 + offset.y) / (tr.z1 + offset.z)) * ky + hh),

		RoundF(((tr.x2 + offset.x) / (tr.z2 + offset.z)) * kx + hw),
		RoundF(-((tr.y2 + offset.y) / (tr.z2 + offset.z)) * ky + hh),
		
		RoundF(((tr.x3 + offset.x) / (tr.z3 + offset.z)) * kx + hw),
		RoundF(-((tr.y3 + offset.y) / (tr.z3 + offset.z)) * ky + hh),

		tr.n1, tr.n2, tr.n3,
		tr.c1, tr.c2, tr.c3,
		tr.z1, tr.z2, tr.z3
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

float NormalDot (struct Normal norm1, struct Normal norm2)
{
	return (norm1.x * norm2.x) + (norm1.y * norm2.y) + (norm1.z * norm2.z);
}

float AngleBetweenVecs (struct Vec3 vec1, struct Vec3 vec2) {
	float len1 = VecLen(vec1);
	float len2 = VecLen(vec2);
	float dot = VecDot(vec1, vec2);

	if (len1 == 0 || len2 == 0)
		return 0.0;

	return (
		acos(
			dot / (len1 * len2)
		)
	);
}

float AngleBetweenNormals (struct Normal norm1, struct Normal norm2) {
	float dot = NormalDot(norm1, norm2);
	return (acos(dot));
}

float NormalLen (struct Normal normal)
{
	return sqrt(pow(normal.x, 2.0) + pow(normal.y, 2.0) + pow(normal.z, 2.0));
}

void Normalize (struct Normal *normal)
{
	float len = NormalLen(*normal);
	if (len == 0.0)
	{
		(*normal).x = 0.0;
		(*normal).y = 0.0;
		(*normal).z = 0.0;
		return;
	}
	(*normal).x /= len;
	(*normal).y /= len;
	(*normal).z /= len;
}

// Maybe i'll need it. Eventually
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

void ComputeLight(ushort w, ushort h, ushort s, u_char** coord, float** zbuffer, struct Normal** normalbuffer, struct Sun sun)
{
	float kx = (float)s * ASPECT_RATIO / tan((float)FOV * M_PI / 360.0);
	float ky = (float)s / tan((float)FOV * M_PI / 360.0);
	float hw = (float)w / 2.0;
	float hh = (float)h / 2.0;
	for (int y = 0; y < h; y++)
	{
		for (int x = 0; x < w; x++)
		{
			if (zbuffer[y][x] != MAX_DISTANCE)
			{
				// Basically reverse of Rasterize(). 
				// It gets x and y values of actual location of the intersection between 
			 	// object triangle and camera ray.
			 	struct Point Intersection = { 						
					((float)(x) - hw) * zbuffer[y][x] / kx, 
					((float)(y) - hh) * zbuffer[y][x] / ky, 
				};
				struct Normal objNormal = {
					normalbuffer[y][x].x, 
					normalbuffer[y][x].y, 
					normalbuffer[y][x].z
				};
				struct Normal objToSun = {
					-sun.x, -sun.y, -sun.z
				};
				Normalize(&objToSun);

				// Getting luminance value per pixel by getting cosine of angle
				// between object normal and sun direction normal.
				coord[y][x] *= fmax(0.0, cos(AngleBetweenNormals(objNormal, objToSun))); 
			}
		}
	}
}
 
void Fetch (char filename[], struct Triangle3 **Triangles, ulong *trianglesCount, float color, struct Point offset)
{
	FILE *objFile = fopen(filename, "rt");
	if (objFile == NULL)
	{
		printf("\033[31mERR:\033[0m Failed to open file \"%s\"\n", filename);
		exit(-1);
	}
	char line[64];
	ulong verticiesCount = 0;
	ulong normalsCount = 0;
	*trianglesCount = 0;
	while (fgets(line, 64, objFile) != NULL)
	{
		char code[3] = {'\0'};
		for (int i = 0; line[i] != ' ' && i < 2; i++)
			code[i] = line[i];
		if (strcmp(code, "v") == 0)
			verticiesCount++;
		if (strcmp(code, "vn") == 0)
			normalsCount++;
		if (strcmp(code, "f") == 0)
		{
			u_char verticiesPerFace = 1;
			for (u_char i = 2; line[i] != '\n'; i++)
			{
				if (line[i] == ' ')
					verticiesPerFace++;
			}
			if (verticiesPerFace == 3)
			{
				*trianglesCount = *trianglesCount + 1;
			}
		}
	}
	fseek(objFile, 0, 0);
	struct Point *Vertices = (struct Point*)calloc(verticiesCount, sizeof(struct Point));
	struct Normal *Normals = (struct Normal*)calloc(normalsCount, sizeof(struct Normal));
	struct Normal *VertexNormals = (struct Normal*)calloc(verticiesCount, sizeof(struct Normal));
	*Triangles = (struct Triangle3*)calloc(*trianglesCount, sizeof(struct Triangle3));
	ulong v = 0;
	ulong n = 0;
	ulong f = 0;
	while (fgets(line, 64, objFile) != NULL)
	{
		char code[3] = {'\0'};
		for (int i = 0; line[i] != ' ' && i < 2; i++)
			code[i] = line[i];
		if (strcmp(code, "v") == 0)
		{
			char c[10] = {'\0'};
			u_char i = 2, i2 = 0;
			while (line[i] != ' ') { c[i2] = line[i]; i++; i2++; }
			Vertices[v].x = -atof(c) + offset.x;
			i++; i2 = 0; memset(&c, 0, 10);
			while (line[i] != ' ') { c[i2] = line[i]; i++; i2++; }
			Vertices[v].y = atof(c) + offset.y;
			i++; i2 = 0;  memset(&c, 0, 10);
			while (line[i] != '\n') { c[i2] = line[i]; i++; i2++; }
			Vertices[v].z = atof(c) + offset.z;
			memset(&c, 0, 10);

			v++;
		}
		if (strcmp(code, "vn") == 0)
		{
			u_char i = 3, i2 = 0;
			char c[8] = {'\0'};
			while (line[i] != ' ') { c[i2] = line[i]; i++; i2++; }
			Normals[n].x = -atof(c);
			i++; i2 = 0; memset(&c, 0, 8);
			while (line[i] != ' ') { c[i2] = line[i]; i++; i2++; }
			Normals[n].y = atof(c);
			i++; i2 = 0; memset(&c, 0, 8);
			while (line[i] != '\n') { c[i2] = line[i]; i++; i2++; }
			Normals[n].z = atof(c);
			memset(&c, 0, 8);

			n++;
		}
		if (strcmp(code, "f") == 0)
		{
			u_char verticiesPerFace = 1;
			for (u_char i = 2; line[i] != '\n'; i++)
			{
				if (line[i] == ' ')
					verticiesPerFace++;
			}
			u_char i = 2;
			for (u_char g = 0; g < verticiesPerFace; g++)
			{
				u_char i2 = 0;
				if (verticiesPerFace == 3)
				{
					char cv[32] = {'\0'};
					while (line[i] != '/') { cv[i2] = line[i]; i++; i2++; }
					// For Vertices
					switch (g)
					{
						case 0:
							(*Triangles)[f].x1 = Vertices[atoi(cv)-1].x;
							(*Triangles)[f].y1 = Vertices[atoi(cv)-1].y;
							(*Triangles)[f].z1 = Vertices[atoi(cv)-1].z;
							break;
						case 1:
							(*Triangles)[f].x2 = Vertices[atoi(cv)-1].x;
							(*Triangles)[f].y2 = Vertices[atoi(cv)-1].y;
							(*Triangles)[f].z2 = Vertices[atoi(cv)-1].z;
							break;
						case 2:
							(*Triangles)[f].x3 = Vertices[atoi(cv)-1].x;
							(*Triangles)[f].y3 = Vertices[atoi(cv)-1].y;
							(*Triangles)[f].z3 = Vertices[atoi(cv)-1].z;
							break;
					}
					(*Triangles)[f].c1 = color;	(*Triangles)[f].c2 = color;	(*Triangles)[f].c3 = color;
					i++; i2 = 0;

					char cu[32] = {'\0'};
					while (line[i] != '/') { cu[i2] = line[i]; i++; i2++; }
					// For UV
					i++; i2 = 0;

					char cn[32] = {'\0'};
					while (line[i] != ' ' && line[i] != '\n') { cn[i2] = line[i]; i++; i2++; }
					if (SMOOTH_SHADING)
					{
						// For Vertex Normals
						VertexNormals[atoi(cv)-1].x += Normals[atoi(cn)-1].x;
						VertexNormals[atoi(cv)-1].y += Normals[atoi(cn)-1].y;
						VertexNormals[atoi(cv)-1].z += Normals[atoi(cn)-1].z;
					}
					else
					{
						// For Normals
						switch (g)
						{
							case 0:
								(*Triangles)[f].n1.x = Normals[atoi(cn)-1].x;
								(*Triangles)[f].n1.y = Normals[atoi(cn)-1].y;
								(*Triangles)[f].n1.z = Normals[atoi(cn)-1].z;
								break;
							case 1:
								(*Triangles)[f].n2.x = Normals[atoi(cn)-1].x;
								(*Triangles)[f].n2.y = Normals[atoi(cn)-1].y;
								(*Triangles)[f].n2.z = Normals[atoi(cn)-1].z;
								break;
							case 2:
								(*Triangles)[f].n3.x = Normals[atoi(cn)-1].x;
								(*Triangles)[f].n3.y = Normals[atoi(cn)-1].y;
								(*Triangles)[f].n3.z = Normals[atoi(cn)-1].z;
								break;
						}
					}
				}
			}
			f++;
		}
	}

	if (SMOOTH_SHADING)
	{
		for (int i = 0; i < verticiesCount; i++)
		{
			Normalize(&VertexNormals[i]);
		}
		fseek(objFile, 0, 0);
		f = 0;
		while (fgets(line, 64, objFile) != NULL)
		{
			char code[3] = {'\0'};
			for (int i = 0; line[i] != ' ' && i < 2; i++)
				code[i] = line[i];
			if (strcmp(code, "f") == 0)
			{
				u_char verticiesPerFace = 1;
				for (u_char i = 2; line[i] != '\n'; i++)
				{
					if (line[i] == ' ')
						verticiesPerFace++;
				}
				u_char i = 2;
				for (u_char g = 0; g < verticiesPerFace; g++)
				{
					u_char i2 = 0;
					if (verticiesPerFace == 3)
					{
						char cv[32] = {'\0'};
						while (line[i] != '/') { cv[i2] = line[i]; i++; i2++; }
						switch (g)
						{
							case 0:
								(*Triangles)[f].n1.x = VertexNormals[atoi(cv)-1].x;
								(*Triangles)[f].n1.y = VertexNormals[atoi(cv)-1].y;
								(*Triangles)[f].n1.z = VertexNormals[atoi(cv)-1].z;
								break;
							case 1:
								(*Triangles)[f].n2.x = VertexNormals[atoi(cv)-1].x;
								(*Triangles)[f].n2.y = VertexNormals[atoi(cv)-1].y;
								(*Triangles)[f].n2.z = VertexNormals[atoi(cv)-1].z;
								break;
							case 2:
								(*Triangles)[f].n3.x = VertexNormals[atoi(cv)-1].x;
								(*Triangles)[f].n3.y = VertexNormals[atoi(cv)-1].y;
								(*Triangles)[f].n3.z = VertexNormals[atoi(cv)-1].z;
								break;
						}
						i++; i2 = 0;

						char cu[32] = {'\0'};
						while (line[i] != '/') { cu[i2] = line[i]; i++; i2++; }
						// For UV
						i++; i2 = 0;

						char cn[32] = {'\0'};
						while (line[i] != ' ' && line[i] != '\n') { cn[i2] = line[i]; i++; i2++; }
					}
				}
				f++;
			}
		}
		fclose(objFile);
		free(Vertices);
		free(Normals);
	}
}
