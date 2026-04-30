//
// Created by Vetle Wegner Ingeberg on 16/04/2021.
//

#include "Trigger.h"

#include <stdexcept>

using namespace Task;

Trigger::Trigger(MCEventQueue_t &input, const UserConfiguration &_config)
        : input_queue( input )
        , output_queue( )
        , config( _config )
{}

std::vector<Entry_t>::iterator find_start(const std::vector<Entry_t>::iterator &begin, const std::vector<Entry_t>::iterator &end, const double &ctime)
{
    // Both T are forward iterators, but end should be smaller than begin.
    auto pos = begin - 1;
    while ( pos >= end ){
        if ( abs((double(pos->timestamp - begin->timestamp) + (pos->cfdcorr - begin->cfdcorr))) > ctime ){
            return pos;
        }
        --pos;
    }
    return end;
}

std::vector<Entry_t>::iterator find_end(const std::vector<Entry_t>::iterator &begin, const std::vector<Entry_t>::iterator &end, const double &ctime)
{
    // Both T are forward iterators, but end should be smaller than begin.
    auto pos = begin;
    while ( pos < end ){
        if ( abs((double(pos->timestamp - begin->timestamp) + (pos->cfdcorr - begin->cfdcorr))) > ctime ){
            return pos + 1;
        }
        ++pos;
    }
    return end;
}

template<typename T>
std::vector<typename T::const_iterator> GetTriggers(const T &entries, const DetectorType &trigger)
{
    std::vector<typename T::const_iterator> triggers;
    for ( auto it = entries.begin() ; it != entries.end() ; ++it ){
        if ( it->type == trigger ){
            triggers.push_back(it);
        }
    }
    return triggers;
}

template<typename T>
std::vector<typename T::const_iterator> GetTriggersExclusive(const T &entries, const DetectorType &trigger)
{
    std::vector<typename T::const_iterator> triggers;
    auto end = entries.end();
    auto pos = std::find_if(entries.begin(), end, [&trigger](const auto &e){ return e.type == trigger; });
    if ( pos != end )
        triggers.push_back(pos);
    while ( pos < end ){
        if ( (double(pos->timestamp - triggers.back()->timestamp) + pos->cfdcorr - triggers.back()->cfdcorr) > 500 )
            triggers.push_back(pos);
        pos = std::find_if(pos+1, entries.end(), [&trigger](const auto &e){ return e.type == trigger; });
    }
    return triggers;
}

void Trigger::Run()
{
    QueueWorker worker(output_queue);
    std::vector<Entry_t> input;
    while ( input_queue.is_not_finish() || !input_queue.empty() ) {
        if ( !input_queue.try_pop(input)) {
            std::this_thread::yield();
            continue;
        }
        ++entries_processed;
        if ( config.GetSortType() == SortType::gap && config.GetTrigger() == DetectorType::any ) {
            output_queue.push(std::make_pair(input, -1));
            continue;
        }

        if ( config.GetSortType() == SortType::gap ){
            // Check if there is an entry that satisfies the trigger
            if ( std::find_if(input.begin(), input.end(), [this](const auto& e){ return e.type == config.GetTrigger(); }) == input.end() )
                continue;
            //output_queue.enqueue(std::make_pair(input, -1));
            output_queue.push({input, -1});
            continue;
        }


        auto triggers = ( config.GetSortType() == SortType::timesort ) ? GetTriggers(input, config.GetTrigger()) : GetTriggersExclusive(input, config.GetTrigger());
        for ( auto &trig : triggers ){

            if ( config.GetSortType() == SortType::timesort ){ // If it is a time calibration run, we only care about the timing relative to the "trigger"
                if ( trig->detectorID != 0 )
                    continue;
            }

            auto begin = trig;
            for ( begin = trig ; begin > input.begin() ; --begin ){
                if ( abs( double((begin-1)->timestamp - trig->timestamp) +
                          ((begin-1)->cfdcorr - trig->cfdcorr) ) > config.GetCoincidenceTime() )
                    break;
            }

            auto end = trig + 1;
            for ( end = trig + 1 ; end != input.end() ; ++end ){
                if ( abs( double(end->timestamp - trig->timestamp) +
                          (end->cfdcorr - trig->cfdcorr) ) > config.GetCoincidenceTime() )
                    break;
            }
            output_queue.push({std::vector(begin, end), trig - begin});
        }
    }
    output_queue.mark_as_finish();
    is_done = true;
}