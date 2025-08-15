#include "experimentsetup.h"


// List of all detectors, sorted by the address. Needs to be edited by
// the user whenever the addresses are changed.

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus


DetectorInfo_t pDetector[] =
{
    {0, f000MHz, unused, 0, 0},
    {1, f000MHz, unused, 0, 0},
    {2, f000MHz, unused, 0, 0},
    {3, f000MHz, unused, 0, 0},
    {4, f000MHz, unused, 0, 0},
    {5, f000MHz, unused, 0, 0},
    {6, f000MHz, unused, 0, 0},
    {7, f000MHz, unused, 0, 0},
    {8, f000MHz, unused, 0, 0},
    {9, f000MHz, unused, 0, 0},
    {10, f000MHz, unused, 0, 0},
    {11, f000MHz, unused, 0, 0},
    {12, f000MHz, unused, 0, 0},
    {13, f000MHz, unused, 0, 0},
    {14, f000MHz, unused, 0, 0},
    {15, f000MHz, unused, 0, 0},
    {16, f000MHz, unused, 0, 0},
    {17, f000MHz, unused, 0, 0},
    {18, f000MHz, unused, 0, 0},
    {19, f000MHz, unused, 0, 0},
    {20, f000MHz, unused, 0, 0},
    {21, f000MHz, unused, 0, 0},
    {22, f000MHz, unused, 0, 0},
    {23, f000MHz, unused, 0, 0},
    {24, f000MHz, unused, 0, 0},
    {25, f000MHz, unused, 0, 0},
    {26, f000MHz, unused, 0, 0},
    {27, f000MHz, unused, 0, 0},
    {28, f000MHz, unused, 0, 0},
    {29, f000MHz, unused, 0, 0},
    {30, f000MHz, unused, 0, 0},
    {31, f000MHz, unused, 0, 0},

    {32, f250MHz, labr, 0, 0},
    {33, f250MHz, unused, 0, 0},
    {34, f250MHz, unused, 0, 0},
    {35, f250MHz, unused, 0, 0},
    {36, f250MHz, labr, 1, 0},
    {37, f250MHz, unused, 0, 0},
    {38, f250MHz, unused, 0, 0},
    {39, f250MHz, unused, 0, 0},
    {40, f250MHz, labr, 2, 0},
    {41, f250MHz, unused, 0, 0},
    {42, f250MHz, unused, 0, 0},
    {43, f250MHz, unused, 0, 0},
    {44, f250MHz, labr, 3, 0},
    {45, f250MHz, unused, 0, 0},
    {46, f250MHz, unused, 0, 0},
    {47, f250MHz, unused, 0, 0},
    
    {48, f250MHz, unused, 0, 0},
    {49, f250MHz, labr, 4, 0},
    {50, f250MHz, unused, 0, 0},
    {51, f250MHz, unused, 0, 0},
    {52, f250MHz, labr, 5, 0},
    {53, f250MHz, unused, 0, 0},
    {54, f250MHz, unused, 0, 0},
    {55, f250MHz, unused, 0, 0},
    {56, f250MHz, labr, 6, 0},
    {57, f250MHz, unused, 0, 0},
    {58, f250MHz, unused, 0, 0},
    {59, f250MHz, unused, 0, 0},
    {60, f250MHz, labr, 7, 0},
    {61, f250MHz, unused, 0, 0},
    {62, f250MHz, unused, 0, 0},
    {63, f250MHz, unused, 0, 0},
    
    {64, f250MHz, labr, 8, 0},
    {65, f250MHz, unused, 0, 0},
    {66, f250MHz, unused, 0, 0},
    {67, f250MHz, unused, 0, 0},
    {68, f250MHz, labr, 9, 0},
    {69, f250MHz, unused, 0, 0},
    {70, f250MHz, unused, 0, 0},
    {71, f250MHz, unused, 0, 0},
    {72, f250MHz, labr, 10, 0},
    {73, f250MHz, unused, 0, 0},
    {74, f250MHz, unused, 0, 0},
    {75, f250MHz, unused, 0, 0},
    {76, f250MHz, labr, 11, 0},
    {77, f250MHz, unused, 0, 0},
    {78, f250MHz, unused, 0, 0},
    {79, f250MHz, unused, 0, 0},
};


DetectorInfo_t GetDetector(uint16_t address)
{
    return (address < TOTAL_NUMBER_OF_ADDRESSES) ? pDetector[address] : pDetector[0];
}

enum ADCSamplingFreq GetSamplingFrequency(uint16_t address)
{
    return (address < TOTAL_NUMBER_OF_ADDRESSES) ? pDetector[address].sfreq : f000MHz;
}

#ifdef __cplusplus
}
#endif // __cplusplus
