import struct
import rsb_event_pb2
import numpy as np
import matplotlib.pyplot as plt

# Функция для чтения и парсинга файла
def parse_rsb_file(file_path):
    with open(file_path, "rb") as f:
        header = f.read(30)
        meta_length = struct.unpack(">I", header[14:18])[0]
        data_length = struct.unpack(">Q", header[18:26])[0]
        meta = f.read(meta_length)
        data = f.read(data_length)
        point = rsb_event_pb2.Point()
        point.ParseFromString(data)
    return point

# Функция для извлечения амплитуд (максимальных значений в каждом фрейме)
def extract_amplitudes(point):
    channel_amplitudes = {}
    for ch in point.channels:
        amplitudes = []
        frame_count = 0
        for block in ch.blocks:
            for frame in block.frames:
                arr = np.frombuffer(frame.data, dtype=np.int16)
                max_amp = np.abs(arr).max()
                amplitudes.append(max_amp)
                frame_count += 1
        
        channel_amplitudes[ch.id] = amplitudes
    
    return channel_amplitudes

# Функция для построения гистограммы по каналам
def plot_channel_histograms(channel_amplitudes, bins=100, range=(0, 8192)):
    plt.figure(figsize=(12, 6))
    colors = plt.cm.get_cmap('tab10', len(channel_amplitudes))
    
    # Построение гистограмм для каждого канала
    for i, (channel_id, amplitudes) in enumerate(channel_amplitudes.items()):
        plt.hist(amplitudes, bins=bins, range=range, 
                alpha=0.6, color=colors(i),
                edgecolor='black', linewidth=0.5,
                label=f'Канал {channel_id}')
    
    plt.title('Распределение амплитуд по каналам')
    plt.xlabel('Амплитуда (по модулю)')
    plt.ylabel('Частота')
    plt.legend()
    plt.grid(True, linestyle='--', alpha=0.5)
    plt.show()

if __name__ == "__main__":
    # Парсинг файла
    file_path = "/home/rsb_parser/data/p44(30s)(HV1=16800).df"
    point = parse_rsb_file(file_path)
    
    # Извлечение амплитуд
    channel_amplitudes = extract_amplitudes(point)
    
    # Построение гистограмм
    plot_channel_histograms(channel_amplitudes)