#ifndef FUNCTIONS_H
#define FUNCTIONS_H

#include <vector>
#include <string>
#include <map>


using FW_Map_t = std::map<std::string, std::string>;


int AdjustBaselineOffset(unsigned short modNo);
std::vector<unsigned short> ReadSlotMap(const char *ini_file = "pxisys.ini");
FW_Map_t ParseFWconfigFile(const char *config);
bool GetFirmwareFile(FW_Map_t firmwares,
                     const unsigned short &revision, const unsigned short &ADCbits, const unsigned short &ADCMSPS,
                     char *ComFPGA, char *SPFPGA, char *DSPcode, char *DSPVar);

#endif // FUNCTIONS_H
