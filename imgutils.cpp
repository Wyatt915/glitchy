#include "imgutils.hpp"
#include <fstream>
#include <iostream>
#include <sstream>

//------------------------------------------[Image Class]-------------------------------------------

image::image(){
    rows = 0;
    cols = 0;
}

image::image(int r, int c):rows(r), cols(c){
    data.resize(rows);
    for(int i = 0; i < rows; i++){
        data[i].resize(c);
    }
}

image::image(std::vector<std::vector<pixel> > v){
    data = v;
    rows = v.size();
    cols = v[0].size();
}

image::image(const image& other){
    rows = other.rows;
    cols = other.cols;
    data = other.data;
}

void image::set_format(std::string f){
    format = f;
}

std::string image::get_format() const {
    return format;
}

int image::r() const {
    return rows;
}

int image::c() const {
    return cols;
}

const std::vector<pixel>& image::operator[](size_t i) const {
    return data[i];
}

std::vector<pixel>& image::operator[](size_t i){
    return data[i];
}

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

//----------------------------------------------[I/O]-----------------------------------------------

void getline_ignore_comments(std::ifstream& in, std::string& l){
    char c = '#';
    do{
        std::getline(in, l);
    } while(l[0] == c);  //loop until <line> is not a comment.
}

image readppm(std::string fname){
    std::ifstream infile;
    infile.open(fname);
    if (!infile){
        std::cerr << "Unable to open file.\n";
        exit(1);
    }
    std::string line;
    
    //Get the magic number
    getline_ignore_comments(infile, line);
    std::string format = line;

    //Get the width and height of the image
    getline_ignore_comments(infile, line);
    std::stringstream linestream(line);
    int width, height;
    linestream >> width >> height;

    //get the max brightness value
    getline_ignore_comments(infile, line);
    double max = std::stod(line);
    
    std::vector<std::vector<pixel> > pixdata; 
    std::string lines = "";
    
    while(std::getline(infile, line)){
        if(line[0] != '#') lines += line + " ";
    }
    
    linestream = std::stringstream(lines);
    std::vector<pixel> temp;
    double r, g, b, y;
    
    //Grayscale image
    if(format == "P2"){
        while(linestream >> y && pixdata.size() < height){
            temp.push_back(pixel(y/max));
            if(temp.size() == width){
                pixdata.push_back(temp);
                temp.clear();
            }
        }
    }
    //Color Image
    else if(format == "P3"){
        while(linestream >> r >> g >> b && pixdata.size() < height){
            temp.push_back(pixel(r/max, g/max, b/max));
            if(temp.size() == width){
                pixdata.push_back(temp);
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
    image img(pixdata);
    img.set_format(format);
    return img;
}

void printppm(const image& img){
    if(img.get_format() == "P2"){
        std::cout << "P2\n" << img.c() << ' ' << img.r() << "\n255\n";
        for(int i = 0; i < img.r(); i++){
            for(pixel pix : img[i]){
                std::cout << int(pix.y * 255) << ' ';
            }
            std::cout << std::endl;
        }
    }
    if(img.get_format() == "P3"){
        std::cout << "P3\n" << img.c() << ' ' << img.r() << "\n255\n";
        for(int i = 0; i < img.r(); i++){
            for(pixel pix : img[i]){
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
    int w = img.c()/scale;
    int h = img.r()/scale;
    int avg, count;
    for(int i = 0; i < img.r() - (img.r()%scale); i += h){
        for(int j = 0; j < img.c() - (img.c()%scale); j+= w){
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
