#pragma once

#include <vector>
#include <string>
#include <cmath>

#define MAX(X,Y) ((X)>(Y)?(X):(Y))
#define MIN(X,Y) ((X)<(Y)?(X):(Y))

#define PI 3.14159265358

//--------------------------------------------[Globals]---------------------------------------------

extern int WIDTH, HEIGHT;
extern std::string FORMAT;

//--------------------------------------[Typedefs and Structs]--------------------------------------

struct coord{
    int row;
    int col;
    coord(int a, int b):row(a), col(b) {}
};

struct pixel{
    bool gray;
    double r, g, b;
    double y;
    pixel(double red, double grn, double blu): 
        r(red), g(grn), b(blu)
        {
            gray = false;
            //RGB to Luma realtion per ITU BT.601
            //https://stackoverflow.com/a/596241
            y = (0.299 * r) + (0.587 * g) + (0.114 * b);
        }
    pixel(double luma): y(luma), gray(true) {
        r = y;
        g = y;
        b = y;
    }
    pixel():r(0), g(0), b(0), y(0), gray(true) {}
};

typedef std::vector<std::vector<double> > matrix;
typedef std::vector<std::vector<pixel> > image;

//----------------------------------------------[I/O]-----------------------------------------------

image readppm(std::string);
void printppm(const image&);

//-------------------------------------------[Functions]--------------------------------------------

image convolution(const image&, const matrix& kernel, double coef = 1.0);
image gaussian(const image&);
image magnitude(const image& x, const image& y);
image newimage();
matrix angle(const image& x, const image& y);
void threshold(image&, double value);
