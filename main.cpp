#include <cmath>        //ceil
#include <cstdlib>      //rand
#include <ctime>
#include <iostream>
#include <unistd.h>

#include "imgutils.hpp"
#include "canny.hpp"
#include "image.hpp"
#include "ppm.hpp"

//-----------------------------------------[Pixel Sorting]------------------------------------------

int main(int argc, char* argv[]){
    opterr = 0;     // don't print error messages
    int c, flag;
    bool has_image = false;
    image img;
    srand(time(NULL));
    while ((c = getopt(argc, argv, "abdehi:s")) != -1) {
        switch (c) {
            case 'a':
                flag = c;
                break;
            case 'b':
                flag = c;
                break;
            case 'd':
                flag = c;
                break;
            case 'e':
                flag = c;
                break;
            case 'h':
                std::cout << "Options:\n"
                          << "\t-a\tPrint an ASCII representation of the image.\n"
                          << "\t-d\tPrint 1-bit dithered image to stdout\n"
                          << "\t-e\tEdge detection and print a PPM image to stdout.\n"
                          << "\t-h\tPrint this message.\n"
                          << "\t-i file\tInput file (PPM format only). If this option is not specified, read from stdin.\n"
                          << "\t-s\tSort pixels and print a PPM image to stdout.\n\n"
                          << "Any PPM image can either be written to a file and viewed with most "
                          << "image software, or it can be piped directly into "
                          << "ImageMagick's \033[1mdisplay\033[0m program.\n";
                return 0;
            case 'i':
                has_image = true;
                img = openppm(std::string(optarg));
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
        img = readppm(std::cin);
    }

    // The reason that this is not just handled in the getopt block is so that we maintain the
    // flexibility of reading an image from stdin, or specifying it with -i, and having the program
    // automatically detect which option is taken.

    switch(flag) {
        case 'a':
            to_ascii(img);
            return 0;
        case 'b':
            to_braille(img);
            return 0;
        case 'e':
            return printppm(canny(img));
        case 'd':
            dither(img);
            return printppm(img);
        case 's':
            pixelsort(img, canny(img));
            return printppm(img);
    }
}
