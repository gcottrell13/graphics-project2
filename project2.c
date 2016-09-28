#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "3dmath.c"
#include "imageread.c"

#define T_CAMERA 1
#define T_SPHERE 2
#define T_PLANE 3

int line = 1;

typedef struct {
	int kind;
	float color[3];
	float a;
	float b;
	float c;
	float d;
} Object;

typedef struct {
	int num_objects;
	Object objects[128];
	float camera_width;
	float camera_height;
	float background_color[3]; // for fun!
} Scene;

int next_c(FILE* file)
{
	int c = fgetc(file);
	if (c == 10) {
		line ++;
	}
	if (c == EOF) {
		fprintf(stderr, "Error: unexpected EOF\n");
		exit(1);
	}
	return c;
}
void expect_c(FILE* file, int d)
{
	int c = next_c(file);
	if (c == d) return;
	fprintf(stderr, "Error: expected %c got %c on line %d\n", d, c, line);
	exit(1);
}
void skip_ws(FILE* file)
{
	int c = next_c(file);
	while(isspace(c))
	{
		c = next_c(file);
	}
	ungetc(c, file);
}

float next_number(FILE* file)
{
	float val;

	int res = fscanf(file, "%f", &val);
	// error
	if(res == 1)
		return val;
	fprintf(stderr, "Error: Could not read number on line %d\n", line);
	exit(1);
}

float* next_vector(FILE* file)
{
	float* v = malloc(3 * sizeof(float));
	expect_c(file, '[');
	skip_ws(file);
	v[0] = next_number(file);
	skip_ws(file);
	expect_c(file, ',');
	skip_ws(file);
	v[1] = next_number(file);
	skip_ws(file);
	expect_c(file, ',');
	skip_ws(file);
	v[2] = next_number(file);
	skip_ws(file);
	expect_c(file, ']');
	return v;
}

char* parse_string(FILE* file)
{
	expect_c(file, '"');
	
	int max_size = 128;
	
	char buffer[max_size];
	int buff_size = 0;
	
	int c = next_c(file);
	while(c != '"' && buff_size < max_size)
	{
		
		if (c < 32 || c > 126)
		{
			fprintf(stderr, "Error: only readable ascii characters are allowed. on line %d\n", line);
			exit(1);
		}
		
		buffer[buff_size] = c;
		buff_size ++;
		c = next_c(file);
	}
	
	buffer[buff_size] = 0;
	
	return strdup(buffer);
}

