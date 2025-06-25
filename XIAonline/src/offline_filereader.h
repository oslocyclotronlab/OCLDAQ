//
// Created by Vetle Wegner Ingeberg on 25/06/2025.
//

#ifndef OFFLINE_FILEREADER_H
#define OFFLINE_FILEREADER_H

#include <memory>
#include <vector>

#include "MemoryMap.h"

class OfflineFileReader
{
private:
   std::unique_ptr<IO::MemoryMap> file_map;
   const uint32_t* begin;
   const uint32_t* end;
   const uint32_t* pos;
public:
  OfflineFileReader(const char* fname);
  std::vector<uint32_t> read(const size_t& size=32768);

};

#endif //OFFLINE_FILEREADER_H
