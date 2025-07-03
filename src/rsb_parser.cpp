#include <iostream>
#include <fstream>
#include <vector>
#include <iomanip>
#include <cstdint>
#include <cstring>
#include <bit>
#include "byteswap.h"
#include "rsb_event.pb.h"
/* From documentation http://npm.mipt.ru/dataforge/docs.html#envelope_format
The default envelope format is developed for storage of binary data or transferring data via byte stream. The structure of this format is the following:

Tag. First 20 bytes of file or stream is reserved for envelope properties binary representation:

#~ - two ASCII symbols, beginning of binary string.
4 bytes - properties type field: envelope format type and version. For default format the string DF02 is used, but in principle other envelope types could use the same format.
2 bytes - properties metaType field: metadata encoding type.
4 bytes - properties metaLength field: metadata length in bytes including new lines and other separators.
4 bytes - properties dataLength field: the data length in bytes.
~# - two ASCII symbols, end of binary string.
\r\n - two bytes, new line.
The values are read as binary and transformed into 4-byte unsigned tag codes (Big endian).
*/
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

int main() { 
    // 

    std::ifstream file("/home/dymasus/rsb_parser/p44(30s)(HV1=16800).df", std::ios::binary);
    if (!file) {
        std::cerr << "Error opening file.\n";
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
    std::cout << "Version: " << version_str << "\n";  // на тестовом df файле байты 00 01 40 00, что не соответствует DF02 в ASCII 
    header.junk1 = from_big_endian(header.junk1);
    header.metaFormatKey = from_big_endian(header.metaFormatKey);
    header.junk2 = from_big_endian(header.junk2);
    header.metaLength = from_big_endian(header.metaLength);
    header.dataLength = from_big_endian(header.dataLength);


    std::cout << "metaFormatKey: " << header.metaFormatKey << "\n";
    std::cout << "metaLength: " << header.metaLength << "\n";
    std::cout << "dataLength: " << header.dataLength << "\n";

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

    // Пример вывода
    std::cout << "Прочитано " << point.channels_size() << " каналов" << std::endl;
    for (const auto& channel : point.channels()) {
        std::cout << "Канал ID: " << channel.id() << std::endl;
        for (const auto& block : channel.blocks()) {
            std::cout << "  Block time: " << block.time()
                      << ", length: " << block.length()
                      << ", bin_size: " << block.bin_size()
                      << std::endl;
        }
    }

    return 0;
}
