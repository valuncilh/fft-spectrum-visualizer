#ifndef GRAPHICS_H
#define GRAPHICS_H

#include <stdint.h>

typedef struct { uint8_t r, g, b; } Color;

void set_pixel(uint8_t* pixels, int w, int h, int x, int y, uint8_t r, uint8_t g, uint8_t b);
void draw_line(uint8_t* pixels, int w, int h, int x0, int y0, int x1, int y1, uint8_t r, uint8_t g, uint8_t b);
void draw_wave(uint8_t* pixels, int w, int h, int x1, int y1, int x2, int y2, double A, double freq, double phase, uint8_t R, uint8_t G, uint8_t B);


#endif