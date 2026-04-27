//
// Created by Vetle Wegner Ingeberg on 24/06/2022.
//

#include "event.h"

#include <algorithm>

Triggered_event::Triggered_event(const Triggered_event &event)
    : entries( event.entries )
    , trigger( event.trigger )
    , type_bounds{ std::pair{invalid, subvector<Entry_t>{nullptr, nullptr}},
                   std::pair{labr, subvector<Entry_t>{nullptr, nullptr}},
                   std::pair{deDet, subvector<Entry_t>{nullptr, nullptr}},
                   std::pair{eDet, subvector<Entry_t>{nullptr, nullptr}},
                   std::pair{eGuard, subvector<Entry_t>{nullptr, nullptr}},
                   std::pair{ppac, subvector<Entry_t>{nullptr, nullptr}},
                   std::pair{rfchan, subvector<Entry_t>{nullptr, nullptr}},
                   std::pair{qint, subvector<Entry_t>{nullptr, nullptr}},
                   std::pair{any, subvector<Entry_t>{nullptr, nullptr}},
                   std::pair{unused, subvector<Entry_t>{nullptr, nullptr}}}
    , de_by_ring{{nullptr, nullptr}, {nullptr, nullptr}, {nullptr, nullptr}, {nullptr, nullptr},
                 {nullptr, nullptr}, {nullptr, nullptr}, {nullptr, nullptr}, {nullptr, nullptr}}
{
    index();
}

Triggered_event::Triggered_event(const std::vector<Entry_t> &_entries)
    : entries( _entries )
    , trigger( {DetectorType::unused, uint16_t(-1), 0, 0, -1, true, -1e9, -1e9, true} )
    , type_bounds{ std::pair{invalid, subvector<Entry_t>{nullptr, nullptr}},
                   std::pair{labr, subvector<Entry_t>{nullptr, nullptr}},
                   std::pair{deDet, subvector<Entry_t>{nullptr, nullptr}},
                   std::pair{eDet, subvector<Entry_t>{nullptr, nullptr}},
                   std::pair{eGuard, subvector<Entry_t>{nullptr, nullptr}},
                   std::pair{ppac, subvector<Entry_t>{nullptr, nullptr}},
                   std::pair{rfchan, subvector<Entry_t>{nullptr, nullptr}},
                   std::pair{qint, subvector<Entry_t>{nullptr, nullptr}},
                   std::pair{any, subvector<Entry_t>{nullptr, nullptr}},
                   std::pair{unused, subvector<Entry_t>{nullptr, nullptr}}}
    , de_by_ring{{nullptr, nullptr}, {nullptr, nullptr}, {nullptr, nullptr}, {nullptr, nullptr},
                 {nullptr, nullptr}, {nullptr, nullptr}, {nullptr, nullptr}, {nullptr, nullptr}}
{
    index();
}

Triggered_event::Triggered_event(const std::vector<Entry_t> &_entries, const Entry_t &_trigger)
    : entries( _entries )
    , trigger( _trigger )
    , type_bounds{ std::pair{invalid, subvector<Entry_t>{nullptr, nullptr}},
                   std::pair{labr, subvector<Entry_t>{nullptr, nullptr}},
                   std::pair{deDet, subvector<Entry_t>{nullptr, nullptr}},
                   std::pair{eDet, subvector<Entry_t>{nullptr, nullptr}},
                   std::pair{eGuard, subvector<Entry_t>{nullptr, nullptr}},
                   std::pair{ppac, subvector<Entry_t>{nullptr, nullptr}},
                   std::pair{rfchan, subvector<Entry_t>{nullptr, nullptr}},
                   std::pair{qint, subvector<Entry_t>{nullptr, nullptr}},
                   std::pair{any, subvector<Entry_t>{nullptr, nullptr}},
                   std::pair{unused, subvector<Entry_t>{nullptr, nullptr}}}
    , de_by_ring{{nullptr, nullptr}, {nullptr, nullptr}, {nullptr, nullptr}, {nullptr, nullptr},
                 {nullptr, nullptr}, {nullptr, nullptr}, {nullptr, nullptr}, {nullptr, nullptr}}
{
    //
    index();
}

