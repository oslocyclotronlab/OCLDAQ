#ifndef MEDIALOCATOR_H
#define MEDIALOCATOR_H

#include <string>
#include <set>

class MediaLocator {

    private:
      std::set<std::string> m_searchPath;

    public:
      MediaLocator() = default;
      MediaLocator(const MediaLocator&) = default;
      ~MediaLocator() = default;
      
      std::string locateFile(const std::string& file);

      void addPath(const std::string& path);

    private:
      bool fileExists(const std::string& path);

};

#endif
