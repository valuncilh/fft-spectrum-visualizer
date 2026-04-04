#include "graphics.h"
#include <math.h>
#include <stdlib.h>

void set_pixel(uint8_t* pixels, int w, int h, int x, int y, uint8_t r, uint8_t g, uint8_t b) {
    if (x >= 0 && x < w && y >= 0 && y < h) {
        int idx = (y * w + x) * 3;
        pixels[idx] = r;
        pixels[idx+1] = g;
        pixels[idx+2] = b;
    }
}

void draw_line(uint8_t* pixels, int w, int h, int x0, int y0, int x1, int y1, uint8_t r, uint8_t g, uint8_t b) {
    int dx = abs(x1 - x0), sx = x0 < x1 ? 1 : -1;
    int dy = -abs(y1 - y0), sy = y0 < y1 ? 1 : -1;
    int err = dx + dy, e2;
    while (1) {
        set_pixel(pixels, w, h, x0, y0, r, g, b);
        if (x0 == x1 && y0 == y1) break;
        e2 = 2 * err;
        if (e2 >= dy) { err += dy; x0 += sx; }
        if (e2 <= dx) { err += dx; y0 += sy; }
    }
}


void draw_wave(uint8_t* pixels, int w, int h, int x1, int y1, int x2, int y2, double A, double freq, double phase, uint8_t R, uint8_t G, uint8_t B) {
    int steps = abs(x2 - x1);
    for (int i = 0; i <= steps; i++) {
        int x = x1 + i;
        double t = (double)i / steps;
        double dy = A * sin(2 * M_PI * freq * t + phase);
        int y = y1 + (int)dy;
        set_pixel(pixels, w, h, x, y, R, G, B);
    }
}
