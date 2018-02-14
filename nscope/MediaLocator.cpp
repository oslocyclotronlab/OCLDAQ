#include "MediaLocator.h"
#include <unistd.h>

#include <iostream>

std::string MediaLocator::locateFile(const std::string& file)
{
  std::string fullPath("");

  for (auto& dirPath :  m_searchPath) {
    std::string testPath = dirPath + "/" + file;
    if (fileExists(dirPath + "/" + file)) {
      fullPath = testPath; 
      break;
    }
  }

  return fullPath;
}


void MediaLocator::addPath(const std::string& path) 
{
  m_searchPath.insert(path);
}

bool MediaLocator::fileExists(const std::string& path) 
{
  int status = ::access(path.c_str(), R_OK);
  return (status == 0);
}

