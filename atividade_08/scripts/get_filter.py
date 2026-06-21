import numpy as np
from scipy.signal import firwin

num_taps = 31
fs       = 400.0 # Hz
cutoff   = 25.0  # Hz

bit_shift = 10

# firwin gera os coeficientes automaticamente usando a janela Hamming por padrão
coeficientes = firwin(num_taps, cutoff, fs = fs, window = 'hamming')

# Imprimindo formatado para colar no código C
print(f"#define NUM_TAPS {num_taps}")
print(f"#define BIT_SHIFT {bit_shift}")
print(f"static const int32_t fir_coeffs[{num_taps}] = " "{")
for i, c in enumerate(coeficientes):
    print(f"    {c:.6f}f * (1 << BIT_SHIFT),", end = "\n" if (i + 1) % 5 == 0 else "")
print("\n};")