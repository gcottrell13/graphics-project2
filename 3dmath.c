
#include <math.h>

void add(double* a, double* b, double* out)
{
	out[0] = a[0] + b[0];
	out[1] = a[1] + b[1];
	out[2] = a[2] + b[2];
}
void subtract(double* a, double* b, double* out)
{
	out[0] = a[0] - b[0];
	out[1] = a[1] - b[1];
	out[2] = a[2] - b[2];
}
void scale(double* a, double b, double* out)
{
	out[0] = a[0] * b;
	out[1] = a[1] * b;
	out[2] = a[2] * b;
}
double dot(double* a, double* b)
{
	return a[0]*b[0] + a[1]*b[1] + a[2]*b[2];
}
void cross(double* a, double*b, double* out)
{
	out[0] = a[1]*b[2] - a[2]*b[1];
	out[1] = a[2]*b[0] - b[2]*a[0];
	out[2] = a[0]*b[1] - a[1]*b[0];
}

static inline double sqr(double v)
{
	return v*v;
}

double* quadratic_formula(double a, double b, double c)
{
	double results[2];
	
	double det = sqr(b) - 4 * a * c;
	if(det < 0) {
		results[0] = NAN;
		return results;
	}
	
	det = sqrt(det);
	
	results[0] = (-b - det) / (2 * a);
	results[1] = (-b + det) / (2 * a);
	
	return results;
}


