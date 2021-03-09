#include "WriteTerminal.h"

#include <iostream>


void WriteTerminal::WriteError(const char *message)
{
    std::lock_guard<std::mutex> guard_io(io_mutex);
    std::cerr << message << std::flush;
    return;
}

void WriteTerminal::Write(const char *message)
{
    std::lock_guard<std::mutex> guard_io(io_mutex);
    std::cout << message << std::flush;
    return;
}
