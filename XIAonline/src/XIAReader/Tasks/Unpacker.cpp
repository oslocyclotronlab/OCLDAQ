//
// Created by Vetle Wegner Ingeberg on 06/04/2022.
//

#include "Unpacker.h"

#include <memory>
#include <utility>
#include <thread>


using namespace Task;

Unpacker::Unpacker(InputQueue_t &input_queue, ConfigManager configManager)
    : input_queue( input_queue )
    , output_queue( )
    , config(std::move( configManager ))
{
    data.reserve(65536);
    overflow.reserve(65536);
}

void Unpacker::Run()
{
    QueueWorker worker(output_queue);
    std::vector<uint32_t> raw;
    while ( !done ){
        if ( input_queue.wait_and_pop(raw, std::chrono::milliseconds(100)) ) {
            data.insert(data.end(), overflow.begin(), overflow.end());
            overflow.clear();
            data.insert(data.end(), raw.begin(), raw.end());
            auto* begin = data.data();
            auto* end = data.data() + data.size();
            auto* pos = begin;
            while ( pos < end ) {
                const auto *header = reinterpret_cast<const XIA_base_t *>(pos);
                if ( pos + header->eventLen <= end ) {
                    if ( config.keep(header) ) {
                        output_queue.push(config(header));
                    }
                } else {
                    overflow.insert(overflow.end(), pos, end);
                }
                pos += header->eventLen;
            }
            data.clear();
        }
    }
    output_queue.mark_as_finish();
}
