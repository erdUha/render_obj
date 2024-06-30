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

	char filename[] = OBJ_FILE;
	FILE *objFile = fopen(filename, "rt");
	if (objFile == NULL)
	{
		printf("\033[31mERR:\033[0m Failed to open file \"%s\"\n", filename);
		return -1;
	}
	char line[64];
	unsigned long verticiesCount = 0;
	unsigned long normalsCount = 0;
	unsigned long trianglesCount = 0;
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
				trianglesCount++;
			}
		}
	}
	fseek(objFile, 0, 0);
	struct Point *Verticies = (struct Point*)calloc(verticiesCount, sizeof(struct Point));
	struct Normal *Normals = (struct Normal*)calloc(normalsCount, sizeof(struct Normal));
	struct Triangle3 *Triangles = (struct Triangle3*)calloc(trianglesCount, sizeof(struct Triangle3));
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
			Verticies[v].x = -atof(c);
			i++; i2 = 0; memset(&c, 0, 10);
			while (line[i] != ' ') { c[i2] = line[i]; i++; i2++; }
			Verticies[v].y = atof(c);
			i++; i2 = 0;  memset(&c, 0, 10);
			while (line[i] != '\n') { c[i2] = line[i]; i++; i2++; }
			Verticies[v].z = atof(c);
			memset(&c, 0, 10);

			v++;
		}
		if (strcmp(code, "vn") == 0)
		{
			u_char i = 3, i2 = 0;
			char c[8] = {'\0'};
			while (line[i] != ' ') { c[i2] = line[i]; i++; i2++; }
			Normals[n].x = atof(c);
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
			char c[32] = {'\0'};
			u_char i = 2;
			for (u_char g = 0; g < verticiesPerFace; g++)
			{
				u_char i2 = 0;
				if (verticiesPerFace == 3)
				{
					while (line[i] != '/') { c[i2] = line[i]; i++; i2++; }
					// For Verticies
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
					i++; i2 = 0;  memset(&c, 0, 32);
					while (line[i] != '/') { c[i2] = line[i]; i++; i2++; }
					// For UV
					i++; i2 = 0; memset(&c, 0, 32);
					while (line[i] != ' ' && line[i] != '\n') { c[i2] = line[i]; i++; i2++; }
					// For Normals
					switch (g)
					{
						case 0:
							Triangles[f].n1.x = Normals[atoi(c)-1].x;
							Triangles[f].n1.y = Normals[atoi(c)-1].y;
							Triangles[f].n1.z = Normals[atoi(c)-1].z;
							break;
						case 1:
							Triangles[f].n2.x = Normals[atoi(c)-1].x;
							Triangles[f].n2.y = Normals[atoi(c)-1].y;
							Triangles[f].n2.z = Normals[atoi(c)-1].z;
							break;
						case 2:
							Triangles[f].n3.x = Normals[atoi(c)-1].x;
							Triangles[f].n3.y = Normals[atoi(c)-1].y;
							Triangles[f].n3.z = Normals[atoi(c)-1].z;
							break;
					}
					memset(&c, 0, 32);
				}
			}
			f++;
		}
	}
	fclose(objFile);
	free(Verticies);
	free(Normals);

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
				}
				putchar('\n');
			}
			//usleep(16667);
			Clear(WIDTH, HEIGHT, coord, zbuffer);
		}
	}
	return 0;
}
