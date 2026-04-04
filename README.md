# FFT Spectrum Visualizer

[![C](https://img.shields.io/badge/language-C-blue.svg)](https://en.wikipedia.org/wiki/C_(programming_language))
[![CMake](https://img.shields.io/badge/build-CMake-green.svg)](https://cmake.org)
[![License](https://img.shields.io/badge/license-MIT-lightgrey.svg)](LICENSE)

Анимированная визуализация спектра сигнала с эффектом растекания. Программа на чистом C читает отсчёты сигнала из текстового файла, вычисляет БПФ и генерирует 100 кадров, где спектр постепенно «вытекает» от нуля до полной амплитуды.

<p align="center">
  <img src="demo.gif" alt="Анимация спектра">
</p>

##  Особенности

- **Загрузка сигнала** – через стандартный диалог выбора `.txt` файла (Windows) или встроенный тестовый сигнал.
- **Быстрое преобразование Фурье** – компактная библиотека KissFFT.
- **Анимация нарастания** – 100 кадров, плавное появление спектра.
- **Сохранение кадров** – в формате PNG (библиотека stb_image_write) в папку `FRAMES`.
- **Автоочистка** – при каждом запуске папка `FRAMES` пересоздаётся.
- **Простая сборка** – CMake, минимум зависимостей.

##  Сборка и запуск

### Требования
- CMake (≥ 3.10)
- Компилятор C (Visual Studio, GCC, Clang)
- (Опционально) ffmpeg для создания GIF/видео

### Инструкция

```bash
git clone https://github.com/ВАШ_ЛОГИН/fft-spectrum-visualizer.git
cd fft-spectrum-visualizer
mkdir build && cd build
cmake ..
cmake --build . --config Release
cd Release
./fft_visualizer.exe          # Windows
# или ./fft_visualizer        # Linux/macOS

🛠️ Используемые библиотеки
KissFFT – быстрое преобразование Фурье (BSD).

stb_image_write – сохранение PNG (общественное достояние).

Графические примитивы (линии, круги, заливка) – собственная реализация.

📜 Лицензия
MIT © [Vladimir]