Scene read_scene(char* json_name)
{
	FILE * json = fopen(json_name, "r");
	Scene scene;

	int c;
	
	skip_ws(json);
	
	// find beginning of file
	expect_c(json, '[');
	
	skip_ws(json);
	
	c = next_c(json);
	if (c == ']')
	{
		fprintf(stderr, "Warning: empty scene file.\n");
		return scene;
	}

	ungetc(c, json);
	skip_ws(json);
	
	int set_camera_width = 0;
	float camera_width = 0;
	int set_camera_height = 0;
	float camera_height = 0;
	
	while(1)
	{
		expect_c(json, '{');
		
		skip_ws(json);

		// parse an object
		char* key = parse_string(json);
		if (strcmp(key, "type") != 0) {
			fprintf(stderr, "Error: expected \"type\" key on line %d\n", line);
			exit(1);
		}
		
		skip_ws(json);
		
		expect_c(json, ':');
		
		skip_ws(json);
		
		char* type_value = parse_string(json);

		int objtype = 0;
		
		int set_radius = 0;
		float radius = 0; // just in case a sphere is read in
		int set_color = 0;
		float color[3];
		int set_normal = 0;
		float normal[3];
		int set_position = 0;
		float position[3];

		Object new_object = scene.objects[scene.num_objects];
		
		if(strcmp(type_value, "camera") == 0) {
			objtype = T_CAMERA;
		} else if(strcmp(type_value, "sphere") == 0) {
			objtype = T_SPHERE;
		} else if(strcmp(type_value, "plane") == 0) {
			objtype = T_PLANE;
		} else {
			
			fprintf(stderr, "Unknown type \"%s\" on line %d\n", type_value, line);
			exit(1);
		}
		
		// copy the information into the new object
		new_object.kind = objtype;

		int finish = 0;

		while(1) 
		{
			skip_ws(json);
			c = next_c(json);

			if (c == '}')
			{
				// stop parsing object
				break;
			}
			else
			{
				if(finish)
				{
					fprintf(stderr, "Expected , and got end of object on line %d\n", line);
					exit(1);
				}

				// read another field
				skip_ws(json);
				char* key = parse_string(json);
				skip_ws(json);
				expect_c(json, ':');
				skip_ws(json);
				
				//printf("Key: %s on line %d\n", key, line);

				if(objtype == T_CAMERA)
				{
					if(strcmp(key, "width") == 0)
					{
						float value = next_number(json);
						camera_width = value;
						set_camera_width = 1;
					}
					else if(strcmp(key, "height") == 0)
					{
						float value = next_number(json);
						camera_height = value;
						set_camera_height = 1;
					}
				}
				else if(strcmp(key, "radius") == 0)
				{
					if(objtype == T_SPHERE)
					{
						float value = next_number(json);
						radius = value;
						set_radius = 1;
					}
				}
				else if(strcmp(key, "color") == 0)
				{
					float* v3 = next_vector(json);
					color[0] = v3[0];
					color[1] = v3[1];
					color[2] = v3[2];
					set_color = 1;
				}
				else if(strcmp(key, "position") == 0)
				{
					float* v3 = next_vector(json);
					position[0] = v3[0];
					position[1] = v3[1];
					position[2] = v3[2];
					set_position = 1;
				}
				else if(strcmp(key, "normal") == 0)
				{
					float* v3 = next_vector(json);
					normal[0] = v3[0];
					normal[1] = v3[1];
					normal[2] = v3[2];
					set_normal = 1;
				}
				else
				{
					fprintf(stderr, "Error: unknown property: %s on line %d\n", key, line);
					exit(1);
				}

				skip_ws(json);
				c = next_c(json);

				if(c != ',')
					finish = 1;

				ungetc(c, json);
			}
		}
		
		
		// error checking to make sure the correct attributes were read in
		// and set the attributes to the last object in the buffer

		if(objtype == T_CAMERA)
		{
			if(set_camera_height != 1)
			{
				fprintf(stderr, "Camera must have a height! Line %d\n", line);
				exit(1);
			}
			if(set_camera_width != 1)
			{
				fprintf(stderr, "Camera must have a width! Line %d\n", line);
				exit(1);
			}
			if(set_position == 1)
				fprintf(stderr, "Warning, Camera does not use position at this time.\n");
			if(set_normal == 1)
				fprintf(stderr, "Warning, Camera does not use a normal vector at this time.\n");
			
			scene.camera_width = camera_width;
			scene.camera_height = camera_height;
			
		}
		if(objtype == T_SPHERE)
		{
			if(set_radius != 1)
			{
				fprintf(stderr, "Sphere must have a defined radius! Line %d\n", line);
				exit(1);
			}
			if(radius < 0)
			{
				fprintf(stderr, "Sphere must have a non-negative radius! Line %d\n", line);
				exit(1);
			}
			if(set_color != 1)
			{
				fprintf(stderr, "Object must have a color! Line %d\n", line);
				exit(1);
			}
			if(set_position != 1)
			{
				fprintf(stderr, "Object must have a position! Line %d\n", line);
				exit(1);
			}
			
			// compute properties of a sphere

			new_object.color[0] = color[0];
			new_object.color[1] = color[1];
			new_object.color[2] = color[2];
			
			new_object.a = position[0];
			new_object.b = position[1];
			new_object.c = position[2];
			new_object.d = radius;
			
		}
		if(objtype == T_PLANE)
		{
			if(set_color != 1)
			{
				fprintf(stderr, "Object must have a color! Line %d\n", line);
				exit(1);
			}
			if(set_position != 1)
			{
				fprintf(stderr, "Object must have a position! Line %d\n", line);
				exit(1);
			}
			if(set_normal != 1)
			{
				fprintf(stderr, "Plane must have a normal vector! Line %d\n", line);
				exit(1);
			}
			
			// calculate the properties of the plane

			new_object.color[0] = color[0];
			new_object.color[1] = color[1];
			new_object.color[2] = color[2];
			
			new_object.a = normal[0];
			new_object.b = normal[1];
			new_object.c = normal[2];
			new_object.d = normal[0] * position[0] + normal[1] * position[1] + normal[2] * position[2];
			
		}
		
		// increment number to move to the next object

		if(objtype != T_CAMERA)
		{
			scene.objects[scene.num_objects] = new_object;
			scene.num_objects ++;
		}
		
		// continue with reading
		
		skip_ws(json);
		c = next_c(json);
		
		if (c == ',')
		{
			// continue
			skip_ws(json);
		}
		else if (c == ']') 
		{
			fclose(json);
			return scene;
		}
		else 
		{
			fprintf(stderr, "Error: Expected ] or , on line %d\n", line);
			exit(1);
		}
		
		// end parsing object
		skip_ws(json);
	}
	
	fclose(json);

	return scene;
}


