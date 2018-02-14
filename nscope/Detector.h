#ifndef DETECTOR_H_
#define DETECTOR_H_
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

//    MultiplicityMask QueryMultiplicity(UShort_t module, UShort_t ChanNum);    
//    void SetMultiplicityMask(UShort_t module, UShort_t ChanNum, const MultiplicityMask& mask);    
    
//    Double_t QueryCoincidenceTimeWindow(UShort_t module, UShort_t chanNum);

private:

	bool newset;
	bool booted;
 	unsigned short *PXISlotMap;
  	unsigned short OfflineMode;
  	//char ComFPGAConfigFile[FILENAME_STR_MAXLEN];
  	//char SPFPGAConfigFile[FILENAME_STR_MAXLEN];
  	//char TrigFPGAConfigFile[FILENAME_STR_MAXLEN];
  	//char DSPCodeFile[FILENAME_STR_MAXLEN];
  	char DSPParFile[FILENAME_STR_MAXLEN];
  	//char DSPVarFile[FILENAME_STR_MAXLEN];

	char ComFPGAConfigFile_RevBCD[FILENAME_STR_MAXLEN];
	char ComFPGAConfigFile_RevF_100MHz_14Bit[FILENAME_STR_MAXLEN];
	char ComFPGAConfigFile_RevF_100MHz_16Bit[FILENAME_STR_MAXLEN];
	char ComFPGAConfigFile_RevF_250MHz_12Bit[FILENAME_STR_MAXLEN];
	char ComFPGAConfigFile_RevF_250MHz_14Bit[FILENAME_STR_MAXLEN];
	char ComFPGAConfigFile_RevF_250MHz_16Bit[FILENAME_STR_MAXLEN];
	char ComFPGAConfigFile_RevF_500MHz_12Bit[FILENAME_STR_MAXLEN];
	char ComFPGAConfigFile_RevF_500MHz_14Bit[FILENAME_STR_MAXLEN];
	
	char SPFPGAConfigFile_RevBCD[FILENAME_STR_MAXLEN];
	char SPFPGAConfigFile_RevF_100MHz_14Bit[FILENAME_STR_MAXLEN];
	char SPFPGAConfigFile_RevF_100MHz_16Bit[FILENAME_STR_MAXLEN];
	char SPFPGAConfigFile_RevF_250MHz_12Bit[FILENAME_STR_MAXLEN];
	char SPFPGAConfigFile_RevF_250MHz_14Bit[FILENAME_STR_MAXLEN];
	char SPFPGAConfigFile_RevF_250MHz_16Bit[FILENAME_STR_MAXLEN];
	char SPFPGAConfigFile_RevF_500MHz_12Bit[FILENAME_STR_MAXLEN];
	char SPFPGAConfigFile_RevF_500MHz_14Bit[FILENAME_STR_MAXLEN];
	
	char DSPCodeFile_RevBCD[FILENAME_STR_MAXLEN];
	char DSPCodeFile_RevF_100MHz_14Bit[FILENAME_STR_MAXLEN];
	char DSPCodeFile_RevF_100MHz_16Bit[FILENAME_STR_MAXLEN];
	char DSPCodeFile_RevF_250MHz_12Bit[FILENAME_STR_MAXLEN];
	char DSPCodeFile_RevF_250MHz_14Bit[FILENAME_STR_MAXLEN];
	char DSPCodeFile_RevF_250MHz_16Bit[FILENAME_STR_MAXLEN];
	char DSPCodeFile_RevF_500MHz_12Bit[FILENAME_STR_MAXLEN];
	char DSPCodeFile_RevF_500MHz_14Bit[FILENAME_STR_MAXLEN];
	
	char DSPVarFile_RevBCD[FILENAME_STR_MAXLEN];
	char DSPVarFile_RevF_100MHz_14Bit[FILENAME_STR_MAXLEN];
	char DSPVarFile_RevF_100MHz_16Bit[FILENAME_STR_MAXLEN];
	char DSPVarFile_RevF_250MHz_12Bit[FILENAME_STR_MAXLEN];
	char DSPVarFile_RevF_250MHz_14Bit[FILENAME_STR_MAXLEN];
	char DSPVarFile_RevF_250MHz_16Bit[FILENAME_STR_MAXLEN];
	char DSPVarFile_RevF_500MHz_12Bit[FILENAME_STR_MAXLEN];
	char DSPVarFile_RevF_500MHz_14Bit[FILENAME_STR_MAXLEN];

	unsigned short ModRev[MAX_NUM_PIXIE16_MODULES];      // module revision in hex format (NSCL had D and F modules
	unsigned short ModADCBits[MAX_NUM_PIXIE16_MODULES];  // adc bits of a module
	unsigned short ModADCMSPS[MAX_NUM_PIXIE16_MODULES];  // sampling rate of a module
	unsigned int   ModSerNum[MAX_NUM_PIXIE16_MODULES];   // module serial number

	char Pixie16_Com_FPGA_File[FILENAME_STR_MAXLEN];
	char Pixie16_SP_FPGA_File[FILENAME_STR_MAXLEN];
	char Pixie16_DSP_Code_File[FILENAME_STR_MAXLEN];
	char Pixie16_DSP_Var_File[FILENAME_STR_MAXLEN];
	char Pixie16_Trig_FPGA_File[FILENAME_STR_MAXLEN];
	
  	bool ReadConfigFile (const char *config = ".cfgPixie");
  	bool ReadFirmwareVersionFile (const char *config = "DDASFirmwareVersions.txt");
  	bool BootSystem (unsigned short NumModules, unsigned short *PXISlotMap);

	//get the sampling frequency of a particular module
};

#endif /*DETECTOR_H_*/
