#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define CAMERA 1
#define SPHERE 2
#define PLANE 3

int line = 1;

typedef struct {
	double a;
	double b;
	double c;
	double d;
} Plane;
typedef struct {
	double a;
	double b;
	double c;
} Sphere;

typedef struct {
	unsigned int spheres[128]; // holds addresses for sphere structs, structs should be malloc'ed
	unsigned int planes[128]; // holds addresses for plane structs
	int num_spheres;
	int num_planes;
	double camera_width;
	double camera_height;
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
	while(isspace(c))
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

int read_scene(char* json_name)
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
	
	// these will hold data for any planes and spheres parsed later
	Plane _plane = malloc(sizeof(Plane));
	Sphere _sphere = malloc(sizeof(Plane));
	
	int set_camera_width = 0;
	int set_camera_height = 0;
	
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
		int radius = 0; // just in case a sphere is read in
		int set_color = 0;
		double color[3];
		int set_normal = 0;
		double normal[3];
		int set_position = 0;
		double position[3];
		
		if(strcmp(value, "camera") == 0) {
			objtype = CAMERA;
		} else if(strcmp(value, "sphere") == 0) {
			objtype = SPHERE;
		} else if(strcmp(value, "plane") == 0) {
			objtype = PLANE;
		} else {
			
			fprintf(stderr, "Unknown type \"%s\" on line %d\n", value, line);
			exit(1);
		}
		
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
				
				if((strcmp(key, "width") == 0) || 
					(strcmp(key, "height") == 0) || 
					(strcmp(key, "radius") == 0))
					{
						double value = next_number(json);
						if(objtype == CAMERA)
						{
							if(strcmp(key, "width") == 0)
							{
								camera_width = value;
								set_camera_width = 1;
							}
							else if(strcmp(key, "height") == 0)
							{
								camera_height = value;
								set_camera_height = 1;
							}
						}
						if(objtype == SPHERE)
						{
							radius = value;
							set_radius = 1;
						}
					}
				else if((strcmp(key, "color") == 0) || 
					(strcmp(key, "position") == 0) || 
					(strcmp(key, "normal") == 0))
					{
						double* v3 = next_vector(json);
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
		if(objtype == CAMERA)
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
		}
		if(objtype == SPHERE)
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
		}
		if(objtype == PLANE)
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
		}
		
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
	return 0;
}


int main(int argc, char** argv)
{
	
	return 0;
}



