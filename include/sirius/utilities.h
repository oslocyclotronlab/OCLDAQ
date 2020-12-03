// -*- c++ -*-

#ifndef UTILITIES_H
#define UTILITIES_H 1

#include <iosfwd>
#include <string>

#include <string.h>
#include <sys/shm.h>

std::string ioprintf(const char* format, ...)
    __attribute__ ((format (printf, 1, 2)));

bool file_exists(std::string const& filename);

void init_sockaddr(struct sockaddr_in *name, const char *hostname, int port);
int make_socket(int port, bool reuse);

inline void swap(unsigned int& v)
{
    if( true )
        v = ((v&0xff) << 24) | ((v&0xff00)<<8) | ((v&0xff0000)>>8) | ((v&0xff000000)>>24);
}

int* attach_shared(key_t key, size_t size, bool create);

std::string escape(const std::string& txt);

std::istream& skipsymbols(std::istream& fp);

std::string strip(const std::string& s);

#endif /* UTILITIES_H */