float intersect_sphere(float* c, float R, float* r0, float* rd)
{
	// A = xd^2 + yd^2 + zd^2
	// B = 2 * (xd * (x0 - xc) + yd * (y0 - yc) + zd * (z0 - zc))
	// C = (x0 - xc)^2 + (y0 - yc)^2 + (z0 - zc)^2 - r^2
	
	float A = sqr(rd[0]) + sqr(rd[1]) + sqr(rd[2]);
	float B = 2 * (rd[0] * (r0[0] - c[0]) + rd[1] * (r0[1] - c[1]) + rd[2] * (r0[2] - c[2]));
	float C = sqr(r0[0] - c[0]) + sqr(r0[1] - c[1]) + sqr(r0[2] - c[2]) - sqr(R);
	
	float* zeroes = quadratic_formula(A, B, C);
	
	if(isnan(zeroes[0]))
		return -1;
	
	if(zeroes[0] > 0) return zeroes[0];
	if(zeroes[1] > 0) return zeroes[1];
	return -1;
}

float intersect_plane(float a, float b, float c, float d, float* r0, float* rd)
{
	// t = -(a*x0 + b*y0 + c*z0) / (a*xd + b*yd + c*zd)
	
	return (a*r0[0] + b*r0[1] + c*r0[2] + d) / (a*rd[0] + b*rd[1] + c*rd[2]);
}


void raycast(Scene scene, char* outfile, PPMmeta fileinfo)
{
	Pixel* data = malloc(sizeof(Pixel) * fileinfo.width * fileinfo.height);
	
	// raycasting here
	
	int N = fileinfo.width;
	int M = fileinfo.height;
	float w = scene.camera_width;
	float h = scene.camera_height;
	
	float pixel_height = h / M;
	float pixel_width = w / N;
	
	float p_z = 0;
	
	float c_x = 0;
	float c_y = 0;
	float c_z = 0;
	
	float r0[3];
	r0[0] = c_x;
	r0[1] = c_y;
	r0[2] = c_z;
	
	float rd[3];
	rd[2] = 1;
	
	int i;
	int j;
	int k;
	
	for(i = 0; i < M; i ++)
	{
		rd[1] = -c_y + h/2.0 - pixel_height * (i + 0.5);
		
		for(j = 0; j < N; j ++)
		{
			rd[0] = c_x - w/2.0 + pixel_width * (j + 0.5);
			
			float best_t = INFINITY;
			Object closest;

			for(k = 0; k < scene.num_objects; k ++)
			{
				float t = -1;
				Object o = scene.objects[k];
				
				if(o.kind == T_SPHERE)
				{
					float c[3];
					c[0] = o.a;
					c[1] = o.b;
					c[2] = o.c;
					t = intersect_sphere(c, o.d, r0, rd);
				}
				else if(o.kind == T_PLANE)
				{
					t = intersect_plane(o.a, o.b, o.c, o.d, r0, rd);
				}
				
				if(t > 0 && t < best_t)
				{
					best_t = t;
					closest = o;
				}
			}
			
			// write to Pixel* data
			
			Pixel pixel = data[i * N + j];
			if(best_t < INFINITY && closest.kind >= 0)
			{
				pixel.r = (char) (closest.color[0] * 255);
				pixel.g = (char) (closest.color[1] * 255);
				pixel.b = (char) (closest.color[2] * 255);
			}
			else
			{
				pixel.r = (char) (255* scene.background_color[0]);
				pixel.g = (char) (255* scene.background_color[1]);
				pixel.b = (char) (255* scene.background_color[2]);
			}

			data[i * N + j] = pixel;
			
		}
		
	}
	
	WritePPM(data, outfile, fileinfo);
}





int main(int argc, char** argv)
{
	if(argc < 5)
	{
		fprintf(stderr, "Usage: width height input.json output.ppm\n");
		exit(1);
	}
	
	PPMmeta fileinfo;
	fileinfo.width = atoi(argv[1]);
	fileinfo.height = atoi(argv[2]);
	fileinfo.max = 255;
	fileinfo.type = 6;
	
	Scene scene = read_scene(argv[3]);

	printf("Read in %d objects\n", scene.num_objects);

	scene.background_color[0] = 0.5;
	scene.background_color[1] = 0.51;
	scene.background_color[2] = 0.6;
	
	raycast(scene, argv[4], fileinfo);
	
	return 0;
}



