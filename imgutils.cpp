#include "imgutils.hpp"
#include <fstream>
#include <iostream>
#include <sstream>

int WIDTH, HEIGHT;
std::string FORMAT;


void clamp(int& val, int min, int max){
    val = MAX(val, min);
    val = MIN(val, max);
}

image newimage(){
    image out;
    out.resize(HEIGHT);
    for(int i = 0; i < HEIGHT; i++){
        out[i].resize(WIDTH);
    }
    return out;
}

image convolution(const image& img, const matrix& kernel, double coef){
    image out = newimage();
    double acc = 0;
    int k_off = (kernel.size() - 1) / 2;
    for(int row = 0; row < HEIGHT; row++){
        for(int col = 0; col < WIDTH; col++){
            acc = 0;
            for(int i = 0; i < kernel.size(); i++){
                for(int j = 0; j < kernel[0].size(); j++){
                    int r = row + i - k_off;
                    int c = col + j - k_off;
                    clamp(c, 0, WIDTH - 1); //Extend the edge pixels to infinity
                    clamp(r, 0, HEIGHT - 1);
                    acc += kernel[i][j] * img[r][c].y;
                }
            }
            acc *= coef;
            acc = MAX(0.0, acc);
            acc = MIN(acc, 1.0);
            out[row][col] = pixel(acc);
        }
    }
    return out;
}

image gaussian(const image& img){
    matrix kernel = {{2, 4,  5,  4,  2},
                     {4, 9,  12, 9,  4},
                     {5, 12, 15, 12, 5},
                     {4, 9,  12, 9,  4},
                     {2, 4,  5,  4,  2}};
    return convolution(img, kernel, 1.0/159.0);
}

image magnitude(const image& mx, const image& my){
    image out = newimage();
    for(int i = 0; i < HEIGHT; i++){
        for(int j = 0; j < WIDTH; j++){
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
    out.resize(HEIGHT);
    for(int i = 0; i < HEIGHT; i++){
        out[i].resize(WIDTH);
    }
    for(int i = 0; i < HEIGHT; i++){
        for(int j = 0; j < WIDTH; j++){
            out[i][j] = roundangle(360 * (atan2(my[i][j].y, mx[i][j].y) / (2*PI)));
        }
    }
    return out;
}

//simple threshold
void threshold(image& img, double val){
    for(int i = 0; i < HEIGHT; i++){
        for(int j = 0; j < WIDTH; j++){
            if(img[i][j].y >= val) img[i][j] = pixel(1.0);
            else img[i][j] = pixel(0);
        }
    }
}

//----------------------------------------------[I/O]-----------------------------------------------


image readppm(std::string fname){
    std::ifstream infile;
    infile.open(fname);
    if (!infile){
        std::cerr << "Unable to open file.\n";
        exit(1);
    }
    std::string line;
    char c = '#';
    do{
        std::getline(infile, line);
    } while(line[0] == c);  //loop until <line> is not a comment.
    FORMAT = line;
    do{
        std::getline(infile, line);
    } while(line[0] == c);
    std::stringstream linestream(line);
    linestream >> WIDTH >> HEIGHT;
    do{
        std::getline(infile, line);
    } while(line[0] == c);
    double max = std::stod(line);
    image out; 
    std::string lines = "";
    while(std::getline(infile, line)){
        if(line[0] != c) lines += line + " ";
    }
    linestream = std::stringstream(lines);
    std::vector<pixel> temp;
    double r, g, b, y;
    //Grayscale image
    if(FORMAT == "P2"){
        while(linestream >> y && out.size() < HEIGHT){
            temp.push_back(pixel(y/max));
            if(temp.size() == WIDTH){
                out.push_back(temp);
                temp.clear();
            }
        }
    }
    //Color Image
    else if(FORMAT == "P3"){
        while(linestream >> r >> g >> b && out.size() < HEIGHT){
            temp.push_back(pixel(r/max, g/max, b/max));
            if(temp.size() == WIDTH){
                out.push_back(temp);
                temp.clear();
            }
        }
    }
    else{
        std::cerr << "Unknown file type.\n";
        exit(2);
        infile.close();
    }
    infile.close();
    return out;
}

void printppm(const image& in){
    if(FORMAT == "P2"){
        std::cout << "P2\n" << WIDTH << ' ' << HEIGHT << "\n255\n";
        for(std::vector<pixel> i : in){
            for(pixel pix : i){
                std::cout << int(pix.y * 255) << ' ';
            }
            std::cout << std::endl;
        }
    }
    if(FORMAT == "P3"){
        std::cout << "P3\n" << WIDTH << ' ' << HEIGHT << "\n255\n";
        for(std::vector<pixel> i : in){
            for(pixel pix : i){
                std::cout << int(pix.r * 255) << ' ' << int(pix.g * 255) << ' ' << int(pix.b * 255) << ' ';
            }
            std::cout << std::endl;
        }
    }
}

void to_ascii(const image& img){
    std::cout << '\n';
    std::string pix = " .'`^\",:;Il!i><~+_-?][}{1)(|\\/tfjrxnuvczXYUJCLQ0OZmwqpdbkhao*#MW&8%B@$";
    int colordepth = pix.length();
    int scale = 75;
    int w = WIDTH/scale;
    int h = HEIGHT/scale;
    int avg, count;
    for(int i = 0; i < HEIGHT - (HEIGHT%scale); i += h){
        for(int j = 0; j < WIDTH - (WIDTH%scale); j+= w){
            avg = 0;
            count = 0;
            for(int y = i; y < i + h; y++){
                for(int x = j; x < j + w; x++){
                    avg += img[y][x].y;
                    count++;
                }
            }
            avg /= count;
            char c = pix[avg*colordepth/255];
            std::cout << c << c;
        }
        std::cout << '\n';
    }
}
