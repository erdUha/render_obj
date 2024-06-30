#include <sys/ioctl.h>
#include <unistd.h>
#include "render_obj.h"

int main ()
{
	struct winsize w;
	ioctl(STDOUT_FILENO, TIOCGWINSZ, &w);
	int WIDTH = w.ws_col;
	int HEIGHT = w.ws_row;
	int SMALLEST = ((float)WIDTH / ASPECT_RATIO < HEIGHT) ? RoundF((float)WIDTH / ASPECT_RATIO) : HEIGHT;

	struct Triangle3 *Triangles;

	ulong trianglesCount;

	Fetch(OBJ_FILE, &Triangles, &trianglesCount);

	struct Sun sun = {-0.5, -1.0, -0.3, 1.0};

	ComputeLight(Triangles, trianglesCount, sun);

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

	for (uint i = 0; i < 1; i++)
	{
		if (!DEBUG)
		{
			for (ulong g = 0; g < trianglesCount; g++)
			{
				struct Triangle2 tr = Rasterize(WIDTH, HEIGHT, SMALLEST, ASPECT_RATIO, FOV, Triangles[g]);
				DrawTriangle2(WIDTH, HEIGHT, coord, zbuffer, &tr);
			}
			for (int y = 0; y < HEIGHT; y++)
			{
				for (int x = 0; x < WIDTH; x++)
				{
					putchar(ASCII_GRADIENT[(int)((float)coord[y][x] * ((float)GRADIENT_SIZE / 256.0))]);
					//printf("%f\n", zbuffer[y][x]);
				}
				putchar('\n');
			}
			//usleep(16667);
			Clear(WIDTH, HEIGHT, coord, zbuffer);
		}
	}
	return 0;
}
