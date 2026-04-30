//
// Created by Vetle Wegner Ingeberg on 16/04/2021.
//

#include "Buffer.h"

using namespace Task;

Buffer::Buffer(EntryQueue_t &input, const size_t &buf_size)
    : input_queue( input )
    , output_queue( )
    , size( buf_size )
{
}

void Buffer::Run()
{
    QueueWorker worker(output_queue);
    Entry_t event;
    while ( input_queue.is_not_finish() || !input_queue.empty() ) {
        if ( !input_queue.try_pop( event )) {
            std::this_thread::yield();
            continue;
        }
        ++entries_processed;
        buffer.push(event);
        if ( buffer.size() > size ){
            output_queue.push(buffer.top());
            buffer.pop();
        }
    }

    while ( !buffer.empty() ) {
        output_queue.push(buffer.top());
        buffer.pop();
    }

    is_done = true;
    output_queue.mark_as_finish();
}