//
// Created by Vetle Wegner Ingeberg on 25/06/2025.
//

#include "offline_filereader.h"
#include <memory>

OfflineFileReader::OfflineFileReader(const char* fname)
    : file_map( std::make_unique<IO::MemoryMap>(fname) )
    , begin( file_map->GetPtr<uint32_t>() )
    , end( file_map->GetPtr<uint32_t>()+file_map->GetSize<uint32_t>() )
    , pos( begin )
{}

std::vector<uint32_t> OfflineFileReader::read(const size_t& size) {
    if ( pos >= end )
      return {};
    if ( pos + size < end ){
      auto vec = std::vector<uint32_t>(pos, pos+size);
      pos += size;
      return vec;
    } else {
        auto vec = std::vector<uint32_t>(pos, end);
        pos = end;
        return vec;
    }
}