//
// Created by Vetle Wegner Ingeberg on 14/05/2026.
//

#ifndef FILEHEADER_H
#define FILEHEADER_H

#include <cstdint>
#include <cstring>

namespace IO {
    struct FileHeader_t {
        static constexpr const char MAGIC[] = "XIARAW";
        static constexpr size_t MAGIC_SIZE = 8;
        static constexpr uint32_t VERSION = 1;
        static constexpr uint32_t ENDIAN = 0x12345678;

        char magic[MAGIC_SIZE];
        uint32_t endian;
        uint32_t version;
        uint64_t bytes_written;
    };

    inline void write_header(void* addr) {
        auto* header = reinterpret_cast<FileHeader_t*>(addr);
        memcpy(header->magic, FileHeader_t::MAGIC, FileHeader_t::MAGIC_SIZE);
        header->endian = FileHeader_t::ENDIAN;
        header->version = FileHeader_t::VERSION;
        header->bytes_written = 0;
    }
}

#endif // FILEHEADER_H
