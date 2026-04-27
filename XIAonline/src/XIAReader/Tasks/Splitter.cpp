//
// Created by Vetle Wegner Ingeberg on 16/04/2021.
//

#include "Splitter.h"
#include "timeval.h"

using namespace Task;


Splitter::Splitter(EntryQueue_t &input, const double &time_gap, const size_t &cap)
    : input_queue( input ), output_queue( /*cap*/ ), gap( time_gap ){}

void Splitter::Run()
{
    QueueWorker worker(output_queue);
    Entry_t entry;
    std::vector<Entry_t> entries;
    entries.reserve(128);
    while ( input_queue.is_not_finish() || !input_queue.empty() ) {
        if ( !input_queue.try_pop(entry) ) {
            std::this_thread::yield();
            continue;
        }
        ++entries_processed;
        if (entries.empty()) {
            entries.emplace_back(entry);
            continue;
        }

        if ( (double(entry.timestamp - entries.back().timestamp) + (entry.cfdcorr - entries.back().cfdcorr)) <= gap) {
            entries.emplace_back(entry);
        } else {
            output_queue.push(entries);
            entries.clear();
            entries.emplace_back(entry);
        }
    }
    output_queue.mark_as_finish();
    is_done = true;
}