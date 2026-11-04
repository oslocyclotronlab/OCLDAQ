//
// Created by Vetle Wegner Ingeberg on 06/04/2022.
//

#include "Unpacker.h"

#include <memory>
#include <utility>
#include <thread>

#include <logfault/logfault.h>


using namespace Task;

namespace {

    enum class HeaderCheck { valid, invalid, incomplete };

    // Check that the words at pos look like a genuine XIA list-mode header.
    // A random data word (or a stray 0xDEADBEEF frame magic) will almost never
    // satisfy all of these at once.
    HeaderCheck check_header(const uint32_t* pos, const uint32_t* end, const ConfigManager& config)
    {
        if ( end - pos < 4 )
            return HeaderCheck::incomplete;
        const auto* header = reinterpret_cast<const XIA_base_t *>(pos);
        switch ( header->headerLen ) {
            case 4: case 6: case 8: case 10: case 12: case 14: case 16: case 18:
                break;
            default:
                return HeaderCheck::invalid;
        }
        if ( header->eventLen != header->headerLen + header->traceLen / 2 )
            return HeaderCheck::invalid;
        if ( !config.valid_address(header->index()) )
            return HeaderCheck::invalid;
        return HeaderCheck::valid;
    }

    // A header is "clean" if it is valid and the event that follows it is
    // also valid (or runs past the end of the data we currently have).
    bool is_clean_header(const uint32_t* pos, const uint32_t* end, const ConfigManager& config)
    {
        if ( check_header(pos, end, config) != HeaderCheck::valid )
            return false;
        const auto* next = pos + reinterpret_cast<const XIA_base_t *>(pos)->eventLen;
        return next >= end || check_header(next, end, config) != HeaderCheck::invalid;
    }

    const uint32_t* find_clean_header(const uint32_t* pos, const uint32_t* end, const ConfigManager& config)
    {
        for ( ; end - pos >= 4 ; ++pos ) {
            if ( is_clean_header(pos, end, config) )
                return pos;
        }
        return end;
    }
}

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
    size_t resyncs = 0;
    while ( !done ){
        if ( input_queue.wait_and_pop(raw, std::chrono::milliseconds(100)) ) {
            data.insert(data.end(), overflow.begin(), overflow.end());
            overflow.clear();
            data.insert(data.end(), raw.begin(), raw.end());
            const auto* begin = data.data();
            const auto* end = data.data() + data.size();
            const auto* pos = begin;
            while ( pos < end ) {
                const auto status = check_header(pos, end, config);
                if ( status == HeaderCheck::incomplete ) {
                    overflow.insert(overflow.end(), pos, end);
                    break;
                }
                if ( status == HeaderCheck::invalid ) {
                    // We are not at an event boundary. This happens when we
                    // join the stream mid-buffer, when a buffer was lost or
                    // when a buffer was corrupted in transit. Skip forward to
                    // the next clean header.
                    const auto* next = find_clean_header(pos + 1, end, config);
                    ++resyncs;
                    LFLOG_WARN << "Unpacker: lost event alignment at word " << (pos - begin)
                               << " of " << data.size() << ", skipped " << (next - pos)
                               << " words to next clean header (resync #" << resyncs << ")";
                    if ( end - next < 4 ) {
                        // Keep the last few words, they may be the start of
                        // a header that continues in the next buffer.
                        overflow.insert(overflow.end(), next, end);
                        break;
                    }
                    pos = next;
                    continue;
                }
                const auto *header = reinterpret_cast<const XIA_base_t *>(pos);
                if ( pos + header->eventLen > end ) {
                    overflow.insert(overflow.end(), pos, end);
                    break;
                }
                if ( config.keep(header) ) {
                    output_queue.push(config(header));
                }
                pos += header->eventLen;
            }
            data.clear();
        }
    }
    output_queue.mark_as_finish();
}
