#include <algorithm>
#include <string>
#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <cmath>

#define MAX(X,Y) ((X)>(Y)?(X):(Y))
#define MIN(X,Y) ((X)<(Y)?(X):(Y))

#define PI 3.14159265358

int WIDTH, HEIGHT;
typedef std::vector<std::vector<int> > matrix;

void clamp(int& val, int min, int max){
    val = MAX(val, min);
    val = MIN(val, max);
}

void clamp(matrix& m, int min, int max){
    int val;
    for(int i = 0; i < m.size(); i++){
        for(int j = 0; j < m[0].size(); j++){
            val = m[i][j];
            val = MAX(val, min);
            val = MIN(val, max);
            m[i][j] = val;
        }
    }
}

matrix readppm(std::string fname){
    std::ifstream infile;
    infile.open(fname);
    if (!infile){
        std::cerr << "Unable to open file.\n";
        exit(1);
    }
    std::string line;
    std::getline(infile, line);
    std::cerr << line << std::endl;
    std::getline(infile, line);
    std::stringstream linestream(line);
    linestream >> WIDTH >> HEIGHT;
    std::cerr << "WIDTH: " << WIDTH << "\tHEIGHT: " << HEIGHT << std::endl;
    std::getline(infile, line);
    matrix out; 
    std::string lines = "";
    while(std::getline(infile, line)){
        lines += line + " ";
    }
    linestream = std::stringstream(lines);
    std::vector<int> temp;
    int pix;
    while(linestream >> pix){
        temp.push_back(pix);
        if(temp.size() == WIDTH){
            out.push_back(temp);
            temp.clear();
        }
    }
    infile.close();
    return out;
}

void printppm(matrix in){
    clamp(in, 0, 255);
    std::cout << "P2\n" << WIDTH << ' ' << HEIGHT << "\n255\n";
    for(std::vector<int> i : in){
        for(int pix : i){
            std::cout << pix << ' ';
        }
        std::cout << std::endl;
    }
}

matrix convolution(const matrix& img, const matrix& kernel, double coef = 1.0){
    std::cerr << "Coef = " << coef << std::endl;
    matrix out;
    std::vector<int> temp;
    int acc = 0;
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
                    acc += kernel[i][j] * img[r][c];
                }
            }
            acc *= coef;
            temp.push_back(acc);
        }
        out.push_back(temp);
        temp.clear();
    }
    return out;
}

matrix gaussian(const matrix& img){
    matrix kernel = {{2, 4,  5,  4,  2},
                     {4, 9,  12, 9,  4},
                     {5, 12, 15, 12, 5},
                     {4, 9,  12, 9,  4},
                     {2, 4,  5,  4,  2}};
    return convolution(img, kernel, 1.0/159.0);
}

matrix magnitude(const matrix& mx, const matrix& my){
    matrix out;
    out.resize(HEIGHT);
    for(int i = 0; i < HEIGHT; i++){
        out[i].resize(WIDTH);
    }
    for(int i = 0; i < HEIGHT; i++){
        for(int j = 0; j < WIDTH; j++){
            out[i][j] = sqrt((mx[i][j] * mx[i][j]) + (my[i][j] * my[i][j]));
        }
    }
    return out;
}

int roundangle(int theta){
    if(theta >=0 && theta < 23) return 0;
    if(theta >= 23 && theta < 68) return 45;
    if(theta >= 68 && theta < 113) return 90;
    if(theta >= 113 && theta < 156) return 135;
    return 0;
}

matrix angle(const matrix& mx, const matrix& my){
    matrix out;
    out.resize(HEIGHT);
    for(int i = 0; i < HEIGHT; i++){
        out[i].resize(WIDTH);
    }
    for(int i = 0; i < HEIGHT; i++){
        for(int j = 0; j < WIDTH; j++){
            out[i][j] = roundangle(360 * (atan2(my[i][j], mx[i][j]) / (2*PI)));
        }
    }
    return out;
}


matrix nmsuppression(const matrix& mang, const matrix& mmag){
    matrix out;
    out.resize(HEIGHT);
    for(int i = 0; i < HEIGHT; i++){
        out[i].resize(WIDTH);
    }
    int testa, testb;
    for(int i = 0; i < HEIGHT; i++){
        for(int j = 0; j < WIDTH; j++){
            //North/South edge
            if(mang[i][j] == 90){
                testa = j > 0 ? mmag[i][j - 1] : 0;
                testb = j < WIDTH - 1 ? mmag[i][j + 1] : 0;
            }
            //NW/SE edge
            else if(mang[i][j] == 135){
                testa = i > 0 && j > 0 ? mmag[i-1][j-1] : 0;
                testb = i < HEIGHT - 1 && j < WIDTH - 1 ? mmag[i+1][j+1] : 0;
            }
            //East/West edge
            else if(mang[i][j] == 0){
                testa = i > 0 ? mmag[i-1][j] : 0;
                testb = i < HEIGHT - 1 ? mmag[i+1][j] : 0;
            }
            //NE/SW edge
            else if(mang[i][j] == 45){
                testa = i > 0 && j < WIDTH - 1 ? mmag[i-1][j+1] : 0;
                testb = i < HEIGHT - 1 && j > 0 ? mmag[i+1][j-1] : 0;
            }
            else std::cerr << mang[i][j] << "\n";


            if(mmag[i][j] > testa && mmag[i][j] > testb) out[i][j] = mmag[i][j];
            else out[i][j] = 0;
        }
    }
    return out;
} 

int main(int argc, char* argv[]){
    matrix img = readppm(argv[1]);
    matrix k = {{-1, -1, -1},
                {-1,  8, -1},
                {-1, -1, -1}};
    matrix x = {{-1, 0, 1},
                {-2, 0, 2},
                {-1, 0, 1}};
    matrix y = {{-1,-2,-1},
                {0, 0, 0},
                {1, 2, 1}};
    matrix smooth = gaussian(img);
    matrix xedge = convolution(smooth, x);
    matrix yedge = convolution(smooth, y);
    matrix mag = magnitude(xedge, yedge);
    matrix ang = angle(xedge, yedge);
    printppm(nmsuppression(ang, mag));
    return 0;
}
