import serial
import threading
import time
import argparse
import matplotlib.pyplot as plt
from matplotlib.animation import FuncAnimation
from collections import deque
import numpy as np
import struct
import sys

# --- Configurações ---
MAX_POINTS = 500

# Estrutura para armazenar os dados dos 3 eixos
data_queues = {
    'ts': deque(maxlen = MAX_POINTS),
    'x':  deque(maxlen = MAX_POINTS),
    'y':  deque(maxlen = MAX_POINTS),
    'z':  deque(maxlen = MAX_POINTS)
}

# Controle de estatísticas
stats = {
    'count': 0, 'last_time': time.time(), 'rate': 0.0,
    'lost_packets': 0, 'total_expected': 0, 'loss_percent': 0.0,
    'unique_count': 0, 'last_ts': 0, 'sensor_rate': 0.0
}

# Variáveis globais para rastrear repetições
last_x = last_y = last_z = None

def serial_read_thread(port, baud):
    global last_x, last_y, last_z
    last_seq = None
    
    try:
        ser = serial.Serial(port, baud, timeout = 0.1)
        print(f"Conectado a {port} a {baud} bps.")
        
        # Formato: 2s (header $A não entra), I (uint32 sequence), q (int64), e seis 'i' (int32)
        PACKET_FORMAT = '<Iqiiiiii'
        PACKET_SIZE = struct.calcsize(PACKET_FORMAT) # 36 bytes restantes
        
        while True:
            ser.read_until(b'$A')

            # lê o restante do pacote (36 bytes)
            data = ser.read(PACKET_SIZE)

            if len(data) == PACKET_SIZE:
                seq, ts, x1, x2, y1, y2, z1, z2 = struct.unpack(PACKET_FORMAT, data)
                
                # --- Detecção de Perda de Dados ---
                if last_seq is not None:
                    esperado = last_seq + 1
                    if seq > esperado:
                        stats['lost_packets'] += (seq - esperado)

                last_seq = seq
                stats['total_expected'] += 1

                # Reconstrói a precisão respeitando o sinal do val2
                x = x1 + (x2 / 1000000.0) if x1 >= 0 else x1 - (abs(x2) / 1000000.0)
                y = y1 + (y2 / 1000000.0) if y1 >= 0 else y1 - (abs(y2) / 1000000.0)
                z = z1 + (z2 / 1000000.0) if z1 >= 0 else z1 - (abs(z2) / 1000000.0)
                        
                # Adiciona às filas
                data_queues['ts'].append(ts)
                data_queues['x'].append(x)
                data_queues['y'].append(y)
                data_queues['z'].append(z)
                        
                # --- Cálculo de tempo da UART (Computador) ---
                stats['count'] += 1
                now = time.time()
                if now - stats['last_time'] >= 1.0:
                    stats['rate'] = stats['count'] / (now - stats['last_time'])
                    
                    total = stats['total_expected'] + stats['lost_packets']
                    if total > 0:
                        stats['loss_percent'] = (stats['lost_packets'] / total) * 100.0
                    
                    stats['count'] = 0
                    stats['last_time'] = now

                # --- Taxa Real de Amostras Novas do Sensor ---
                if (x != last_x) or (y != last_y) or (z != last_z):
                    stats['unique_count'] += 1
                    last_x, last_y, last_z = x, y, z

                if stats['last_ts'] == 0:
                    stats['last_ts'] = ts

                delta_ms = ts - stats['last_ts']
                if delta_ms >= 1000: # Baseado no relógio da placa
                    stats['sensor_rate'] = stats['unique_count'] / (delta_ms / 1000.0)
                    stats['unique_count'] = 0
                    stats['last_ts'] = ts

    except Exception as e:
        print(f"Erro na porta serial: {e}")
        sys.exit()

# --- Função Auxiliar para calcular a FFT ---
def compute_fft(data_arr, dt):
    # Remove componente DC (Média)
    centered = data_arr - np.mean(data_arr)
    # Janela de Hanning reduz leakage
    window = np.hanning(len(centered))
    windowed = centered * window
    
    fft_vals = np.fft.rfft(windowed)
    fft_freqs = np.fft.rfftfreq(len(windowed), d = dt)
    
    # Magnitude normalizada (para ter significado físico de amplitude)
    fft_mag = np.abs(fft_vals) / len(windowed) 
    
    return fft_freqs, fft_mag

