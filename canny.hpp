#pragma once

#include "imgutils.hpp"

image canny(const image&);
image threshold(const image&, double, double, std::vector<coord>&);
void threshold_values(const image&, double&, double&);
image nmsuppression(const matrix&, const image&);
void hysteresis(image&, std::vector<coord>);
