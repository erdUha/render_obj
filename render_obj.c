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

	char filename[] = "cube.obj";
	FILE *objFile = fopen(filename, "rt");
	if (objFile == NULL)
	{
		printf("\033[31mERR:\033[0m Failed to open file \"%s\"\n", filename);
		return -1;
	}
	char line[32];
	int verticiesCount = 0;
	int trianglesCount = 0;
	while (fgets(line, 32, objFile) != NULL)
	{
		char code[3] = {'\0'};
		for (int i = 0; line[i] != ' ' && i < 2; i++)
			code[i] = line[i];
		if (strcmp(code, "v") == 0)
			verticiesCount++;
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
				trianglesCount++;
			}
		}
	}
	fseek(objFile, 0, 0);
	struct Point *Verticies = (struct Point*)calloc(verticiesCount, sizeof(struct Point));
	struct Triangle3 *Triangles = (struct Triangle3*)calloc(trianglesCount, sizeof(struct Triangle3));
	int v = 0;
	int f = 0;
	while (fgets(line, 32, objFile) != NULL)
	{
		//char code[3] = "  \0";
		char code[3] = {'\0'};
		for (int i = 0; line[i] != ' ' && i < 2; i++)
			code[i] = line[i];
		if (strcmp(code, "v") == 0)
		{
			char c[10] = {'\0'};
			u_char i = 2, i2 = 0;
			while (line[i] != ' ') { c[i2] = line[i]; i++; i2++; }
			Verticies[v].x = -atof(c);
			i++;
			i2 = 0;
			while (line[i] != ' ') { c[i2] = line[i]; i++; i2++; }
			Verticies[v].y = atof(c);
			i++;
			i2 = 0;
			while (line[i] != '\n') { c[i2] = line[i]; i++; i2++; }
			Verticies[v].z = atof(c);

			v++;
		}
		if (strcmp(code, "f") == 0)
		{
			u_char verticiesPerFace = 1;
			for (u_char i = 2; line[i] != '\n'; i++)
			{
				if (line[i] == ' ')
					verticiesPerFace++;
			}
			char c[10] = {'\0'};
			u_char i = 2, i2 = 0;
			for (u_char g = 0; g < verticiesPerFace; g++)
			{
				while (line[i] != '/') { c[i2] = line[i]; i++; i2++; }
				// For Verticies
				if (verticiesPerFace == 3)
				{
					switch (g)
					{
						case 0:
							Triangles[f].x1 = Verticies[atoi(c)-1].x;
							Triangles[f].y1 = Verticies[atoi(c)-1].y;
							Triangles[f].z1 = Verticies[atoi(c)-1].z;
							break;
						case 1:
							Triangles[f].x2 = Verticies[atoi(c)-1].x;
							Triangles[f].y2 = Verticies[atoi(c)-1].y;
							Triangles[f].z2 = Verticies[atoi(c)-1].z;
							break;
						case 2:
							Triangles[f].x3 = Verticies[atoi(c)-1].x;
							Triangles[f].y3 = Verticies[atoi(c)-1].y;
							Triangles[f].z3 = Verticies[atoi(c)-1].z;
							break;
					}
					Triangles[f].s1 = 1.0; Triangles[f].s2 = 1.0; Triangles[f].s3 = 1.0;
				}
				i++; i2 = 0; 
				while (line[i] != '/') { c[i2] = line[i]; i++; i2++; }
				// For UV
				i++; i2 = 0; memset(&c, 0, 10);
				while (line[i] != ' ' && line[i] != '\n') { c[i2] = line[i]; i++; i2++; }
				// For Normals
				i++; i2 = 0; memset(&c, 0, 10);
			}
			f++;
		}
	}
	fclose(objFile);

	u_char **coord = (u_char**)calloc(HEIGHT, sizeof(u_char*));
	for (int i = 0; i < HEIGHT; i++)
	{
		coord[i] = (u_char*)calloc(WIDTH, sizeof(u_char));
	}

	for (int i = 0; i < 1; i++)
	{
		if (!DEBUG)
		{
			for (int g = 0; g < trianglesCount; g++)
			{
				struct Triangle2 tr = Rasterize(WIDTH, HEIGHT, SMALLEST, ASPECT_RATIO, FOV, Triangles[g]);
				DrawTriangle2(WIDTH, HEIGHT, coord, &tr);
			}
			for (int y = 0; y < HEIGHT; y++)
			{
				for (int x = 0; x < WIDTH; x++)
				{
					putchar(ASCII_GRADIENT[(int)((float)coord[y][x] * ((float)GRADIENT_SIZE / 256.0))]);
				}
				putchar('\n');
			}
			//usleep(16667);
			Clear(WIDTH, HEIGHT, coord);
		}
	}
	/*

	struct Point Light = {2.0, 2.0, 3.0};
	struct Triangle tr1 = {
		0.0, 2.0, 3.0, 
		2.0, -2.0, 3.0,
		-2.0, -2.0, 3.0
	};
	
	for (int i = 0; i < pow(2, (sizeof(int) * 8)); i++)
	{
		for (int y = HEIGHT; y >= 0; y--)
		{
			for(int x = 0; x <= WIDTH; x++)
			{
				struct Vec3 ray = {
					((float)(x) - ((float)(WIDTH) / 2.0)) / (float)(SMALLEST) / ratioX * tan((float)FOV * M_PI / 360.0) * 2.0,
					((float)(y) - ((float)(HEIGHT) / 2.0)) / (float)(SMALLEST) / ratioY * tan((float)FOV * M_PI / 360.0) * 2.0,
					1.0,
					0.0,
					0.0,
					0.0
				};
			}
			putchar('\n');
		}
		usleep(41600);
		Light.x = cos((float)(i) / 10) * 2.0;
		Light.z = 3.0 + sin((float)(i) / 10) * 2.0;
	}
	*/
	return 0;
}
