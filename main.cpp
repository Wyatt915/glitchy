#include <algorithm>
#include <cmath>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

#define MAX(X,Y) ((X)>(Y)?(X):(Y))
#define MIN(X,Y) ((X)<(Y)?(X):(Y))

#define PI 3.14159265358

int WIDTH, HEIGHT;

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
            y = (0.299 * r) + (0.587 * g) + (0.114 * b);
        }
    pixel(double luma): y(luma), gray(true) {}
    pixel():r(0), g(0), b(0), y(0), gray(true) {}
};

typedef std::vector<std::vector<double> > matrix;
typedef std::vector<std::vector<pixel> > image;

//----------------------------------------[Helper Functions]----------------------------------------

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

void getline_skip_comments(std::ifstream& fs, std::string& l, char c){
}

image readpgm(std::string fname){
    std::ifstream infile;
    infile.open(fname);
    if (!infile){
        std::cerr << "Unable to open file.\n";
        exit(1);
    }
    std::string line;
    char c = '#';
    //do{
        std::getline(infile, line);
    //} while(line[0] != c);
    std::string magicnumber = line;
    //do{
        std::getline(infile, line);
    //} while(line[0] != c);
    std::stringstream linestream(line);
    linestream >> WIDTH >> HEIGHT;
    //do{
        std::getline(infile, line);
    //} while(line[0] != c);
    double max = std::stod(line);
    image out; 
    std::string lines = "";
    while(std::getline(infile, line)){
        if(line[0] != c) lines += line + " ";
    }
    linestream = std::stringstream(lines);
    std::vector<pixel> temp;
    double r, g, b, y;
    std::cerr << "are we here?";
    //Grayscale image
    if(magicnumber == "P2"){
        while(linestream >> y){
            temp.push_back(pixel(y/max));
            if(temp.size() == WIDTH){
                out.push_back(temp);
                temp.clear();
            }
        }
    }
    //Color Image
    else if(magicnumber == "P3"){
        while(linestream >> r, g, b){
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

void printpgm(const image& in){
    std::cout << "P3\n" << WIDTH << ' ' << HEIGHT << "\n255\n";
    for(std::vector<pixel> i : in){
        for(pixel pix : i){
            std::cout << pix.r * 255 << ' ' << pix.g * 255 << ' ' << pix.b * 255 << ' ';
        }
        std::cout << std::endl;
    }
}

void to_ascii(const image& img){
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

//------------------------------------------[Convolution]-------------------------------------------

image convolution(const image& img, const matrix& kernel, double coef = 1.0){
    image out;
    std::vector<pixel> temp;
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
            temp.push_back(pixel(acc));
        }
        out.push_back(temp);
        temp.clear();
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
    image out;
    out.resize(HEIGHT);
    for(int i = 0; i < HEIGHT; i++){
        out[i].resize(WIDTH);
    }
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

//--------------------------------------[Threshold Functions]---------------------------------------

//double threshold
image threshold(const image& img, int weak, int strong, std::vector<coord>& stronglist){
    image out;
    out.resize(HEIGHT);
    for(int i = 0; i < HEIGHT; i++){
        out[i].resize(WIDTH);
    }
    for(int i = 0; i < HEIGHT; i++){
        for(int j = 0; j < WIDTH; j++){
            //Strong pixels have a value of 255,
            //candidates are 128, and weak pixels are 0.
            if(img[i][j].y >= strong){
                out[i][j] = pixel(1.0);
                stronglist.push_back(coord(i, j));
            }
            else if(img[i][j].y >= weak) out[i][j] = pixel(0.5);
            else out[i][j] = pixel(0);
        }
    }
    return out;
}

//simple threshold
void threshold(image& img, int val){
    for(int i = 0; i < HEIGHT; i++){
        for(int j = 0; j < WIDTH; j++){
            if(img[i][j].y >= val) img[i][j] = pixel(1.0);
            else img[i][j] = pixel(0);
        }
    }
}

//calculate some usable values for the double threashold pass
void threshold_values(const image& img, int& weak, int& strong){
    double average = 0;
    for(int i = 0; i < HEIGHT; i++){
        for(int j = 0; j < WIDTH; j++){
            average += img[i][j].y;
        }
    }
    average /= WIDTH * HEIGHT;
    double weak_avg = 0;
    int weak_count = 0;
    double strong_avg = 0;
    int strong_count = 0;
    for(int i = 0; i < HEIGHT; i++){
        for(int j = 0; j < WIDTH; j++){
            if(img[i][j].y < average){
                weak_avg += img[i][j].y;
                weak_count++;
            }
            else{
                strong_avg += img[i][j].y;
                strong_count++;
            }
        }
    }
    weak = (average + weak_avg/weak_count)/2;
    strong = strong_avg/strong_count;
}

//-----------------------------------------[Edge Thinning]------------------------------------------

//Non-maximum suppression
image nmsuppression(const matrix& mang, const image& mmag){
    image out;
    out.resize(HEIGHT);
    for(int i = 0; i < HEIGHT; i++){
        out[i].resize(WIDTH);
    }
    double testa, testb;
    for(int i = 0; i < HEIGHT; i++){
        for(int j = 0; j < WIDTH; j++){
            //North/South edge
            if(mang[i][j] == 0){
                testa = j > 0 ? mmag[i][j - 1].y : 0;
                testb = j < WIDTH - 1 ? mmag[i][j + 1].y : 0;
            }
            //NW/SE edge
            else if(mang[i][j] == 45){
                testa = i > 0 && j > 0 ? mmag[i-1][j-1].y : 0;
                testb = i < HEIGHT - 1 && j < WIDTH - 1 ? mmag[i+1][j+1].y : 0;
            }
            //East/West edge
            else if(mang[i][j] == 90){
                testa = i > 0 ? mmag[i-1][j].y : 0;
                testb = i < HEIGHT - 1 ? mmag[i+1][j].y : 0;
            }
            //NE/SW edge
            else if(mang[i][j] == 135){
                testa = i > 0 && j < WIDTH - 1 ? mmag[i-1][j+1].y : 0;
                testb = i < HEIGHT - 1 && j > 0 ? mmag[i+1][j-1].y : 0;
            }
            else std::cerr << mang[i][j] << "\n";


            if(mmag[i][j].y > testa && mmag[i][j].y > testb) out[i][j] = mmag[i][j];
            else out[i][j] = pixel(0);
        }
    }
    return out;
} 

//-------------------------------------------[Hysteresis]-------------------------------------------

int stdepth = 0;
void chain(image& img, int r, int c, std::vector<std::vector<bool> >& visited){
    if(visited[r][c]) return; //already been here, don't bother
    stdepth++;
    std::cerr << stdepth << '\n';
    visited[r][c] = true;
    img[r][c] = pixel(1.0); //we can only get here from a strong pixel, so we can make this one strong.
    //look at the 3x3 grid around [r][c]
    for(int i = r - 1; i <= r + 1; i++){
        for(int j = c - 1; j <=c + 1; j++){
            //if the pixel is out of bounds, ignore it.
            if(i < 0 || j < 0 || i >= HEIGHT || j >= WIDTH) continue;
            //if the next pixel is strong, or is a candidate, add it to the chain.
            if(!visited[i][j] && img[i][j].y > 0.5) {chain(img, i, j, visited); stdepth--;}
        }
    }
    return;
}

void hysteresis(image& img, std::vector<coord> slist){
    std::vector<std::vector<bool> > visited;
    for(int i = 0; i < HEIGHT; i++){
        visited.push_back(std::vector<bool>(WIDTH, false));
    }
    
    for(int i = 0; i < slist.size(); i++){
        //start a chain IFF the pixel is strong.
        if(img[slist[i].row][slist[i].col].y == 1.0) chain(img, slist[i].row, slist[i].col, visited);
    }
    std::cerr << "Error here.\n";
    //remove any unconnected weak edges
    threshold(img, 1.0);
}

//Canny Edge Detector
image canny(const image& img){
    matrix x = {{-1, 0, 1},
                {-2, 0, 2},
                {-1, 0, 1}};
    matrix y = {{-1,-2,-1},
                {0, 0, 0},
                {1, 2, 1}};
    std::cerr << "smoothing...";
    image smooth = gaussian(img);
    std::cerr << "x-derivative...";
    image xedge = convolution(smooth, x);
    std::cerr << "y-derivative...";
    image yedge = convolution(smooth, y);
    std::cerr << "magnitude...";
    image mag = magnitude(xedge, yedge);
    std::cerr << "angle...";
    matrix ang = angle(xedge, yedge);
    int weak, strong;
    std::vector<coord> stronglist;
    std::cerr << "thresholding...";
    threshold_values(mag, weak, strong);
    image out = threshold(nmsuppression(ang, mag), weak, strong, stronglist);
    std::cerr << "Hysteresis...";
    hysteresis(out, stronglist);
    return out;
}

//-----------------------------------------[Pixel Sorting]------------------------------------------

void pixelsort(image& img){
    image edge = canny(img);
    int offset = 0;
    int prevpx = 0; //Default to the previous pixel not being an edge
    for(int i = 0; i < HEIGHT; i++){
        for(int j = 0; j < WIDTH; j++){
            //if we change from black to white or vice versa
            if(edge[i][j].y != prevpx || j == WIDTH - 1){
                std::sort(begin(img[i]) + offset, begin(img[i]) + j,
                    [](const pixel& a, const pixel& b){
                        return a.y > b.y;
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


int main(int argc, char* argv[]){
    image img = readpgm(argv[1]);
    pixelsort(img);
    printpgm(img);
    return 0;
}
