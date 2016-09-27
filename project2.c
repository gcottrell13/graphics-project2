#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "3dmath.c"

#define T_CAMERA 1
#define T_SPHERE 2
#define T_PLANE 3

int line = 1;

typedef struct {
	int kind;
	double color[3];
	double a;
	double b;
	double c;
	double d;
} Object;

typedef struct {
	int num_objects;
	Object objects[128];
	double camera_width;
	double camera_height;
	double background_color[3]; // for fun!
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
}
int expect_c(FILE* file, int d)
{
	int c = next_c(file);
	if (c == d) return;
	fprintf(stderr, "Error: expected %c got %c on line %d\n", d, c, line);
	exit(1);
}
void skip_ws(FILE* file)
{
	int c = next_c(file);
	while(c == 10 || c == ' ')
	{
		c = next_c(file);
	}
	ungetc(c, file);
}

double next_number(FILE* file)
{
	double val;
	fscanf(file, "%f", &value);
	// error
	return val;
}

double* next_vector(FILE* file)
{
	double* v = malloc(3 * sizeof(double));
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
	int c = next_c(file);
	
	if(c != '"') 
	{
		fprintf(stderr, "Error: expected \" on line %d\n", line);
		exit(1);
	}
	
	int max_size = 128;
	
	char buffer[max_size];
	int buff_size = 0;
	
	c = next_c(file);
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
	int c;
	
	skip_ws(json);
	
	// find beginning of file
	expect_c(json, '[');
	
	skip_ws(json);
	
	c = next_c(json);
	if (c == ']')
	{
		fprintf(srderr, "Warning: empty scene file.\n");
		return 0;
	}
	
	Scene scene = malloc(sizeof(Scene));
	
	// these will hold data for any objects parsed later
	Object new_object = malloc(sizeof(Object));
	
	int set_camera_width = 0;
	double camera_width = 0;
	int set_camera_height = 0;
	double camera_height = 0;
	
	while(1)
	{
		expect_c(json, '{');
		
		// parse an object
		char* key = parse_string(json);
		if (strcmp(key, "type") != 0) {
			fprintf(stderr, "Error: expected \"type\" key on line %d\n", line);
			exit(1);
		}
		
		skip_ws(json);
		
		expect_c(json, ':');
		
		skip_ws(json);
		
		char* value = parse_string(json);
		
		int objtype = 0;
		
		int set_radius = 0;
		double radius = 0; // just in case a sphere is read in
		int set_color = 0;
		double color[3];
		int set_normal = 0;
		double normal[3];
		int set_position = 0;
		double position[3];
		
		if(strcmp(value, "camera") == 0) {
			objtype = T_CAMERA;
		} else if(strcmp(value, "sphere") == 0) {
			objtype = T_SPHERE;
		} else if(strcmp(value, "plane") == 0) {
			objtype = T_PLANE;
		} else {
			
			fprintf(stderr, "Unknown type \"%s\" on line %d\n", value, line);
			exit(1);
		}
		
		// copy the information into the new object
		new_object.kind = objtype;
		
		skip_ws(json);
		
		while(1) 
		{
			c = next_c(json);
			if (c == '}')
			{
				// stop parsing object
				break;
			}
			else if (c == ',')
			{
				// read another field
				skip_ws(json);
				char* key = parse_string(json);
				skip_ws(json);
				expect_c(json, ':');
				skip_ws(json);
				
				if(objtype == T_CAMERA)
				{
					if(strcmp(key, "width") == 0)
					{
						double value = next_number(json);
						camera_width = value;
						set_camera_width = 1;
					}
					else if(strcmp(key, "height") == 0)
					{
						double value = next_number(json);
						camera_height = value;
						set_camera_height = 1;
					}
				}
				else if(strcmp(key, "radius") == 0)
				{
					if(objtype == T_SPHERE)
					{
						double value = next_number(json);
						radius = value;
						set_radius = 1;
					}
				}
				else if(strcmp(key, "color") == 0)
				{
					double* v3 = next_vector(json);
					color = v3;
					set_color = 1;
				}
				else if(strcmp(key, "position") == 0)
				{
					double* v3 = next_vector(json);
					color = v3;
					set_color = 1;
				}
				else if(strcmp(key, "normal") == 0)
				{
					double* v3 = next_vector(json);
					color = v3;
					set_color = 1;
				}
				else
					{
						fprintf(stderr, "Error: unknown property: %s on line %d\n", key, line);
						exit(1);
					}
				skip_ws(json);
			}
			else 
			{
				fprintf(stderr, "Error: unexpected value on line %d\n", line);
				exit(1);
			}
		}
		
		
		// error checking to make sure the correct attributes were read in
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
			
			new_object.a = position[0];
			new_object.b = position[1];
			new_object.c = position[2];
			new_object.d = radius * radius;
			
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
			
			new_object.a = normal[0];
			new_object.b = normal[1];
			new_object.c = normal[2];
			new_object.d = normal[0] * position[0] + normal[1] * position[1] + normal[2] * position[2];
			
		}
		
		// store read object and create a new empty one
		scene.objects[scene.num_objects] = new_object;
		scene.num_objects ++;
		new_object = malloc(sizeof(Object));
		
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
			return 0;
		}
		else 
		{
			fprintf(stderr, "Error: Expected ] or , on line %d\n", line);
			exit(1);
		}
		
		// end parsing object
		
		skip_ws(json);
		c = next_c(json);
	}
	
	fclose(json);
	return scene;
}


