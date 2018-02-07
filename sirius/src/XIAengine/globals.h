#ifndef GLOBALS_H
#define GLOBALS_H

#include "DataMerger.h"
#include "WriteTerminal.h"


// Global objects
extern EventMerger merger;
extern WriteTerminal termWrite;

extern char leaveprog;
extern bool stopped;
extern bool stopXIA;
extern bool have_spit; // To signal that we have a split of an event across two buffers. (Solution is: pad or add)

#endif // GLOBALS_H
