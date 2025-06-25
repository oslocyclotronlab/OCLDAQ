#include "MemoryMap.h"

#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>

#include <cerrno>
#include <iostream>
#include <string>

using namespace IO;

MemoryMap::MemoryMap(const char *fname) : memory_buffer(nullptr), file(0), size(0) {
  struct stat sb;

  file = open(fname, O_RDONLY, NULL);

  if (file == -1) {
    std::string errmsg = "Unable to open file '";
    errmsg += fname;
    errmsg += "', got error '";
    errmsg += strerror(errno);
    errmsg += "'.";
    throw std::runtime_error(errmsg);
  }

  // Check size of file
  if (fstat(file, &sb) == -1) {
    std::string errmsg = "Unable to estimate file size, got error '";
    errmsg += strerror(errno);
    errmsg += "'.";
    throw std::runtime_error(errmsg);
  }

  size = sb.st_size;

  // Try to memory map the file
  memory_buffer = reinterpret_cast<char *>(mmap(nullptr, size, PROT_READ, MAP_PRIVATE, file, 0));
  if (memory_buffer == MAP_FAILED) {
    std::string errmsg = "Unable to memory map file, got error '";
    errmsg += strerror(errno);
    errmsg += "'.";
    throw std::runtime_error(errmsg);
  }
}

/*MemoryMap::MemoryMap(MemoryMap &&lhs) : memory_buffer( lhs.memory_buffer ), file( lhs.file ), size( lhs.size )
{
    // Need to set all of these fields to zero for the other object since we now own the resources.
    lhs.memory_buffer = nullptr;
    lhs.file = 0;
    lhs.size = 0;
}

MemoryMap &MemoryMap::operator=(MemoryMap &&lhs)
{
    // First we need to ensure that any resources held by this object are destroyed
    if (memory_buffer) {
        if (munmap(reinterpret_cast<void *>(memory_buffer), size) == -1) {
            std::string errmsg = "Unable to de-allocate mapped memory, got error '";
            errmsg += strerror(errno);
            errmsg += "'.";
            std::cerr << errmsg << std::endl;
        }
    }
    close(file);

    // Next we will take control over the other objects resources
    memory_buffer = lhs.memory_buffer;
    file = lhs.file;
    size = lhs.size;

    // Remove other objects references to avoid them being cleaned up by the destructor
    lhs.memory_buffer = nullptr;
    lhs.file = 0;
    lhs.size = 0;

    return *this;
}*/

MemoryMap::~MemoryMap() {
  // First we need de-map the memory.
  if (memory_buffer) {
    if (munmap(reinterpret_cast<void *>(memory_buffer), size) == -1) {
      std::string errmsg = "Unable to de-allocate mapped memory, got error '";
      errmsg += strerror(errno);
      errmsg += "'.";
      std::cerr << errmsg << std::endl;
    }
  }
  close(file);
}