# --- Configuração do Gráfico ---
fig, (ax, ax_fft) = plt.subplots(
    2, 1,
    figsize=(10, 8),
    gridspec_kw={'height_ratios': [2, 1]}
)
ax.set_title("Aquisição de Dados Acelerômetro (X, Y, Z)")
ax.set_xlabel("Tempo [ms]")
ax.set_ylabel("Aceleração [g]")
ax.grid(True)

# Linhas para os 3 eixos no tempo
line_x, = ax.plot([], [], lw = 1, label = 'X', color = 'red',   alpha = 0.7)
line_y, = ax.plot([], [], lw = 1, label = 'Y', color = 'green', alpha = 0.7)
line_z, = ax.plot([], [], lw = 1, label = 'Z', color = 'blue',  alpha = 0.7)
ax.legend(loc = 'upper right')

ax_fft.set_title("Análise Espectral (FFT)")
ax_fft.set_xlabel("Frequência [Hz]")
ax_fft.set_ylabel("Magnitude")
ax_fft.grid(True)

# Linhas para os 3 eixos na frequência
line_fft_x, = ax_fft.plot([], [], color = 'red',   lw = 1, label = 'FFT X', alpha=0.8)
line_fft_y, = ax_fft.plot([], [], color = 'green', lw = 1, label = 'FFT Y', alpha=0.8)
line_fft_z, = ax_fft.plot([], [], color = 'blue',  lw = 1, label = 'FFT Z', alpha=0.8)
ax_fft.legend(loc='upper right')

# Caixa de texto das taxas fixada no topo esquerdo
text_rate = ax.text(0.02, 0.96, '', transform = ax.transAxes, verticalalignment = 'top', 
                    bbox = dict(boxstyle='round', facecolor='white', alpha = 0.8))

def update_plot(frame):
    if len(data_queues['ts']) == 0:
        return line_x, line_y, line_z, line_fft_x, line_fft_y, line_fft_z, text_rate

    ts_arr = np.array(data_queues['ts'])
    x_arr  = np.array(data_queues['x'])
    y_arr  = np.array(data_queues['y'])
    z_arr  = np.array(data_queues['z'])
    
    # FFT dos 3 Eixos
    if len(x_arr) > 32:
        dt = np.mean(np.diff(ts_arr)) / 1000.0  # ms -> s

        if dt > 0:
            fs = 1.0 / dt

            # Calcula a FFT modularizada para os 3 eixos
            freqs_x, mag_x = compute_fft(x_arr, dt)
            freqs_y, mag_y = compute_fft(y_arr, dt)
            freqs_z, mag_z = compute_fft(z_arr, dt)

            # Atualiza os dados no gráfico de FFT
            line_fft_x.set_data(freqs_x, mag_x)
            line_fft_y.set_data(freqs_y, mag_y)
            line_fft_z.set_data(freqs_z, mag_z)

            # Ajuste dinâmico dos limites de Frequência e Magnitude
            ax_fft.set_xlim(0, fs / 2)
            max_mag = max(np.max(mag_x), np.max(mag_y), np.max(mag_z))
            ax_fft.set_ylim(0, max_mag * 1.1 if max_mag > 0 else 0.1)

    # Atualiza os dados no gráfico de Tempo
    line_x.set_data(ts_arr, x_arr)
    line_y.set_data(ts_arr, y_arr)
    line_z.set_data(ts_arr, z_arr)
    
    # Ajuste dinâmico dos limites de Tempo
    ax.set_xlim(ts_arr[0], ts_arr[-1])
    min_val = min(x_arr.min(), y_arr.min(), z_arr.min())
    max_val = max(x_arr.max(), y_arr.max(), z_arr.max())
    ax.set_ylim(min_val - 1, max_val + 1)
    
    text_rate.set_text(
        f"Taxa UART: {stats['rate']:.2f} Hz\n"
        f"Taxa Sensor: {stats['sensor_rate']:.2f} Hz\n"
        f"Pacotes Perdidos: {stats['lost_packets']}\n"
        f"Taxa de Perda: {stats['loss_percent']:.2f}%"
    )
    
    return line_x, line_y, line_z, line_fft_x, line_fft_y, line_fft_z, text_rate

def main():
    parser = argparse.ArgumentParser()
    parser.add_argument('-p', '--port', default = '/dev/ttyACM0')
    parser.add_argument('-b', '--baud', default = 1000000, type = int)
    args = parser.parse_args()

    thread = threading.Thread(target = serial_read_thread, args = (args.port, args.baud), daemon = True)
    thread.start()

    ani = FuncAnimation(fig, update_plot, interval = 50, blit = False, cache_frame_data = False)
    plt.tight_layout()
    plt.show()

if __name__ == "__main__":
    main()