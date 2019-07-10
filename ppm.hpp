#pragma once

#include <vector>
#include <string>

class image;
struct pixel;

image readppm(std::istream&);
image openppm(std::string);
int printppm(const image&);
