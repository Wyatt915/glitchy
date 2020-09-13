#pragma once

class image;

void dither(image&);
void dither(image&, int);
image sdither(const image&);
void to_ascii(const image &);
void to_braille(image);
void pixelsort(image&, const image&);
void jitter(image&, int);
