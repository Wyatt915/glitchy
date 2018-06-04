#include <algorithm>    //std::sort
#include <cmath>        //ceil
#include <cstdlib>      //rand
#include <ctime>
#include <iostream>
#include <unistd.h>

#include "imgutils.hpp"
#include "canny.hpp"

//-----------------------------------------[Pixel Sorting]------------------------------------------

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


int main(int argc, char* argv[]){
    opterr = 0;     // don't print error messages
    int c, flag;
    bool has_image = false;
    image img;
    
    while ((c = getopt(argc, argv, "aehi:s")) != -1) {
        switch (c) {
            case 'a':
                flag = c;
                break;
            case 'e':
                flag = c;
                break;
            case 'h':
                std::cout << "Options:\n"
                          << "\t-a\tPrint an ASCII representation of the image.\n"
                          << "\t-e\tEdge detection and print a PPM image to stdout.\n"
                          << "\t-h\tPrint this message.\n"
                          << "\t-i file\tInput file (PPM)\n"
                          << "\t-s\tSort pixels and print a PPM image to stdout.\n\n"
                          << "Any PPM image can either be written to a file and viewed with most "
                          << "image software, or it can be piped directly into "
                          << "ImageMagick's \033[1mdisplay\033[0m program.\n";
                return 0;
            case 'i':
                has_image = true;
                img = readppm(std::string(optarg));
                break;
            case 's':
                flag = c;
                break;
            case '?':
                std::cerr << "Unknown option.\n";
                return 1;
            default:
                std::cerr << "Unknown error.\n";
                return 2;
        }
    }
    
    if(!has_image) {
        std::cerr << "No imput specified.\n";
        return 3;
    }

    switch(flag) {
        case 'a':
            to_ascii(img);
            return 0;
        case 'e':
            printppm(canny(img));
            return 0;
        case 's':
            pixelsort(img, canny(img));
            printppm(img);
            return 0;
    }
}
