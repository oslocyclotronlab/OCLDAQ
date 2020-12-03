// -*- c++ -*-

#ifndef ACQ_MESSAGE_H
#define ACQ_MESSAGE_H 1

#include <string>

enum { LOG_ERR, LOG_INFO };
extern void log_message(int level, const char *fmt, ...)
    __attribute__ ((format (printf, 2, 3)));

void logbook_message(std::string const& subject, std::string const& message);

#endif
