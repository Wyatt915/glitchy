#include <algorithm>
#include <string>
#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>

#define MAX(X,Y) ((X)>(Y)?(X):(Y))
#define MIN(X,Y) ((X)<(Y)?(X):(Y))

int WIDTH, HEIGHT;
typedef std::vector<std::vector<int> > matrix;

void clamp(int& val, int min, int max){
    val = MAX(val, min);
    val = MIN(val, max);
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
    std::cout << "P2\n" << WIDTH << ' ' << HEIGHT << "\n255\n";
    for(std::vector<int> i : in){
        for(int pix : i){
            std::cout << pix << ' ';
        }
        std::cout << std::endl;
    }
}

matrix convolution(const matrix& img, const matrix& kernel){
    matrix out;
    std::vector<int> temp;
    int acc = 0;
    int k_off = (kernel.size() - 1) / 2;
    for(int row = 0; row < HEIGHT; row++){
        for(int col = 0; col < WIDTH; col++){
            acc = 0;
            for(int i = 0; i < kernel.size(); i++){
                for(int j = 0; j < kernel[0].size(); j++){
                    int x = col + j - k_off;
                    int y = row + i - k_off;
                    clamp(x, 0, WIDTH);
                    clamp(y, 0, HEIGHT);
                    acc += kernel[i][j] * img[x][y];
                }
            }
            clamp(acc, 0, 255);
            temp.push_back(acc);
        }
        out.push_back(temp);
        temp.clear();
    }
    return out;
}


int main(int argc, char* argv[]){
    matrix img = readppm(argv[1]);
    matrix k = {{-1, -1, -1},
                {-1,  8, -1},
                {-1, -1, -1}};
    printppm(convolution(img, k));
    return 0;
}
