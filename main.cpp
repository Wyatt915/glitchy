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

//------------------------------------------[Input/Output]------------------------------------------

matrix readpgm(std::string fname){
    std::ifstream infile;
    infile.open(fname);
    if (!infile){
        std::cerr << "Unable to open file.\n";
        exit(1);
    }
    std::string line;
    std::getline(infile, line);
    std::getline(infile, line);
    std::stringstream linestream(line);
    linestream >> WIDTH >> HEIGHT;
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

void printpgm(matrix in){
    clamp(in, 0, 255);
    std::cout << "P2\n" << WIDTH << ' ' << HEIGHT << "\n255\n";
    for(std::vector<int> i : in){
        for(int pix : i){
            std::cout << pix << ' ';
        }
        std::cout << std::endl;
    }
}

void to_ascii(const matrix& img){
    std::cout << '\n';
    std::string pix = " .'`^\",:;Il!i><~+_-?][}{1)(|\\/tfjrxnuvczXYUJCLQ0OZmwqpdbkhao*#MW&8%B@$";
    int colordepth = pix.length();
    std::cout << "colordepth: " << colordepth << '\n';
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
                    avg += img[y][x];
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

//------------------------------------------[Convolution]-------------------------------------------

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
    if(theta < 23) return 0;
    if(theta < 68) return 45;
    if(theta < 113) return 90;
    if(theta < 156) return 135;
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

//--------------------------------------[Threshold Functions]---------------------------------------

//double threshold
matrix threshold(const matrix& img, int weak, int strong){
    matrix out;
    out.resize(HEIGHT);
    for(int i = 0; i < HEIGHT; i++){
        out[i].resize(WIDTH);
    }
    for(int i = 0; i < HEIGHT; i++){
        for(int j = 0; j < WIDTH; j++){
            //Strong pixels have a value of 255,
            //candidates are 128, and weak pixels are 0.
            if(img[i][j] >= strong) out[i][j] = 255;
            else if(img[i][j] >= weak) out[i][j] = 128;
            else out[i][j] = 0;
        }
    }
    return out;
}

//simple threshold
void threshold(matrix& img, int val){
    for(int i = 0; i < HEIGHT; i++){
        for(int j = 0; j < WIDTH; j++){
            if(img[i][j] >= val) img[i][j] = 255;
            else img[i][j] = 0;
        }
    }
}

//calculate some usable values for the double threashold pass
void threshold_values(const matrix& img, int& weak, int& strong){
    int average = 0;
    for(int i = 0; i < HEIGHT; i++){
        for(int j = 0; j < WIDTH; j++){
            average+= img[i][j];
        }
    }
    average /= WIDTH * HEIGHT;
    int weak_avg = 0;
    int weak_count = 0;
    int strong_avg = 0;
    int strong_count = 0;
    for(int i = 0; i < HEIGHT; i++){
        for(int j = 0; j < WIDTH; j++){
            if(img[i][j] < average){
                weak_avg += img[i][j];
                weak_count++;
            }
            else{
                strong_avg += img[i][j];
                strong_count++;
            }
        }
    }
    weak = (average + weak_avg/weak_count)/2;
    strong = strong_avg/strong_count;
}

//-----------------------------------------[Edge Thinning]------------------------------------------

//Non-maximum suppression
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
            if(mang[i][j] == 0){
                testa = j > 0 ? mmag[i][j - 1] : 0;
                testb = j < WIDTH - 1 ? mmag[i][j + 1] : 0;
            }
            //NW/SE edge
            else if(mang[i][j] == 45){
                testa = i > 0 && j > 0 ? mmag[i-1][j-1] : 0;
                testb = i < HEIGHT - 1 && j < WIDTH - 1 ? mmag[i+1][j+1] : 0;
            }
            //East/West edge
            else if(mang[i][j] == 90){
                testa = i > 0 ? mmag[i-1][j] : 0;
                testb = i < HEIGHT - 1 ? mmag[i+1][j] : 0;
            }
            //NE/SW edge
            else if(mang[i][j] == 135){
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

//-------------------------------------------[Hysteresis]-------------------------------------------

void chain(matrix& img, int r, int c, std::vector<std::vector<bool> >& visited){
    if(visited[r][c]) return; //already been here, don't bother
    visited[r][c] = true;
    img[r][c] == 255; //we can only get here from a strong pixel, so we can make this one strong.
    for(int i = r - 1; i <= r + 1; i++){
        for(int j = c - 1; j <=c + 1; j++){
            //if the pixel is out of bounds, ignore it.
            if(i < 0 || j < 0 || i >= HEIGHT || j >= WIDTH) continue;
            //if the next pixel is strong, or is a candidate, add it to the chain.
            if(!visited[i][j] && img[i][j] > 127) chain(img, i, j, visited);
        }
    }
    return;
}

void hysteresis(matrix& img){
    std::vector<std::vector<bool> > visited;
    for(int i = 0; i < HEIGHT; i++){
        visited.push_back(std::vector<bool>(WIDTH, false));
    }
    
    for(int i = 0; i < HEIGHT; i++){
        for(int j = 0; j < WIDTH; j++){
            //start a chain IFF the pixel is strong.
            if(img[i][j] == 255) chain(img, i, j, visited);
        }
    }
    //remove any unconnected weak edges
    threshold(img, 255);
}

matrix canny(const matrix& img){
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
    int weak, strong;
    threshold_values(mag, weak, strong);
    matrix out = threshold(nmsuppression(ang, mag), weak, strong);
    hysteresis(out);
    return out;
}

int main(int argc, char* argv[]){
    matrix img = readpgm(argv[1]);
    printpgm(canny(img));
    return 0;
}
