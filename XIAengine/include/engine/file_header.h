#pragma once

#include <cstdint>
#include <cstring>

namespace IO {

// Endianness markers
constexpr uint32_t ENDIANNESS_LITTLE = 0x01;
constexpr uint32_t ENDIANNESS_BIG = 0x02;

struct FileHeader {
    static constexpr const char MAGIC[] = "XIAENG";
    static constexpr size_t MAGIC_SIZE = 8;
    static constexpr uint32_t VERSION = 1;
    static constexpr uint32_t ENDIANNESS = ENDIANNESS_LITTLE;
    
    char magic[MAGIC_SIZE];
    uint32_t endianness;
    uint32_t version;
    uint64_t bytes_written;
    uint32_t sequence;  // File/run sequence number
    
    // Helper to create a valid header
    static FileHeader create() {
        FileHeader hdr;
        memcpy(hdr.magic, MAGIC, MAGIC_SIZE);
        hdr.endianness = ENDIANNESS;
        hdr.version = VERSION;
        hdr.bytes_written = 0;
        hdr.sequence = 0;
        return hdr;
    }
    
    // Check if this is a valid header
    bool isValid() const {
        return magic[0] == 'X' && magic[1] == 'I' && magic[2] == 'A' &&
               magic[3] == 'E' && magic[4] == 'N' && magic[5] == 'G' &&
               magic[6] == '\0' && magic[7] == '\0' &&
               endianness == ENDIANNESS || endianness == ENDIANNESS_BIG;
    }
    
    // Get endianness as a string for debugging
    const char* getEndiannessStr() const {
        return endianness == ENDIANNESS_LITTLE ? "little" : "big";
    }
};

} // namespace IO
