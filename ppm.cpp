#include "ppm.hpp"
#include "image.hpp"

#include <iostream>
#include <sstream>
#include <fstream>

void getline_ignore_comments(std::istream& in, std::string& l){
    char c = '#';
    do{
        std::getline(in, l);
    } while(l[0] == c);  //loop until <line> is not a comment.
}

image openppm(std::string fname){
    std::filebuf infile;
    image img;
    if (infile.open(fname, std::ios::in)){
        std::istream is(&infile);
        img = readppm(is);
        infile.close();
    }
    else {
        std::cerr << "Unable to open file.\n";
        exit(1);
    }
    return img;
}

image readppm(std::istream& in){
    std::string line;
    //Get the magic number
    getline_ignore_comments(in, line);
    std::string format = line;

    //Get the width and height of the image
    getline_ignore_comments(in, line);
    std::stringstream linestream(line);
    int width, height;
    linestream >> width >> height;

    //get the max brightness value
    getline_ignore_comments(in, line);
    double max = std::stod(line);

    std::vector<std::vector<pixel> > pixdata;
    std::string lines = "";

    while(std::getline(in, line)){
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
    }

    image img(pixdata);
    img.set_format(format);
    return img;
}

int printppm(const image& img){
    if(img.get_format() == "P1"){
        std::cout << "P1\n" << img.c() << ' ' << img.r() << "\n";
        for(int i = 0; i < img.r(); i++){
            for(pixel pix : img[i]){
                std::cout << int(1-pix.y) << ' ';
            }
            std::cout << std::endl;
        }
        return 0;
    }
    if(img.get_format() == "P2"){
        std::cout << "P2\n" << img.c() << ' ' << img.r() << "\n255\n";
        for(int i = 0; i < img.r(); i++){
            for(pixel pix : img[i]){
                std::cout << int(pix.y * 255) << ' ';
            }
            std::cout << std::endl;
        }
        return 0;
    }
    if(img.get_format() == "P3"){
        std::cout << "P3\n" << img.c() << ' ' << img.r() << "\n255\n";
        for(int i = 0; i < img.r(); i++){
            for(pixel pix : img[i]){
                std::cout << int(pix.r * 255) << ' ' << int(pix.g * 255) << ' ' << int(pix.b * 255) << ' ';
            }
            std::cout << std::endl;
        }
        return 0;
    }
    return 1;
}
