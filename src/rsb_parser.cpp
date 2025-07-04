#include <iostream>
#include <fstream>
#include <vector>
#include <iomanip>
#include <cstdint>
#include <cstring>
#include <bit>
#include "byteswap.h"
#include "rsb_event.pb.h"
#include <sstream>


#pragma pack(push, 1)
struct EnvelopeHeader {
    char start[2];
    char version[4];
    uint32_t junk1;
    uint16_t metaFormatKey;
    uint16_t junk2;
    uint32_t metaLength;
    uint64_t dataLength;
    char end[4];
};
#pragma pack(pop)

bool check_signature(const EnvelopeHeader& h) {
    return h.start[0] == '#' && h.start[1] == '!' &&
        h.end[0] == '!' && h.end[1] == '#' &&
        h.end[2] == '\r' && h.end[3] == '\n';
}

// Функция для красивого вывода массива int16_t
std::string format_int16_array(const int16_t* data, size_t n, int per_line = 10, int indent = 0) {
    std::ostringstream oss;
    std::string pad(indent, ' ');
    for (size_t i = 0; i < n; ++i) {
        if (i % per_line == 0) oss << pad;
        oss << data[i];
        if (i + 1 < n) oss << ", ";
        if ((i + 1) % per_line == 0) oss << "\n";
    }
    if (n % per_line != 0) oss << "\n";
    return oss.str();
}

int main(int argc, char* argv[]) {
    if (argc < 2) {
        std::cerr << "Необходимо указать имя файла для разбора!\n";
        std::cerr << "Пример использования: " << argv[0] << " input.df [output.txt]\n";
        return 1;
    }
    std::string input_filename = argv[1];
    std::string output_filename = "output.txt";
    if (argc > 2) {
        output_filename = argv[2];
    }
    std::ofstream out(output_filename);
    if (!out) {
        std::cerr << "Ошибка открытия файла для вывода: " << output_filename << std::endl;
        return 1;
    }

    std::ifstream file(input_filename, std::ios::binary);
    if (!file) {
        std::cerr << "Error opening file: " << input_filename << std::endl;
        return 1;
    }

    EnvelopeHeader header;
    file.read(reinterpret_cast<char*>(&header), sizeof(header));
    if (file.gcount() != sizeof(header)) {
        std::cerr << "Failed to read complete header.\n";
        return 1;
    }

    if (!check_signature(header)) {
        std::cerr << "Invalid envelope signature.\n";
        return 1;
    }
    std::string version_str(header.version, 4);
    // Краткая сводка в консоль
    std::cout << "Версия: " << version_str << std::endl;
    header.junk1 = from_big_endian(header.junk1);
    header.metaFormatKey = from_big_endian(header.metaFormatKey);
    header.junk2 = from_big_endian(header.junk2);
    header.metaLength = from_big_endian(header.metaLength);
    header.dataLength = from_big_endian(header.dataLength);

    std::cout << "metaFormatKey: " << header.metaFormatKey << std::endl;
    std::cout << "metaLength: " << header.metaLength << std::endl;
    std::cout << "dataLength: " << header.dataLength << std::endl;

    // Чтение метаданных
    std::vector<char> meta(header.metaLength);
    file.read(meta.data(), header.metaLength);
    if (file.gcount() != header.metaLength) {
        std::cerr << "Ошибка чтения метаданных.\n";
        return 1;
    }

    // Чтение основного блока данных
    std::vector<char> data(header.dataLength);
    file.read(data.data(), header.dataLength);
    if (file.gcount() != header.dataLength) {
        std::cerr << "Ошибка чтения основного блока данных.\n";
        return 1;
    }

    GOOGLE_PROTOBUF_VERIFY_VERSION;

    Rsh::Point point;
    if (!point.ParseFromArray(data.data(), data.size())) {
        std::cerr << "Ошибка парсинга protobuf-данных!" << std::endl;
        return 1;
    }

    out << "Прочитано " << point.channels_size() << " каналов\n";
    for (int ch_idx = 0; ch_idx < point.channels_size(); ++ch_idx) {
        const auto& channel = point.channels(ch_idx);
        out << "Канал ID: " << channel.id() << "\n";
        for (int blk_idx = 0; blk_idx < channel.blocks_size(); ++blk_idx) {
            const auto& block = channel.blocks(blk_idx);
            out << "  ├─ Блок " << blk_idx << ": время=" << block.time()
                << ", длительность=" << block.length()
                << ", размер бина=" << block.bin_size() << "\n";
            // Сырые события (frames)
            out << "  │   ├─ Сырых событий: " << block.frames_size() << "\n";
            for (int fr_idx = 0; fr_idx < block.frames_size(); ++fr_idx) {
                const auto& frame = block.frames(fr_idx);
                out << "  │   │   ├─ Frame " << fr_idx << ": время=" << frame.time();
                const std::string& data_bytes = frame.data();
                size_t n_values = data_bytes.size() / sizeof(int16_t);
                out << ", размер данных=" << data_bytes.size() << " байт\n";
                if (n_values > 0) {
                    const int16_t* data_ptr = reinterpret_cast<const int16_t*>(data_bytes.data());
                    out << format_int16_array(data_ptr, n_values, 10, 18);
                }
            }
            // Обработанные события (events)
            if (block.has_events()) {
                const auto& events = block.events();
                int n_events = std::min(events.times_size(), events.amplitudes_size());
                out << "  │   └─ Обработанных событий: " << n_events << "\n";
                for (int ev_idx = 0; ev_idx < n_events; ++ev_idx) {
                    out << "  │       ├─ Событие " << ev_idx << ": время=" << events.times(ev_idx)
                        << ", амплитуда=" << events.amplitudes(ev_idx) << "\n";
                }
            }
        }
        out << "------------------------------------------------------------\n";
    }
    out << "\nГотово. Всего каналов: " << point.channels_size() << std::endl;
    std::cout << "Детальный вывод сохранён в " << output_filename << std::endl;
    return 0;
}
