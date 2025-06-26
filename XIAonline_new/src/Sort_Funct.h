#ifndef SORT_FUNCT_H
#define SORT_FUNCT_H

#include <vector>

#include "Event.h"

struct calibration_t;

calibration_t *GetCalibration();

void sort_singles(const std::vector<word_t> &buffer);
void sort_coincidence(Event &event);


#endif // SORT_FUNCT_H
