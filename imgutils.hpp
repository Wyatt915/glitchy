#pragma once

#include <vector>
#include <string>
#include <cmath>

#define MAX(X,Y) ((X)>(Y)?(X):(Y))
#define MIN(X,Y) ((X)<(Y)?(X):(Y))
#define ABS(N)   ((N<0)?(-N):(N))

#define PI 3.14159265358

//--------------------------------------[Typedefs and Structs]--------------------------------------

struct coord{
    int row;
    int col;
    coord(int a, int b):row(a), col(b) {}
};

typedef std::vector<std::vector<double> > matrix;

class image;

//-------------------------------------------[Functions]--------------------------------------------

image convolution(const image&, const matrix& kernel, double coef = 1.0);
image gaussian(const image&);
image magnitude(const image& x, const image& y);
image newimage();
void downscale(image&);
matrix angle(const image& x, const image& y);
void threshold(image&, double value);
void clip(image&);
void clamp(int&, int, int);
int int_to_utf8(uint32_t);
void remap(image&, double, double);
