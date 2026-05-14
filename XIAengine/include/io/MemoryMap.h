#pragma once

#include <cstddef>
#include <sys/fcntl.h>
#include <sys/mman.h>

namespace IO {

  /*!
   * \brief Memory-mapped input file that can be accessed by multiple threads
   */
  class MemoryMap {
  private:
      /*! The memory mapped file */
      char *memory_buffer;

      /*! File descriptor */
      int file;

      /*! Size of mapping in bytes (max size for writable files) */
      std::size_t mapped_size;

      /*! Number of bytes actually used/valid (for writable files) */
      std::size_t used_size;

      /*! Is this mapping writable? */
      const bool writable;

  public:
      /*!
       * @brief Open a file for read only
       * @param fname Path to file
       * @param flags Flags for the mapping (e.g. MAP_PRIVATE OR MAP_SHARED)
       */
      MemoryMap(const char *fname, const int& flags = MAP_SHARED);

      /*!
       * \brief Open (and possibly create) a file for read-write use.
       *
       * \param fname     Path to file.
       * \param max_size  Maximum size to map/allocate for this file.
       *                  The file will be extended to this size.
       * \param truncate  If true, truncate existing file to zero before sizing.
       */
      MemoryMap(const char *fname, size_t max_size, bool truncate, const int& flags = MAP_SHARED);

      MemoryMap(MemoryMap &) = delete;
      MemoryMap(MemoryMap &&) = delete;

      ~MemoryMap();

      template <typename T = char> inline const T *GetPtr() const { return reinterpret_cast<const T *>(memory_buffer); }
      template <typename T = char> inline size_t GetSize() const { return mapped_size / sizeof(T); }

      /*!
       * \brief Get pointer to underlying bytes (writable).
       *        Only valid if the mapping is writable.
       */
      template <typename T = char>
      inline T *GetWritePtr() {
          return reinterpret_cast<T *>(memory_buffer);
      }


      /*!
       * \brief Set the number of bytes actually used (for writable files).
       *        Must be <= mapped_size.
       */
      void SetUsedSize(size_t new_size);

      size_t GetUsedSize() const { return used_size; }

      bool IsWritable() const { return writable; }
  };

}  // namespace IO