Triggered_event::Triggered_event(std::vector<Entry_t> &&_entries, const Entry_t &_trigger)
    : entries( std::move(_entries) )
    , trigger( _trigger )
    , type_bounds{ std::pair{invalid, subvector<Entry_t>{nullptr, nullptr}},
                   std::pair{labr, subvector<Entry_t>{nullptr, nullptr}},
                   std::pair{deDet, subvector<Entry_t>{nullptr, nullptr}},
                   std::pair{eDet, subvector<Entry_t>{nullptr, nullptr}},
                   std::pair{eGuard, subvector<Entry_t>{nullptr, nullptr}},
                   std::pair{ppac, subvector<Entry_t>{nullptr, nullptr}},
                   std::pair{rfchan, subvector<Entry_t>{nullptr, nullptr}},
                   std::pair{qint, subvector<Entry_t>{nullptr, nullptr}},
                   std::pair{any, subvector<Entry_t>{nullptr, nullptr}},
                   std::pair{unused, subvector<Entry_t>{nullptr, nullptr}}}
    , de_by_ring{{nullptr, nullptr}, {nullptr, nullptr}, {nullptr, nullptr}, {nullptr, nullptr},
                 {nullptr, nullptr}, {nullptr, nullptr}, {nullptr, nullptr}, {nullptr, nullptr}}
{
    //
    index();
}


void Triggered_event::index()
{
    // First step, sort the entries
    std::sort(entries.begin(), entries.end(), [](const auto &lhs, const auto &rhs){
        return lhs.type < rhs.type;
    });

    // Now we will sort through the detector types and fill our mapping for fast lookup later.
    for ( auto &type : {DetectorType::labr, DetectorType::deDet, DetectorType::eDet, DetectorType::ppac, DetectorType::rfchan, DetectorType::qint} ){
        auto begin = std::find_if(entries.begin(), entries.end(), [&type](const auto &c){ return c.type == type; });
        auto end = std::find_if_not(begin, entries.end(), [&type](const auto &c){ return c.type == type; });
        std::sort(begin, end, [](const auto &lhs, const auto &rhs){
            return lhs.detectorID < rhs.detectorID;
        });
        //type_bounds[type] = subvector(&(*begin), &(*(end)));
        type_bounds.at(type) = subvector(&(*begin), &(*end));
    }

    // Lastly we will divide the DE by the ring numbers.
    auto de_events = GetDetector(DetectorType::deDet);
    for ( size_t ring = 0 ; ring < NUM_SI_DE_TEL ; ++ring ){
        auto begin = std::find_if(de_events.begin(), de_events.end(), [&ring](const auto &r){
            return ( r.detectorID / NUM_SI_DE_TEL ) == ring;
        });
        auto end = std::find_if(begin, de_events.end(), [&ring](const auto &r){
            return ( r.detectorID / NUM_SI_DE_TEL ) > ring;
        });

        de_by_ring[ring] = {begin, end};
    }

    /*auto begin = std::find_if(entries.begin(), entries.end(), [](const auto &c){
        return c.type == deDet;
    });
    auto end = std::find_if_not(begin, entries.end(), [](const auto &c){
        return c.type == eDet;
    });

    std::sort(begin, (end == entries.end()) ? end : end + 1, [](const auto &lhs, const auto &rhs){
        return lhs.detectorID < rhs.detectorID;
    });

    // Last point is to sort the particle entries by the detector ID to make life easier later on.
    std::sort(type_bounds[deDet].begin(), type_bounds[deDet].end(), [](const auto &lhs, const auto &rhs){
        return lhs.detectorID < rhs.detectorID;
    });
    std::sort(type_bounds[eDet].begin(), type_bounds[eDet].end(), [](const auto &lhs, const auto &rhs){
        return lhs.detectorID < rhs.detectorID;
    });*/
}

subvector<Entry_t> Triggered_event::GetRing(const size_t &ringNo)
{
    return de_by_ring[ringNo];
}

//std::pair<subvector<Entry_t>, subvector<Entry_t>> Triggered_event::GetTrap(const size_t &trapNo) const
std::pair<std::vector<Entry_t>, std::vector<Entry_t>> Triggered_event::GetTrap(const size_t &trapNo) const
{
    auto e_events = GetDetector(DetectorType::eDet);
    auto e_begin = std::find_if(e_events.begin(), e_events.end(),
                              [&trapNo](const Entry_t &evt){ return evt.detectorID == trapNo; });
    auto e_end = std::find_if(e_begin, e_events.end(), [&trapNo](const Entry_t &evt){ return evt.detectorID > trapNo; });

    auto de_events = GetDetector(DetectorType::deDet);
    auto de_begin = std::find_if(de_events.begin(), de_events.end(),
                                 [&trapNo](const Entry_t &evt){return (evt.detectorID / NUM_SI_DE_TEL) == trapNo;});
    auto de_end = std::find_if(de_begin, de_events.end(),
                               [&trapNo](const Entry_t &evt){ return (evt.detectorID / NUM_SI_DE_TEL) > trapNo; });
    //return std::make_pair(subvector(de_begin, de_end), subvector(e_begin, e_end));
    return std::make_pair(std::vector(de_begin, de_end), std::vector(e_begin, e_end));
}