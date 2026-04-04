#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <windows.h>
#include <commdlg.h>
#include <direct.h>

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

#include "graphics.h"
#include "fft_processor.h"

#define WIDTH 800
#define HEIGHT 600
#define NUM_FRAMES 100
#define MAX_SIGNAL_SIZE 2048

static char szFileFilter[] = "Text Files (*.txt)\0*.txt\0All Files (*.*)\0*.*\0";

static int open_file_dialog(char* out_path, size_t out_size) {
    OPENFILENAMEA ofn = {0};
    char file_name[MAX_PATH] = "";
    ofn.lStructSize = sizeof(ofn);
    ofn.hwndOwner = NULL;
    ofn.lpstrFilter = szFileFilter;
    ofn.lpstrFile = file_name;
    ofn.nMaxFile = MAX_PATH;
    ofn.Flags = OFN_FILEMUSTEXIST | OFN_HIDEREADONLY;
    if (GetOpenFileNameA(&ofn)) {
        strncpy(out_path, file_name, out_size);
        out_path[out_size - 1] = '\0';
        return 1;
    }
    return 0;
}

static int read_signal_from_file(const char* filename, double* signal, int max_size) {
    FILE* f = fopen(filename, "r");
    if (!f) return 0;
    int count = 0;
    while (count < max_size && fscanf(f, "%lf", &signal[count]) == 1) {
        count++;
    }
    fclose(f);
    return count;
}

static void generate_test_signal(double* signal, int size) {
    double Fs = size;
    double f1 = 101.3, f2 = 253.7;
    double A1 = 1.0, A2 = 0.7;
    for (int i = 0; i < size; i++) {
        double t = i / Fs;
        signal[i] = A1 * sin(2 * M_PI * f1 * t) + A2 * sin(2 * M_PI * f2 * t);
    }
    printf("Using default test signal (f1=%.1f, f2=%.1f), N=%d\n", f1, f2, size);
}

// Удаление папки со всем содержимым (рекурсивно)
static void remove_directory(const char* path) {
    char cmd[512];
    snprintf(cmd, sizeof(cmd), "rmdir /s /q \"%s\"", path);
    system(cmd);
}

// Градиентный фон
static void draw_gradient_background(uint8_t* pixels, int width, int height) {
    for (int y = 0; y < height; y++) {
        float t = (float)y / height;
        uint8_t r = (uint8_t)(30 + 30 * t);
        uint8_t g = (uint8_t)(40 + 80 * t);
        uint8_t b = (uint8_t)(80 + 100 * t);
        for (int x = 0; x < width; x++) {
            int idx = (y * width + x) * 3;
            pixels[idx] = r;
            pixels[idx+1] = g;
            pixels[idx+2] = b;
        }
    }
}

// Цветной спектр (зелёный -> красный) с логарифмическим масштабированием
static void draw_spectrum_colored(uint8_t* pixels, int width, int height,
                                  double* magnitudes, int num_bins,
                                  double max_mag, double factor,
                                  int x_start, int y_base, int bar_width) {
    int max_bar_height = y_base - 40;
    for (int i = 0; i < num_bins; i++) {
        int x = x_start + i * bar_width;
        double amp = magnitudes[i] * factor;
        int bar_height = (int)((amp / max_mag) * max_bar_height);
        if (bar_height < 0) bar_height = 0;
        
        // Логарифмическая шкала: растягивает малые амплитуды
        double intensity_linear = amp / max_mag;
        double intensity = log10(1.0 + 100.0 * intensity_linear) / log10(101.0);
        if (intensity > 1.0) intensity = 1.0;
        if (intensity < 0.0) intensity = 0.0;
        
        uint8_t r = (uint8_t)(255 * intensity);
        uint8_t g = (uint8_t)(255 * (1 - intensity));
        uint8_t b = 0;
        
        for (int y = 0; y < bar_height; y++) {
            int py = y_base - y;
            for (int dx = 0; dx < bar_width; dx++) {
                set_pixel(pixels, width, height, x + dx, py, r, g, b);
            }
        }
        draw_line(pixels, width, height, x, y_base, x, y_base - bar_height, 0, 0, 0);
    }
}

