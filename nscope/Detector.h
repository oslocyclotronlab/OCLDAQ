#ifndef DETECTOR_H_
#define DETECTOR_H_

#include "Configuration.h"


#include <iostream>
#include <fstream>
#include <stdlib.h>
#include <vector>
#include "pixie16app_defs.h"
#include "pixie16app_export.h"
#include "Rtypes.h"
#define  FILENAME_STR_MAXLEN     256
#define  MAX_NUM_PIXIE16_MODULES 24
#define  TOTAL_PIXIE16_VARIANTS  8

class Detector
{
public:
	Detector();
	virtual ~Detector();
	int ExitSystem();
	void Boot();
	bool IsBooted();
	void SetSetFile(const char *newsetfile);
	int Syncronise ();
    int StartLSMRun (int continue_run);
    int StartRun (int contine_run, int modnum);
	int StartBaselineRun (int contine_run, int modnum);
  	int RunStatus ();
  	int Write2FileLSM(char *name);
  	int AcquireADCTrace (unsigned short *trace, unsigned long size,
		       unsigned short module, unsigned short ChanNum);

	unsigned short NumModules;

    unsigned short GetNumberModules() const {return NumModules;}
    std::vector<unsigned short> GetPXISlotMap() const;
    unsigned short GetModuleMSPS(unsigned int i) {return ModADCMSPS[i];}

private:
    int synchronize(int modnum);

private:

    bool m_newset;
	bool booted;
 	unsigned short *PXISlotMap;
  	unsigned short OfflineMode;
    DAQ::DDAS::Configuration m_config;

    unsigned short ModRev[MAX_NUM_PIXIE16_MODULES];      // module revision in hex format (NSCL had D and F modules
    unsigned short ModADCBits[MAX_NUM_PIXIE16_MODULES];  // adc bits of a module
    unsigned short ModADCMSPS[MAX_NUM_PIXIE16_MODULES];  // sampling rate of a module

};

#endif /*DETECTOR_H_*/
