#include "image.hpp"



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
