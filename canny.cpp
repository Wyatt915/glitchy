#include "imgutils.hpp"
#include "canny.hpp"

//Canny Edge Detector
image canny(const image& img){
    matrix x = {{-1.0, 0.0, 1.0},
                {-2.0, 0.0, 2.0},
                {-1.0, 0.0, 1.0}};
    matrix y = {{-1,-2,-1},
                {0, 0, 0},
                {1, 2, 1}};
    image smooth = gaussian(img);
    image xedge = convolution(smooth, x);
    image yedge = convolution(smooth, y); //range: [-4, 4]
    image mag = magnitude(xedge, yedge);  //range: [0, sqrt(32)]
    remap(mag, 0, sqrt(32));
    matrix ang = angle(xedge, yedge);
    double weak, strong;
    std::vector<coord> stronglist;
    threshold_values(mag, weak, strong);
    image suppressed = nmsuppression(ang, mag);
    image out = threshold(suppressed, weak, strong, stronglist);
    hysteresis(out, stronglist);
    return out;
}

//--------------------------------------[Threshold Functions]---------------------------------------

//double threshold
image threshold(const image& img, double weak, double strong, std::vector<coord>& stronglist){
    image out = newimage();
    for(int i = 0; i < HEIGHT; i++){
        for(int j = 0; j < WIDTH; j++){
            //Strong pixels have a value of 1,
            //candidates are 1/2, and weak pixels are 0.
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


//calculate some usable values for the double threashold pass
void threshold_values(const image& img, double& weak, double& strong){
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
    //weak = weak_avg/weak_count;
    strong = strong_avg/strong_count;
    weak = (average + strong) / 2;
}

//-----------------------------------------[Edge Thinning]------------------------------------------

//Non-maximum suppression
image nmsuppression(const matrix& ang, const image& mag){
    image out = newimage();
    double testa, testb;
    for(int i = 0; i < HEIGHT; i++){
        for(int j = 0; j < WIDTH; j++){
            //East/West edge
            if(ang[i][j] == 0){
                testa = i > 0 ? mag[i-1][j].y : 0;
                testb = i < HEIGHT - 1 ? mag[i+1][j].y : 0;
            }
            //NE/SW edge
            else if(ang[i][j] == 45){
                testa = i > 0 && j < WIDTH - 1 ? mag[i-1][j+1].y : 0;
                testb = i < HEIGHT - 1 && j > 0 ? mag[i+1][j-1].y : 0;
            }
            //North/South edge
            else if(ang[i][j] == 90){
                testa = j > 0 ? mag[i][j - 1].y : 0;
                testb = j < WIDTH - 1 ? mag[i][j + 1].y : 0;
            }
            //NW/SE edge
            else if(ang[i][j] == 135){
                testa = i > 0 && j > 0 ? mag[i-1][j-1].y : 0;
                testb = i < HEIGHT - 1 && j < WIDTH - 1 ? mag[i+1][j+1].y : 0;
            }

            if(mag[i][j].y > testa && mag[i][j].y > testb) out[i][j] = mag[i][j];
            else out[i][j] = pixel(0);
        }
    }
    return out;
} 

//-------------------------------------------[Hysteresis]-------------------------------------------

void chain(image& img, int r, int c, bool** visited){
    if(visited[r][c]) return; //already been here, don't bother
    visited[r][c] = true;
    img[r][c] = pixel(1.0); //we can only get here from a strong pixel, so we can make this one strong.
    //look at the 3x3 grid around [r][c]
    for(int i = r - 1; i <= r + 1; i++){
        for(int j = c - 1; j <=c + 1; j++){
            //if the pixel is out of bounds, ignore it.
            if(i < 0 || j < 0 || i >= HEIGHT || j >= WIDTH) continue;
            //if the next pixel is strong, or is a candidate, add it to the chain.
            if(!visited[i][j] && img[i][j].y >= 0.5) chain(img, i, j, visited);
        }
    }
    return;
}

void hysteresis(image& img, std::vector<coord> slist){
    bool** visited = new bool*[HEIGHT];
    for(int i = 0; i < HEIGHT; i++){
       visited[i] = new bool[WIDTH];
        for(int j = 0; j < WIDTH; j++){
            visited[i][j] = false;
        }
    }
    
    for(int i = 0; i < slist.size(); i++){
        //start a chain IFF the pixel is strong.
        if(img[slist[i].row][slist[i].col].y == 1.0) chain(img, slist[i].row, slist[i].col, visited);
    }
    for(int i = 0; i < HEIGHT; i++){
        delete[] visited[i];
    }
    delete[] visited;
    //remove any unconnected weak edges
    threshold(img, 1.0);
}
