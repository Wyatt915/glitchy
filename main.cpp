#include <algorithm>
#include <cmath>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

#include "imgutils.hpp"
#include "canny.hpp"

//-----------------------------------------[Pixel Sorting]------------------------------------------

void pixelsort(image& img, const image& edge){
    int offset = 0;
    int count = 0;
    int prevpx = 0; //Default to the previous pixel not being an edge
    for(int i = 0; i < HEIGHT; i++){
        for(int j = 0; j < WIDTH; j++){
            //if we change from black to white or vice versa
            if(edge[i][j].y != prevpx || j == WIDTH - 1){
                count++;
            }
            if(count %2){
                std::sort(begin(img[i]) + offset, begin(img[i]) + j,
                    [](const pixel& a, const pixel& b){
                        return a.y > b.y;
                    }
                );
                offset = j;
                //prevpx = edge[i][j].y;
            }
        }
        offset = 0;
        prevpx = 0;
    }
}


int main(int argc, char* argv[]){
    image img = readppm(argv[1]);
    //image blur = readppm(argv[2]);
    //pixelsort(img, canny(blur));
    printppm(canny(img));
    return 0;
}
