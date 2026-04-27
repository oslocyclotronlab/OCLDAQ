//
// Created by Vetle Wegner Ingeberg on 24/06/2022.
//

#ifndef XIA2TREE_EVENT_H
#define XIA2TREE_EVENT_H

#include <array>
#include <vector>
#include <algorithm>
#include <map>
#include <stdexcept>
#include <cstddef>

#include "entry.h"
#include "../Configuration/DetectorTypes.h"

struct Entry_t;


template<typename Key, typename Value, std::size_t size>
struct Map {
    std::array<std::pair<Key, Value>, size> data;

    [[nodiscard]] constexpr Value at(const Key &key) const {
        const auto it = std::find_if(std::begin(data), std::end(data),
                                     [&key](const auto &v){ return v.first == key; });
        if ( it != std::end(data) ) {
            return it->second;
        } else {
            throw std::range_error("Key not found");
        }
    }

    [[nodiscard]] constexpr Value &at(const Key &key) {
        const auto it = std::find_if(std::begin(data), std::end(data),
                                     [&key](const auto &v){ return v.first == key; });
        if ( it != std::end(data) ) {
            return it->second;
        } else {
            throw std::range_error("Key not found");
        }
    }

};

template<typename T>
class subvector
{
private:
    T *_begin;
    T *_end;

public:

    inline T *begin(){ return _begin; }
    inline T *end(){ return _end; }

    [[nodiscard]] T *begin() const { return _begin; }
    [[nodiscard]] T *end() const { return _end; }

    [[nodiscard]] inline size_t size() const { return _end - _begin; }

    inline T *operator[](const size_t &idx){ return _begin + idx; }
    inline T *operator[](const size_t &idx) const { return _begin + idx; }

    subvector() : _begin( nullptr ), _end( nullptr ){}
    subvector(T *begin, T *end) : _begin( begin ), _end( end ){}
};

class Triggered_event {
private:
    std::vector<Entry_t> entries;
    Entry_t trigger;
    Map<DetectorType, subvector<Entry_t>, 10> type_bounds;
    subvector<Entry_t> de_by_ring[NUM_SI_DE_DET];

    void index(); // Should run each initialization, unless empty.

public:
    Triggered_event() = default;
    Triggered_event(const Triggered_event &event);
    explicit Triggered_event(const std::vector<Entry_t> &_entries);
    explicit Triggered_event(const std::vector<Entry_t> &_entries, const Entry_t &trigger);
    explicit Triggered_event(std::vector<Entry_t> &&_entries, const Entry_t &trigger);

    [[nodiscard]] inline std::vector<Entry_t> GetEntries() const { return entries; }
    [[nodiscard]] inline subvector<Entry_t> GetDetector(const DetectorType &type) const { return type_bounds.at(type); }
    [[nodiscard]] inline const Entry_t *GetTrigger() const { return ( trigger.type == unused ) ? nullptr : &trigger; }

    [[nodiscard]] subvector<Entry_t> GetRing(const size_t &ringNo);
    [[nodiscard]] std::pair<std::vector<Entry_t>, std::vector<Entry_t>> GetTrap(const size_t &ringNo) const;
};

#endif //XIA2TREE_EVENT_H