double intersect_sphere(double* C, double R, double* r0, double* rd)
{
	// A = xd^2 + yd^2 + zd^2
	// B = 2 * (zd * (x0 - xc) + yd * (y0 - yc) + xd * (z0 - zc))
	// C = (x0 - xc)^2 + (y0 - yc)^2 + (z0 - zc)^2 - r^2
	
	double A = sqr(rd[0]) + sqr(rd[1]) + sqr(rd[2]);
	double B = 2 * (rd[2] * (r0[0] - C[0]) + rd[1] * (r0[1] - C[1]) + rd[0] * (r0[2] - C[2]));
	double C = sqr(r0[0] - C[0]) + sqr(r0[1] - C[1]) + sqr(r0[2] - C[2]) - sqr(R);
	
	double* zeroes = quadratic_formula(A, B, C);
	
	if(isnan(zeroes[0]))
		return -1;
	
	if(zeroes[0] > 0) return zeroes[0];
	if(zeroes[1] > 0) return zeroes[1];
	return -1;
}

double intersect_plane(double a, double b, double c, double* r0, double* rd)
{
	// t = -(a*x0 + b*y0 + c*z0) / (a*xd + b*yd + c*zd)
	
	return -(a*r0[0] + b*r0[1] + c*r0[2]) / (a*rd[0] + b*rd[1] + c*rd[2]);
}


void raycast(Scene scene, char* outfile, PPMmeta fileinfo)
{
	Pixel* data = malloc(sizeof(Pixel) * fileinfo.width * fileinfo.height);
	
	// raycasting here
	
	int N = fileinfo.width;
	int M = fileinfo.height;
	double w = scene.camera_width;
	double h = scene.camera_height;
	
	double pixel_height = h / M;
	double pixel_width = W / N;
	
	double p_z = -1;
	
	double c_x = 0;
	double c_y = 0;
	double c_z = 0;
	
	double r0[3];
	r0[2] = p_z;
	
	double rd[3];
	rd[0] = 0;
	rd[1] = 0;
	rd[2] = 1;
	
	int i;
	int j;
	int k;
	
	for(i = 0; i < M; i ++)
	{
		double p_y = c_y - h/2.0 + pixel_height * (i + 0.5);
		r0[1] = p_y;
		
		for(j = 0; j < N; j ++)
		{
			double p_x = c_x - w/2.0 + pixel_width * (j + 0.5);
			r0[0] = p_x;
			
			double best_t = INFINITY;
			Object closest;
			
			for(k = 0; k < scene.num_objects; k ++)
			{
				double t;
				Object o = scene.objects[k];
				
				if(o.kind == T_SPHERE)
				{
					double c[3];
					c[0] = o.a;
					c[1] = o.b;
					c[2] = o.c;
					
					t = intersect_sphere(c, o.d, r0, rd);
				}
				else if(o.kind == T_PLANE)
				{
					t = intersect_plane(o.a, o.b, o.c, r0, rd);
				}
				
				if(t > 0 && t < best_t)
				{
					best_t = t;
					closest = o;
				}
			}
			
			// write to Pixel* data
			
			Pixel pixel;
			if(best_t < INFINITY)
			{
				pixel.r = closest.color[0];
				pixel.g = closest.color[1];
				pixel.b = closest.color[2];
			}
			else
			{
				pixel.r = scene.background_color[0];
				pixel.g = scene.background_color[1];
				pixel.b = scene.background_color[2];
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
	
	scene.background_color[0] = 100;
	scene.background_color[1] = 110;
	scene.background_color[2] = 160;
	
	raycast(scene, argv[4], fileinfo);
	
	return 0;
}



