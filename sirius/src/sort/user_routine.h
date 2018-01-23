// -*- c++ -*-

#ifndef USER_ROUTINE_H
#define USER_ROUTINE_H 1

#include "sort_format.h"

#include <string>

class UserRoutine {
public:
    virtual ~UserRoutine() { }
    
    /** Return a string useful for identifying the specific user sorting routine. */
    virtual const char* GetId() = 0;

    virtual bool Init(bool online) { return true || online; }

    /** Called when a new 'data' statement is found (e.g. a new file is opened). */
    virtual bool Data(const std::string& filename) { return true || filename.empty(); }

    /** Called when a new buffer is started. */
    virtual bool Buffer(int buffer_number) { return true || buffer_number>0; }

    virtual bool Cmd(const std::string& cmd) = 0;
    virtual bool Sort(unpacked_t* u) = 0;
    virtual bool Finish() { return true; }
};

#endif /* USER_ROUTINE_H */
