#include "MemoryMap.h"

#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>

#include <cstring>
#include <cerrno>
#include <iostream>
#include <string>

using namespace IO;

[[noreturn]] void throw_errno(const std::string &prefix) {
    std::string errmsg = prefix;
    errmsg += ", got error '";
    errmsg += std::strerror(errno);
    errmsg += "'.";
    throw std::runtime_error(errmsg);
}

MemoryMap::MemoryMap(const char *fname, const int& flags)
    : memory_buffer(nullptr)
    , file( 0 )
    , mapped_size( 0 )
    , used_size( 0 )
    , writable( false )
{


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
    struct stat sb{};
    if (fstat(file, &sb) == -1) {
      std::string errmsg = "Unable to estimate file size, got error '";
      errmsg += strerror(errno);
      errmsg += "'.";
      throw std::runtime_error(errmsg);
    }

    mapped_size = sb.st_size;

    // Try to memory map the file
    void* addr = mmap(nullptr, mapped_size, PROT_READ, flags, file, 0);
    if (addr == MAP_FAILED) {
        std::string errmsg = "Unable to memory map file, got error '";
        errmsg += strerror(errno);
        errmsg += "'.";
        throw std::runtime_error(errmsg);
    }
    memory_buffer = static_cast<char *>(addr);
}

MemoryMap::MemoryMap(const char *fname, size_t max_size, bool truncate_file, const int& flags)
    : memory_buffer(nullptr)
    ,  file(-1)
    ,  mapped_size(0)
    ,  used_size(0)
    ,  writable( true )
{
    if (max_size == 0) {
      throw std::invalid_argument("max_size must be > 0 for writable mapping");
    }

    int o_flags = O_RDWR | O_CREAT;
    if (truncate_file) {
      o_flags |= O_TRUNC;
    }
    // Mode only matters with O_CREAT
    file = open(fname, o_flags, 0644);
    if (file == -1) {
        std::string errmsg = "Unable to open file '";
        errmsg += fname;
        errmsg += "', got error '";
        errmsg += strerror(errno);
        errmsg += "'.";
        throw std::runtime_error(errmsg);
    }

    // Ensure the file is at least max_size bytes.
    if (ftruncate(file, static_cast<off_t>(max_size)) == -1) {
        close(file);
        std::string errmsg = "Unable to estimate file size, got error '";
        errmsg += strerror(errno);
        errmsg += "'.";
        throw std::runtime_error(errmsg);
    }

    mapped_size = max_size;
    used_size   = 0;  // nothing written yet

    // Try to memory map the file
    void *addr = mmap(nullptr, mapped_size, PROT_READ | PROT_WRITE, flags, file, 0);
    if (addr == MAP_FAILED) {
      close(file);
      std::string errmsg = "Unable to memory map file, got error '";
      errmsg += strerror(errno);
      errmsg += "'.";
      throw std::runtime_error(errmsg);
    }
    memory_buffer = static_cast<char *>(addr);
}


// --------------------
// Destructor
// --------------------
MemoryMap::~MemoryMap() {
    if (memory_buffer && mapped_size > 0) {

        // If writable, shrink file to the used size.
        if (writable) {
            // Clamp used_size to mapped_size just in case.
            if (used_size > mapped_size)
                used_size = mapped_size;

            if (ftruncate(file, static_cast<off_t>(used_size)) == -1) {
                std::string errmsg = "Unable to shrink file to used size, got error '";
                errmsg += std::strerror(errno);
                errmsg += "'.";
                std::cerr << errmsg << std::endl;
            }

            // Optionally flush changes before unmapping.
            msync(memory_buffer, mapped_size, MS_SYNC);
        }

        if (munmap(memory_buffer, mapped_size) == -1) {
            std::string errmsg = "Unable to de-allocate mapped memory, got error '";
            errmsg += std::strerror(errno);
            errmsg += "'.";
            std::cerr << errmsg << std::endl;
        }
    }

    if (file != -1) {
        close(file);
    }
}

// --------------------
// SetUsedSize
// --------------------
void MemoryMap::SetUsedSize(std::size_t new_used_size) {
    if (!writable) throw std::logic_error("SetUsedSize called on read-only mapping");
    if (new_used_size > mapped_size) throw std::out_of_range("new_used_size exceeds mapped capacity");
    used_size = new_used_size;
}