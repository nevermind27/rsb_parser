import struct
import rsb_event_pb2
import numpy as np
import matplotlib.pyplot as plt
import os

with open("/home/dymasus/rsb_parser/data/p44(30s)(HV1=16800).df", "rb") as f:
    # Читаем бинарный заголовок (30 байт)
    header = f.read(30)
    # Распаковываем metaLength (uint32, big endian, смещение 14)
    metaLength = struct.unpack(">I", header[14:18])[0]
    # Распаковываем dataLength (uint64, big endian, смещение 18)
    dataLength = struct.unpack(">Q", header[18:26])[0]
    # Читаем метаданные
    meta = f.read(metaLength)
    # Читаем основной блок данных
    data = f.read(dataLength)
    # Парсим protobuf
    point = rsb_event_pb2.Point()
    point.ParseFromString(data)


# Сохраняем распарсенный объект в текстовом виде
with open("python_out.txt", "w", encoding="utf-8") as out:
    out.write(str(point))

# Словарь: channel_id -> (список номеров фреймов, список амплитуд)
channel_data = {}

for ch in point.channels:
    ch_id = ch.id  # реальный id канала
    frame_numbers = []
    amplitudes = []
    frame_idx = 0
    for block in ch.blocks:
        for frame in block.frames:
            arr = np.frombuffer(frame.data, dtype=np.int16)
            max_amp = np.abs(arr).max()  # максимум по модулю
            frame_numbers.append(frame_idx)
            amplitudes.append(max_amp)
            frame_idx += 1
    channel_data[ch_id] = (frame_numbers, amplitudes)

# Цвета для разных каналов 
colors = ['blue', 'green', 'red', 'orange', 'purple', 'cyan', 'magenta', 'brown', 'gray', 'olive']

plt.figure(figsize=(12, 6))
for idx, (ch_id, (frame_numbers, amplitudes)) in enumerate(channel_data.items()):
    plt.plot(frame_numbers, amplitudes, '.', color=colors[idx % len(colors)], label=f'Канал {ch_id}')

plt.title("Амплитуда фрейма (максимум по модулю) по номеру фрейма")
plt.xlabel("Номер фрейма")
plt.ylabel("Амплитуда (максимум по модулю)")
plt.legend()
plt.grid(True)
plt.show()
