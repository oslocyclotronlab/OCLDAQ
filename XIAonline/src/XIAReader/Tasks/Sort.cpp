//
// Created by Vetle Wegner Ingeberg on 06/03/2026.
//

#include "Sort.h"


using namespace Task;


Sorter::Sorter(TEventQueue_t &input, const UserConfiguration& config)
    : input_queue( input )
    , configuration( config )
{
}

void Sorter::Run() {
    std::pair<std::vector<Entry_t>, int> entries;
    while ( input_queue.is_not_finish() || !input_queue.empty() ) {
        if ( !input_queue.try_pop(entries) ) {
            std::this_thread::yield();
            continue;
        }
        ++entries_processed;
    }
}