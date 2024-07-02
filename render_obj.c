#include <sys/ioctl.h>
#include <unistd.h>
#include "render_obj.h"

int main ()
{
	struct winsize w;
	ioctl(STDOUT_FILENO, TIOCGWINSZ, &w);
	ushort WIDTH = w.ws_col;
	ushort HEIGHT = w.ws_row;
	ushort SHORTEST = ((float)WIDTH / ASPECT_RATIO < HEIGHT) ? RoundF((float)WIDTH / ASPECT_RATIO) : HEIGHT;

	struct Triangle3 *Triangles;

	ulong trianglesCount;

	struct Point offset = { OBJ_X, OBJ_Y, OBJ_Z };

	Fetch(OBJ_FILE, &Triangles, &trianglesCount, 1.0, offset);

	struct Sun sun = {SUN_X, SUN_Y, SUN_Z, 1.0};

	u_char *coord[HEIGHT];
	for (int i = 0; i < HEIGHT; i++)
	{
		coord[i] = (u_char*)calloc(WIDTH, sizeof(u_char));
	}

	float *zbuffer[HEIGHT];
	for (int i = 0; i < HEIGHT; i++)
	{
		zbuffer[i] = (float*)calloc(WIDTH, sizeof(float));
		for (int i2 = 0; i2 < WIDTH; i2++)
		{
			zbuffer[i][i2] = MAX_DISTANCE;
		}
	}

	struct Normal *normalbuffer[HEIGHT];
	for (int i = 0; i < HEIGHT; i++)
	{
		normalbuffer[i] = (struct Normal*)calloc(WIDTH, sizeof(struct Normal));
	}

	struct Point movement_offset = {0.0, 0.0, 0.0};

	for (uint i = 0; i < 1; i++)
	{
		if (!DEBUG)
		{
			for (ulong g = 0; g < trianglesCount; g++)
			{
				struct Triangle2 tr = Rasterize(WIDTH, HEIGHT, SHORTEST, ASPECT_RATIO, Triangles[g], movement_offset);

				DrawTriangle2(WIDTH, HEIGHT, coord, zbuffer, normalbuffer, &tr);
			}
			ComputeLight(WIDTH, HEIGHT, SHORTEST, coord, zbuffer, normalbuffer, sun);
			for (int y = 0; y < HEIGHT; y++)
			{
				for (int x = 0; x < WIDTH; x++)
				{
					putchar(ASCII_GRADIENT[(int)((float)coord[y][x] * ((float)GRADIENT_SIZE / 256.0))]);
				}
				putchar('\n');
			}
			usleep(16667);
			Clear(WIDTH, HEIGHT, coord, zbuffer);
		}
	}
	return 0;
}
