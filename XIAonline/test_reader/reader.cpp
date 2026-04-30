//
// Created by Vetle Wegner Ingeberg on 29/04/2026.
//

#include <optional>
#include <thread>
#include <structopt/app.hpp>
#include <sys/time.h>

#include "MemoryMap.h"
#include "engine_shm.h"
#include "Format/xiaformat.h"

struct Options_t {
    std::optional<std::string> input_file;
};

STRUCTOPT(Options_t, input_file);

void start_reader(const char* fname) {
    IO::MemoryMap mem_map = IO::MemoryMap(fname);

    unsigned int *engine_shm = engine_shm_attach(true);
    if( !engine_shm ) {
        std::cerr << "Failed to attach engine shm." << std::endl;
        exit(EXIT_FAILURE);
    }
    unsigned int* time_us       = &engine_shm[ENGINE_TIME_US];
    unsigned int* time_s        = &engine_shm[ENGINE_TIME_S ];
    unsigned int* data          = engine_shm + engine_shm[ENGINE_DATA_START];
    unsigned int* first_header  = &engine_shm[ENGINE_FIRST_HEADER];
    const unsigned int datalen  = engine_shm[ENGINE_DATA_SIZE];
    unsigned int datalen_char = datalen*sizeof(int);

    const auto *begin = mem_map.GetPtr<uint32_t>();
    const auto *end = begin + mem_map.GetSize<uint32_t>();
    const auto *pos = begin;
    std::string tmp;
    std::cout << "Press enter to begin";
    std::cin >> tmp;

    if ( tmp == "c" ) {
        *time_us = *time_s = 0;
        bzero(data, datalen);
        return;
    }

    int read_no = 0;
    while ( pos <= end ) {
        int write_pos = 0;
        while ( write_pos < datalen && pos <= end ) {
            const XIA_base_t* header = reinterpret_cast<const XIA_base_t *>(pos);
            for ( int i = 0 ; i < header->eventLen && write_pos < datalen ; ++i)
            if ( write_pos + header->eventLen < datalen ) {
                for ( int i = 0; i < header->eventLen; ++i ) {
                    data[write_pos++] = *pos;
                    ++pos;
                }
            } else {

            }

        }



        if ( pos + datalen <= end ) {
            memcpy(reinterpret_cast<void*>(data), pos, datalen_char);
        } else {
            memcpy(reinterpret_cast<void*>(data), pos, (end-pos)*sizeof(uint32_t));
        }
        *time_us = *time_s = 0;
        // write actual timestamp
        timeval t{};
        gettimeofday(&t, 0);
        *time_us = t.tv_usec;
        *time_s  = t.tv_sec;
        pos += datalen;
        if ( read_no++ % 10000 ) {
            std::cout << "\r" << std::flush;
            std::cout << "Progress: " << 100*(1 - double(end - pos)/double(end - begin)) << "%" << std::flush;
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
    }

    engine_shm_detach();
}

int main(int argc, char **argv) {
    try {
        auto options = structopt::app("reader").parse<Options_t>(argc, argv);
        start_reader(options.input_file->c_str());
    } catch (structopt::exception& e) {
        std::cout << e.what() << "\n";
        std::cout << e.help() << std::endl;
        exit(-1);
    }
}