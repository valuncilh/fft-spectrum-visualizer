import math
import random

# Параметры сигнала (можно менять прямо в коде)
DURATION = 1.0          # секунды
SAMPLE_RATE = 1024      # Гц (лучше степень двойки)
FREQUENCIES = [101.3, 253.7]
AMPLITUDES = [1.0, 0.7]
ADD_NOISE = False
NOISE_AMP = 0.05

# Генерация имени файла (signal_1.txt, signal_2.txt ...)
import os
counter = 1
while os.path.exists(f"signal_{counter}.txt"):
    counter += 1
filename = f"signal_{counter}.txt"

# Генерация сигнала
n = int(DURATION * SAMPLE_RATE)
t = [i / SAMPLE_RATE for i in range(n)]
signal = [0.0] * n
for f, A in zip(FREQUENCIES, AMPLITUDES):
    for i, ti in enumerate(t):
        signal[i] += A * math.sin(2 * math.pi * f * ti)
if ADD_NOISE:
    for i in range(n):
        signal[i] += random.gauss(0, NOISE_AMP)

# Сохранение
with open(filename, "w") as f:
    for val in signal:
        f.write(f"{val}\n")

print(f"Saved {n} samples to {filename}")