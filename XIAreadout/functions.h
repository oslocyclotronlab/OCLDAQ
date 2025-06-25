#ifndef FUNCTIONS_H
#define FUNCTIONS_H

#include <vector>

int AdjustBaselineOffset(unsigned short modNo);
std::vector<unsigned short> ReadSlotMap(const char *ini_file = "pxisys.ini");

#endif // FUNCTIONS_H
