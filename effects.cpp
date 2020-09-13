#include <algorithm>
#include <iostream>

#include <sys/ioctl.h> //ioctl() and TIOCGWINSZ
#include <unistd.h> // for STDOUT_FILENO

#include "effects.hpp"
#include "image.hpp"
#include "imgutils.hpp"

//stochastic dither
image sdither(const image& img){
    image out(img.r(), img.c());
    for(int i = 0; i < img.r(); i++){
        for(int j = 0; j < img.c(); j++){
            out[i][j] = pixel(img[i][j].y * 1000 > rand() % 1000);
            //out[i][j] = pixel(img[i][j].y);
        }
    }
    out.set_format("P2");
    return out;
}

//floyd-steinberg dither
void dither(image& img){
    pixel oldpixel;
    pixel newpixel;
    double q_error;
    for(int i = 0; i < img.r(); i++){
        for(int j = 0; j < img.c(); j++){
            oldpixel = img[i][j];
            newpixel = pixel(oldpixel.y > 0.5);
            img[i][j] = newpixel;
            q_error = oldpixel.y - newpixel.y;
            if (j < img.c() - 1)            img[i  ][j+1].y += q_error * 7.0 / 16.0;
            if (j > 0 && i < img.r() - 1)   img[i+1][j-1].y += q_error * 3.0 / 16.0;
            if (i < img.r() - 1)            img[i+1][j  ].y += q_error * 5.0 / 16.0;
            if (i<img.r()-1 && j<img.c()-1) img[i+1][j+1].y += q_error * 1.0 / 16.0;
        }
    }
    img.set_format("P1");
}

double find_closest_palette_color(double color, int colordepth){
    double step = 1.0 / double(colordepth - 1);
    double palette_color = 0.0;
    while(palette_color < color){
        palette_color += step;
    }
    //if (ABS(color - palette_color) > ABS(color - (palette_color - step))){
    //    palette_color -= step;
    //}
    return MIN(palette_color, 1.0f);
}

//floyd-steinberg dither
void dither(image& img, int colordepth){
    pixel oldpixel;
    pixel newpixel;
    double q_error;
    for(int i = 0; i < img.r(); i++){
        for(int j = 0; j < img.c(); j++){
            oldpixel = img[i][j];
            newpixel = pixel(find_closest_palette_color(oldpixel.y, colordepth));
            img[i][j] = newpixel;
            q_error = oldpixel.y - newpixel.y;
            if (j < img.c() - 1)            img[i  ][j+1].y += q_error * 7.0 / 16.0;
            if (j > 0 && i < img.r() - 1)   img[i+1][j-1].y += q_error * 3.0 / 16.0;
            if (i < img.r() - 1)            img[i+1][j  ].y += q_error * 5.0 / 16.0;
            if (i<img.r()-1 && j<img.c()-1) img[i+1][j+1].y += q_error * 1.0 / 16.0;
        }
    }
    img.set_format("P2");
}

void to_ascii(const image& original){
    std::cout << '\n';
    //std::string pix = " .'`^\",:;Il!i><~+_-?][}{1)(|\\/tfjrxnuvczXYUJCLQ0OZmwqpdbkhao*#MW&8%B@$";
    std::string pix = " .:-=+*#%@";
    int colordepth = pix.length();
    image img = original;
    // get terminal size
    struct winsize size;
    ioctl(STDOUT_FILENO, TIOCGWINSZ, &size);
    // each ASCII character is repeated twice
    while(img.c() > size.ws_col / 2){
        downscale(img);
    }
    dither(img, colordepth);
    double avg, count;
    for(int i = 0; i < img.r(); i++){
        for(int j = 0; j < img.c(); j++){
            char c = pix[int(round(img[i][j].y*colordepth))];
            std::cout << c << c;
        }
        std::cout << '\n';
    }
}
void to_braille(image img){
    //unicode 8-dot braille starts at codepoint U+2800,
    //and the dots are arranged as follows:
    // 0 3
    // 1 4
    // 2 5
    // 6 7

    // get terminal size
    struct winsize size;
    ioctl(STDOUT_FILENO, TIOCGWINSZ, &size);
    // each braille character has a width of 2 pixels
    while(img.c() > size.ws_col * 2){
        downscale(img);
    }
    dither(img); //get the image to 1-bit b&w
    int codepoint = 0;
    for(int row = 0; row + 4 <= img.r(); row += 4){
        for(int col = 0; col + 2 <= img.c(); col += 2){
            codepoint = 0;
            codepoint |= int(img[row+0][col+0].y);
            codepoint |= int(img[row+1][col+0].y) << 1;
            codepoint |= int(img[row+2][col+0].y) << 2;
            codepoint |= int(img[row+0][col+1].y) << 3;
            codepoint |= int(img[row+1][col+1].y) << 4;
            codepoint |= int(img[row+2][col+1].y) << 5;
            codepoint |= int(img[row+3][col+0].y) << 6;
            codepoint |= int(img[row+3][col+1].y) << 7;
            int_to_utf8(codepoint + 0x2800);
        }
        putchar('\n');
    }
    putchar('\n');
}


void pixelsort(image& img, const image& edge){
    int offset = 0;
    int count = 0;
    int prevpx = 0; //Default to the previous pixel not being an edge
    for(int i = 0; i < img.r(); i++){
        for(int j = 0; j < img.c(); j++){
            //if we change from black to white or vice versa
            if(edge[i][j].y != prevpx || j == img.c() - 1){
                count++;
            //}
            //if(count %2){
                std::sort(begin(img[i]) + offset, begin(img[i]) + j,
                    [](const pixel& a, const pixel& b){
                        return a.y < b.y;
                    }
                );
                offset = j;
                prevpx = edge[i][j].y;
            }
        }
        offset = 0;
        prevpx = 0;
    }
}

void jitter(image& img, int radius){
    srand(time(NULL));
    int xoff, yoff;
    pixel temp;
    for(int i = 0; i < img.r(); i++){
        for(int j = 0; j < img.c(); j++){
            xoff = rand() % radius - ceil(double(radius) / 2.0);
            yoff = rand() % radius - ceil(double(radius) / 2.0);
            clamp(xoff, 0, img.c());
            clamp(yoff, 0, img.r());
            temp = img[i][j];
            img[i][j] = img[yoff][xoff];
            img[yoff][xoff] = temp;
        }
    }
}
