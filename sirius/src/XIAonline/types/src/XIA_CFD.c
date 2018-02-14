#include "XIA_CFD.h"



#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

#include <stdlib.h>
#include <stdint.h>

#define BIT15TO15	0x8000
#define BIT14TO14	0x4000
#define BIT15TO13	0xE000
#define BIT14TO0	0x7FFF
#define BIT13TO0	0x3FFF
#define BIT12TO0	0x1FFF

double XIA_CFD_Fraction_100MHz(uint16_t CFDvalue, char* fail)
{
	double correction;
	uint32_t cfdfailbit, timecfd;

	cfdfailbit = ((CFDvalue & BIT15TO15) >> 15);
	timecfd = ((CFDvalue & BIT14TO0) >> 0);
	correction = (10*(double)timecfd)/32768.0;
    *fail = cfdfailbit;
	return correction;
}

double XIA_CFD_Fraction_250MHz(uint16_t CFDvalue, char* fail)
{
	double correction;
	uint32_t cfdfailbit, cfdtrigsource, timecfd;

	cfdfailbit = ((CFDvalue & BIT15TO15) >> 15);
	cfdtrigsource = ((CFDvalue & BIT14TO14) >> 14);
	timecfd = ((CFDvalue & BIT13TO0) >> 0);
	correction = (((double)timecfd)/16384.0 - cfdtrigsource)*4.0;
    *fail = cfdfailbit;
	return correction;
}

double XIA_CFD_Fraction_500MHz(uint16_t CFDvalue, char* fail)
{
	double correction;
	uint32_t cfdtrigsource, timecfd;

	cfdtrigsource = ((CFDvalue & BIT15TO13) >> 13);
	timecfd = ((CFDvalue & BIT12TO0) >> 0);
	correction = (((double)timecfd)/8192.0 + cfdtrigsource - 1.0)*2.0;
    *fail = (cfdtrigsource >= 7) ? 1 : 0;
	return correction;
}

double XIA_time_in_ns_100MHz(int64_t timestamp, uint16_t CFDvalue)
{
	double correction, time_in_ns;
    char fail;

	correction = XIA_CFD_Fraction_100MHz(CFDvalue, &fail);
	correction = fail ? 10.*((double)rand()/RAND_MAX) : correction;

	time_in_ns = 10.0*(double)timestamp;
	time_in_ns = time_in_ns + correction;
	return time_in_ns;
}

double XIA_time_in_ns_250MHz(int64_t timestamp, uint16_t CFDvalue)
{
	double correction, time_in_ns;
    char fail;

	correction = XIA_CFD_Fraction_250MHz(CFDvalue, &fail);
	correction = fail ? 4.0*((double)rand()/RAND_MAX - 1) : correction;

	time_in_ns = 8.0*(double)timestamp;
	time_in_ns = time_in_ns + correction;
	return time_in_ns;
}

double XIA_time_in_ns_500MHz(int64_t timestamp, uint16_t CFDvalue)
{
	double correction, time_in_ns;
    char fail;

	correction = XIA_CFD_Fraction_500MHz(CFDvalue, &fail);
	correction = fail ? 2.0*(7.*(double)rand()/RAND_MAX - 1) : correction;

	time_in_ns = 10.0*(double)timestamp;
	time_in_ns = time_in_ns + correction;
	return time_in_ns;
}

#ifdef __cplusplus
}
#endif // __cplusplus
