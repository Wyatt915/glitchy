#include <cstdlib>
#include <iostream>
#include <math.h>

#include "canny.hpp"
#include "imgutils.hpp"
#include "image.hpp"

//--------------------------------------[Image manipulations]---------------------------------------

void clamp(int& val, int min, int max){
    val = MAX(val, min);
    val = MIN(val, max);
}

//Convolution may produce pixels outside the range [0,1]
image convolution(const image& img, const matrix& kernel, double coef){
    image out(img.r(), img.c());
    double acc = 0;
    int k_off = (kernel.size() - 1) / 2;
    for(int row = 0; row < img.r(); row++){
        for(int col = 0; col < img.c(); col++){
            acc = 0;
            for(int i = 0; i < kernel.size(); i++){
                for(int j = 0; j < kernel[0].size(); j++){
                    int r = row + i - k_off;
                    int c = col + j - k_off;
                    clamp(c, 0, img.c() - 1); //Extend the edge pixels to infinity
                    clamp(r, 0, img.r() - 1);
                    acc += kernel[i][j] * img[r][c].y;
                }
            }
            acc *= coef;
            out[row][col] = pixel(acc);
        }
    }
    return out;
}

//clips pixels < 0 to 0 and pixels > 1 to 1.
void clip(image& img){
    for(int i = 0; i < img.r(); i++){
        for(int j = 0; j < img.c(); j++){
            img[i][j].y = MAX(0.0, img[i][j].y);
            img[i][j].y = MIN(img[i][j].y, 1.0);
        }
    }
}

//linear map of pixel values from range [a, b] to [0, 1]
void remap(image& img, double a, double b){
    for(int i = 0; i < img.r(); i++){
        for(int j = 0; j < img.c(); j++){
            img[i][j].y = (img[i][j].y - a) * (1.0 / (b - a));
        }
    }
}

image gaussian(const image& img){
    matrix kernel = {{2, 4,  5,  4,  2},
                     {4, 9,  12, 9,  4},
                     {5, 12, 15, 12, 5},
                     {4, 9,  12, 9,  4},
                     {2, 4,  5,  4,  2}};
    matrix kx = {{0.0545, 0.2442, 0.4026, 0.2442, 0.0545}};
    matrix ky = {{0.0545},
                 {0.2442},
                 {0.4026},
                 {0.2442},
                 {0.0545}};
    return convolution(convolution(img, kx), ky);
    //return convolution(img, kernel, 1.0/159.0);
}

image magnitude(const image& mx, const image& my){
    image out(mx.r(), mx.c());
    for(int i = 0; i < mx.r(); i++){
        for(int j = 0; j < mx.c(); j++){
            out[i][j] = pixel(sqrt((mx[i][j].y * mx[i][j].y) + (my[i][j].y * my[i][j].y)));
        }
    }
    return out;
}

int roundangle(int theta){
    if(theta < 23) return 0;
    if(theta < 68) return 45;
    if(theta < 113) return 90;
    if(theta < 156) return 135;
    return 0;
}

matrix angle(const image& mx, const image& my){
    matrix out;
    out.resize(mx.r());
    for(int i = 0; i < mx.r(); i++){
        out[i].resize(mx.c());
    }
    for(int i = 0; i < mx.r(); i++){
        for(int j = 0; j < mx.c(); j++){
            out[i][j] = roundangle(360 * (atan2(my[i][j].y, mx[i][j].y) / (2*PI)));
        }
    }
    return out;
}

//simple threshold
void threshold(image& img, double val){
    for(int i = 0; i < img.r(); i++){
        for(int j = 0; j < img.c(); j++){
            if(img[i][j].y >= val) img[i][j] = pixel(1.0);
            else img[i][j] = pixel(0);
        }
    }
}

void downscale(image& img){
    image temp(img.r()/2, img.c()/2);
    double r, g, b;
    for(int row = 0; row + 2 <= img.r(); row += 2){
        for(int col = 0; col + 2 <= img.c(); col += 2){
            r = 0; g = 0; b = 0;
            for(int i = 0; i < 2; i++){
                for(int j = 0; j < 2; j++){
                    r += img[row+i][col+j].r;
                    g += img[row+i][col+j].g;
                    b += img[row+i][col+j].b;
                }
            }
            r /= 4; g /= 4; b /= 4;
            temp[row/2][col/2] = pixel(r, g, b);
        }
    }
    img = temp;
}

int int_to_utf8(uint32_t val){
    int width = 0; //The width of the output character in bytes.
    if (val < 0x80){
        //trivial case - this is just ASCII. We're done.
        putchar(val);
        return 0;
    }
    else if (val < 0x00000800)  width = 2;
    else if (val < 0x00010000)  width = 3;
    else if (val < 0x00200000)  width = 4;
    else if (val < 0x04000000)  width = 5;
    else if (val < 0x80000000)  width = 6;
    else{
        fprintf(stderr, "Illegal codepoint.\n");
        return 1;
    }

    uint8_t firstbits = 7 - width;
    uint8_t totalbits = firstbits + (width - 1) * 6;
    //This bitwise hacking produces a byte with a number of leading 1s equal to the value <width>.
    //For example, if width == 5, then msb_mask will be 0b11111000.
    uint8_t msb_mask = ~((1 << (firstbits + 1)) - 1);

    //The first byte is the leading 1s defined by msb_mask followed by the first few bits of <val>.
    putchar(msb_mask | (val >> (totalbits - firstbits)));
    //The remaining bytes are prefixed with 0b10 and contain 6 bits of data.
    char byte;
    for (int i = width - 2; i >= 0; i--){
        byte = 0x3f & (val >> 6 * i); //0x3f isolates the six least significant bits
        byte |= 0x80;
        putchar(byte);
    }
    return 0;
}