int main() {
    // Очищаем старую папку FRAMES (если есть)
    remove_directory("FRAMES");
    // Создаём новую папку
    if (mkdir("FRAMES") == 0) {
        printf("Created fresh FRAMES directory.\n");
    } else {
        printf("Could not create FRAMES directory.\n");
        return 1;
    }

    double* signal = (double*)malloc(sizeof(double) * MAX_SIGNAL_SIZE);
    int N = 0;

    printf("Opening file selection dialog...\n");
    char filepath[MAX_PATH];
    if (open_file_dialog(filepath, sizeof(filepath))) {
        N = read_signal_from_file(filepath, signal, MAX_SIGNAL_SIZE);
        if (N > 0) {
            printf("Loaded %d samples from file: %s\n", N, filepath);
        } else {
            printf("File is empty or invalid. Using test signal.\n");
        }
    } else {
        printf("No file selected. Using test signal.\n");
    }

    if (N == 0) {
        N = 1024;
        generate_test_signal(signal, N);
    }

    if ((N & (N - 1)) != 0) {
        int newN = 1;
        while (newN < N) newN <<= 1;
        if (newN > MAX_SIGNAL_SIZE) newN = MAX_SIGNAL_SIZE;
        for (int i = N; i < newN; i++) signal[i] = 0.0;
        N = newN;
        printf("Padded to %d samples (power of two).\n", N);
    }

    double* magnitudes = (double*)malloc(sizeof(double) * (N / 2));
    compute_fft(signal, N, magnitudes);

    double max_mag = 0;
    for (int i = 0; i < N / 2; i++) {
        if (magnitudes[i] > max_mag) max_mag = magnitudes[i];
    }
    if (max_mag == 0) max_mag = 1;
    printf("Max magnitude: %f\n", max_mag);

    int x_start = 60;
    int y_base = HEIGHT - 60;
    int bar_width = (WIDTH - x_start - 40) / (N / 2);
    if (bar_width < 1) bar_width = 1;

    for (int frame = 0; frame < NUM_FRAMES; frame++) {
        uint8_t* pixels = (uint8_t*)malloc(WIDTH * HEIGHT * 3);
        draw_gradient_background(pixels, WIDTH, HEIGHT);
        
        double t = (double)frame / (NUM_FRAMES - 1);
        double factor = t;
        
        draw_spectrum_colored(pixels, WIDTH, HEIGHT,
                              magnitudes, N / 2, max_mag, factor,
                              x_start, y_base, bar_width);
        
        char fname[512];
        snprintf(fname, sizeof(fname), "FRAMES/frame_%04d.png", frame);
        if (!stbi_write_png(fname, WIDTH, HEIGHT, 3, pixels, WIDTH * 3)) {
            printf("Error writing file: %s\n", fname);
        }
        free(pixels);
        printf("Frame %d/%d done\n", frame + 1, NUM_FRAMES);
    }

    free(signal);
    free(magnitudes);
    printf("Done! Frames saved in FRAMES folder.\n");

    // Создание GIF (если ffmpeg доступен)
    printf("Creating GIF using ffmpeg...\n");
    int ret = system("ffmpeg -framerate 30 -i FRAMES/frame_%04d.png -vf \"fps=10,scale=800:-1\" -loop 0 output.gif 2>nul");
    if (ret != 0) {
        printf("ffmpeg not found. To create GIF manually, run:\n");
        printf("ffmpeg -framerate 30 -i FRAMES/frame_%%04d.png -vf \"fps=10,scale=800:-1\" -loop 0 output.gif\n");
    } else {
        printf("GIF created: output.gif\n");
    }
    
    return 0;
}