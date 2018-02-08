#include "Detector.h"
#include <string.h>
#include <limits>
#include <iostream>
#include <fstream>

using namespace std;
Detector::Detector()
{
  newset = false;
  booted = false;
  const char* vFile ="/home/vetlewi/Desktop/nscope/DDASFirmwareVersions.txt";
  ReadFirmwareVersionFile(vFile);
  const char* pFile ="cfgPixie16.txt";
  ReadConfigFile (pFile);
  //BootSystem (NumModules, PXISlotMap);
}

Detector::~Detector()
{
}

void Detector::Boot()
{
  BootSystem(NumModules, PXISlotMap);
}

int Detector::ExitSystem()
{
  cout << " Exit System " << endl;

  int retval = 0;
  retval = Pixie16ExitSystem (NumModules);

  if (retval != 0) {
    cout << " Pixie exit has failed: " << retval << endl;
    return retval;
  } else {
    booted = false;
    return 0;
  }

}

void Detector::SetSetFile(const char *newsetfile)
{
  //char tempname[FILENAME_STR_MAXLEN];
  strcpy(DSPParFile,newsetfile);
  //DSPParFile = tempname;
  newset = true;
  cout << "Using new set file " << DSPParFile << endl;;
}

bool Detector::ReadConfigFile (const char *config)
{
  ifstream input;
  string line;
  char *temp = new char[FILENAME_STR_MAXLEN];
  input.open (config, ios::in);
  int crateNum;
  
  if (input.fail ()) {
    cout << "Can't open the config file ! " << config << endl << flush;
    return false;
  } else {
    cout << "Reading config file... " << config << endl << flush;
  }
  
  // Read and ignore the crate number that readout uses:
  
  input >> crateNum;
  input.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
  
  input >> NumModules;
  cout << "\n" << NumModules << " modules, in slots: ";
  input.getline (temp, FILENAME_STR_MAXLEN);
  PXISlotMap = new unsigned short[NumModules];
  for (int i = 0; i < NumModules; i++) {
    input >> PXISlotMap[i];
    input.getline (temp, FILENAME_STR_MAXLEN);
    cout << PXISlotMap[i] << " ";
  }
  cout << "\n" << endl;
  input >> DSPParFile;
  input.getline (temp, FILENAME_STR_MAXLEN);
  //cout << "DSPParFile:         " << DSPParFile << endl;

  //check to make sure that this line contains a set file (.set extension)
  //since the format has changed from previous versions of the code.
  if( strstr(DSPParFile, ".set") == NULL) {
    cout << "The file " << DSPParFile << " read in from " << config 
	 << " does not appear to be a *.set file required by DDAS " << endl;
    cout << "Program will now terminate " << endl;
    exit(1);
  }


  while (getline(input, line)) {
    if (line == "[100MSPS]") {
      // load in files to overide defaults
      //load syspixie
      input >> ComFPGAConfigFile_RevBCD;
      //load fippipixe
      input >> SPFPGAConfigFile_RevBCD;
      //load ldr file
      input >> DSPCodeFile_RevBCD;
      //load var file
      input >> DSPVarFile_RevBCD;

    } else if (line == "[250MSPS]"){
      input >> ComFPGAConfigFile_RevF_250MHz_14Bit;
      input >> SPFPGAConfigFile_RevF_250MHz_14Bit;
      input >> DSPCodeFile_RevF_250MHz_14Bit;
      input >> DSPVarFile_RevF_250MHz_14Bit;

    } else if (line == "[500MSPS]"){
      input >> ComFPGAConfigFile_RevF_500MHz_12Bit;
      input >> SPFPGAConfigFile_RevF_500MHz_12Bit;
      input >> DSPCodeFile_RevF_500MHz_12Bit;
      input >> DSPVarFile_RevF_500MHz_12Bit;
    }
    
  }
  
  ////cout << endl << "Firmware files: \n";
  //input >> ComFPGAConfigFile;
  //input.getline (temp, FILENAME_STR_MAXLEN);
  ////cout << "ComFPGAConfigFile:  " << ComFPGAConfigFile << endl;
  //input >> SPFPGAConfigFile;
  //input.getline (temp, FILENAME_STR_MAXLEN);
  ////cout << "SPFPGAConfigFile:   " << SPFPGAConfigFile << endl;
  //input >> TrigFPGAConfigFile;
  //input.getline (temp, FILENAME_STR_MAXLEN);
  ////cout << "TrigFPGAConfigFile: " << TrigFPGAConfigFile << endl;
  //input >> DSPCodeFile;
  //input.getline (temp, FILENAME_STR_MAXLEN);
  ////cout << "DSPCodeFile:        " << DSPCodeFile << endl;
  //input >> DSPVarFile;
  //input.getline (temp, FILENAME_STR_MAXLEN);
  ////cout << "DSPVarFile:         " << DSPVarFile << endl;
  ////cout << "--------------------------------------------------------\n\n";
  

  return true;
}

bool Detector::ReadFirmwareVersionFile(const char *config){

  ifstream input;
  string line;
  unsigned short count;
  input.open(config, ios::in);

  if(input.fail()) {
    cout << "Can't open the FirmwareVersion file !" << config << endl << flush;
    return false;
  } else {
    cout << "Reading Firmware Version file... " << config << endl << flush;
  }

  // read input file with code provided by XIA using XIA defined formatted file
  while(!(input.eof()))
    {
      input>>line;
        
      if(line == "[FPGAFirmwarefiles]") 
	{
	  cout<<"Found Firmware    #"<<line<<endl;
	  
	  // Loop through all possible Pixie-16 variants
	  count = 0;
	  while (count < TOTAL_PIXIE16_VARIANTS)
	    {
	       input>>line;
	      
	      if (line == "***Rev-B/C/D***")
		{
		  input >> ComFPGAConfigFile_RevBCD;
		  input >> SPFPGAConfigFile_RevBCD;
		  count ++;
		}
	      else if (line == "***Rev-F-14Bit-100MSPS***")
		{
		  input >> ComFPGAConfigFile_RevF_100MHz_14Bit;
		  input >> SPFPGAConfigFile_RevF_100MHz_14Bit;
		  count ++;
		}
	      else if (line == "***Rev-F-16Bit-100MSPS***")
		{
		  input >> ComFPGAConfigFile_RevF_100MHz_16Bit;
		  input >> SPFPGAConfigFile_RevF_100MHz_16Bit;
		  count ++;
		}
	      else if (line == "***Rev-F-12Bit-250MSPS***")
		{
		  input >> ComFPGAConfigFile_RevF_250MHz_12Bit;
		  input >> SPFPGAConfigFile_RevF_250MHz_12Bit;
		  count ++;
		}
	      else if (line == "***Rev-F-14Bit-250MSPS***")
		{
		  input >> ComFPGAConfigFile_RevF_250MHz_14Bit;
		  input >> SPFPGAConfigFile_RevF_250MHz_14Bit;
		  count ++;
		}
	      else if (line == "***Rev-F-16Bit-250MSPS***")
		{
		  input >> ComFPGAConfigFile_RevF_250MHz_16Bit;
		  input >> SPFPGAConfigFile_RevF_250MHz_16Bit;
		  count ++;
		}
	      else if (line == "***Rev-F-12Bit-500MSPS***")
		{
		  input >> ComFPGAConfigFile_RevF_500MHz_12Bit;
		  input >> SPFPGAConfigFile_RevF_500MHz_12Bit;
		  count ++;
		}
	      else if (line == "***Rev-F-14Bit-500MSPS***")
		{
		  input >> ComFPGAConfigFile_RevF_500MHz_14Bit;
		  input >> SPFPGAConfigFile_RevF_500MHz_14Bit;
		  // cout<<"test: "<<ComFPGAConfigFile_RevF_500MHz_14Bit<<endl;
		  count ++;
		}
	    }
	}
      else if (line == "[DSPCodefiles]")
	{
	  // Loop through all possible Pixie-16 variants
	  count = 0;
	  while (count < TOTAL_PIXIE16_VARIANTS)
	    {
	      input>>line;
	      
	      if (line == "***Rev-B/C/D***")
		{
		  input >> DSPCodeFile_RevBCD;
		  input >> DSPVarFile_RevBCD;
		  count ++;
		}
	      else if (line == "***Rev-F-14Bit-100MSPS***")
		{
		  input >> DSPCodeFile_RevF_100MHz_14Bit;
		  input >> DSPVarFile_RevF_100MHz_14Bit;
		  count ++;
		}
	      else if (line == "***Rev-F-16Bit-100MSPS***")
		{
		  input >> DSPCodeFile_RevF_100MHz_16Bit;
		  input >> DSPVarFile_RevF_100MHz_16Bit;
		  count ++;
		}
	      else if (line == "***Rev-F-12Bit-250MSPS***")
		{
		  input >> DSPCodeFile_RevF_250MHz_12Bit;
		  input >> DSPVarFile_RevF_250MHz_12Bit;
		  count ++;
		}
	      else if (line == "***Rev-F-14Bit-250MSPS***")
		{
		  input >> DSPCodeFile_RevF_250MHz_14Bit;
		  input >> DSPVarFile_RevF_250MHz_14Bit;
		  count ++;
		}
	      else if (line == "***Rev-F-16Bit-250MSPS***")
		{
		  input >> DSPCodeFile_RevF_250MHz_16Bit;
		  input >> DSPVarFile_RevF_250MHz_16Bit;
		  count ++;
		}
	      else if (line == "***Rev-F-12Bit-500MSPS***")
		{
		  input >> DSPCodeFile_RevF_500MHz_12Bit;
		  input >> DSPVarFile_RevF_500MHz_12Bit;
		  count ++;
		}
	      else if (line == "***Rev-F-14Bit-500MSPS***")
		{
		  input >> DSPCodeFile_RevF_500MHz_14Bit;
		  input >> DSPVarFile_RevF_500MHz_14Bit;
		  count ++;
		}
	    }
	}
    }
  
  input.close();
  input.clear();
  
  return true;  
}

std::vector<unsigned short> Detector::GetPXISlotMap() const
{
    std::vector<unsigned short> slotmap(PXISlotMap,PXISlotMap+NumModules);
    return slotmap;
}

bool Detector::IsBooted()
{
  if(booted) return true;
  else return false;
}

bool
Detector::BootSystem (unsigned short NumModules, unsigned short *PXISlotMap)
{
  booted = true; 
  
  int retval = 0;
  retval = Pixie16InitSystem (NumModules, PXISlotMap, 0);
  
  if (retval != 0) {
    cout << "PCI Pixie init has failed: " << retval << endl;
    return false;
  }

  //////////////////////////////////////////////////////////
  //  Read hardware variant information of each Pixie-16 module
  //////////////////////////////////////////////////////////
  for(unsigned short k=0; k<NumModules; k++) {
    retval = Pixie16ReadModuleInfo(k, &ModRev[k], &ModSerNum[k], &ModADCBits[k], &ModADCMSPS[k]);
    if (retval < 0)
      {
	cout<<"Reading hardware variant information failed in module "<<k<<", retval="<<retval<<endl;
	return -3;
      }
  }

  //////////////////////////////////////////////////////////
  //  Assign firmware files based on hardware variant information
  //  and then boot each Pixie-16 module
  //////////////////////////////////////////////////////////
	
  cout<<"Booting all Pixie-16 modules...\n";
	
  // Set Pixie16_Trig_FPGA_File to a dummy string since it is no longer in use - just a place holder for software compatibility
  strcpy(Pixie16_Trig_FPGA_File, "pixie16trigger.bin");

  // Select firmware and dsp files based on hardware variant
  for(unsigned short k=0; k<NumModules; k++) {
    switch (ModRev[k]) {
      case 11:
      case 12:
      case 13:
	strcpy(Pixie16_Com_FPGA_File, ComFPGAConfigFile_RevBCD);
	strcpy(Pixie16_SP_FPGA_File,  SPFPGAConfigFile_RevBCD);
	strcpy(Pixie16_DSP_Code_File, DSPCodeFile_RevBCD);
	strcpy(Pixie16_DSP_Var_File,  DSPVarFile_RevBCD);
	break;
      case 15:
	if ((ModADCBits[k] == 14) && (ModADCMSPS[k] == 100)) {
	  strcpy(Pixie16_Com_FPGA_File, ComFPGAConfigFile_RevF_100MHz_14Bit);
	  strcpy(Pixie16_SP_FPGA_File,  SPFPGAConfigFile_RevF_100MHz_14Bit);
	  strcpy(Pixie16_DSP_Code_File, DSPCodeFile_RevF_100MHz_14Bit);
	  strcpy(Pixie16_DSP_Var_File,  DSPVarFile_RevF_100MHz_14Bit);
	}
	else if ((ModADCBits[k] == 16) && (ModADCMSPS[k] == 100)) {
	  strcpy(Pixie16_Com_FPGA_File, ComFPGAConfigFile_RevF_100MHz_16Bit);
	  strcpy(Pixie16_SP_FPGA_File,  SPFPGAConfigFile_RevF_100MHz_16Bit);
	  strcpy(Pixie16_DSP_Code_File, DSPCodeFile_RevF_100MHz_16Bit);
	  strcpy(Pixie16_DSP_Var_File,  DSPVarFile_RevF_100MHz_16Bit);
	}
	else if ((ModADCBits[k] == 12) && (ModADCMSPS[k] == 250)) {
	  strcpy(Pixie16_Com_FPGA_File, ComFPGAConfigFile_RevF_250MHz_12Bit);
	  strcpy(Pixie16_SP_FPGA_File,  SPFPGAConfigFile_RevF_250MHz_12Bit);
	  strcpy(Pixie16_DSP_Code_File, DSPCodeFile_RevF_250MHz_12Bit);
	  strcpy(Pixie16_DSP_Var_File,  DSPVarFile_RevF_250MHz_12Bit);
	}
	else if ((ModADCBits[k] == 14) && (ModADCMSPS[k] == 250)) {
	  strcpy(Pixie16_Com_FPGA_File, ComFPGAConfigFile_RevF_250MHz_14Bit);
	  strcpy(Pixie16_SP_FPGA_File,  SPFPGAConfigFile_RevF_250MHz_14Bit);
	  strcpy(Pixie16_DSP_Code_File, DSPCodeFile_RevF_250MHz_14Bit);
	  strcpy(Pixie16_DSP_Var_File,  DSPVarFile_RevF_250MHz_14Bit);
	}
	else if ((ModADCBits[k] == 16) && (ModADCMSPS[k] == 250)) {
	  strcpy(Pixie16_Com_FPGA_File, ComFPGAConfigFile_RevF_250MHz_16Bit);
	  strcpy(Pixie16_SP_FPGA_File,  SPFPGAConfigFile_RevF_250MHz_16Bit);
	  strcpy(Pixie16_DSP_Code_File, DSPCodeFile_RevF_250MHz_16Bit);
	  strcpy(Pixie16_DSP_Var_File,  DSPVarFile_RevF_250MHz_16Bit);
	}
	else if ((ModADCBits[k] == 12) && (ModADCMSPS[k] == 500)) {
	  strcpy(Pixie16_Com_FPGA_File, ComFPGAConfigFile_RevF_500MHz_12Bit);
	  strcpy(Pixie16_SP_FPGA_File,  SPFPGAConfigFile_RevF_500MHz_12Bit);
	  strcpy(Pixie16_DSP_Code_File, DSPCodeFile_RevF_500MHz_12Bit);
	  strcpy(Pixie16_DSP_Var_File,  DSPVarFile_RevF_500MHz_12Bit);
	}
	else if ((ModADCBits[k] == 14) && (ModADCMSPS[k] == 500)) {
	  strcpy(Pixie16_Com_FPGA_File, ComFPGAConfigFile_RevF_500MHz_14Bit);
	  strcpy(Pixie16_SP_FPGA_File,  SPFPGAConfigFile_RevF_500MHz_14Bit);
	  strcpy(Pixie16_DSP_Code_File, DSPCodeFile_RevF_500MHz_14Bit);
	  strcpy(Pixie16_DSP_Var_File,  DSPVarFile_RevF_500MHz_14Bit);
	}
	break;
      default:
	cout<<"Wrong revision information in module "<<k<<", revision="<<ModRev[k]<<"\n\n";
	return -4;
    }
  
    cout<<"\nBooting Pixie-16 module #"<<k<<", Rev="<<ModRev[k]<<", S/N="<<ModSerNum[k]<<", Bits="<<ModADCBits[k]<<", MSPS="<<ModADCMSPS[k]<<endl;
    cout << "ComFPGAConfigFile:  " << Pixie16_Com_FPGA_File << endl;
    cout << "SPFPGAConfigFile:   " << Pixie16_SP_FPGA_File << endl;
    cout << "DSPCodeFile:        " << Pixie16_DSP_Code_File << endl;
    cout << "DSPVarFile:         " << Pixie16_DSP_Var_File << endl;
    cout << "--------------------------------------------------------\n\n";
	
    retval = Pixie16BootModule (Pixie16_Com_FPGA_File, 
				// Name of communications FPGA config. file
				Pixie16_SP_FPGA_File, 
			      // Name of signal processing FPGA config. file
				Pixie16_Trig_FPGA_File, 
			      // placeholder name of trigger FPGA configuration file
				Pixie16_DSP_Code_File, 
			      // Name of executable code file for digital 
			      // signal processor (DSP)
				DSPParFile,           // Name of DSP parameter file
				Pixie16_DSP_Var_File, // Name of DSP variable names file
				k,                    // Pixie module number
				0x7F);                // Fast boot pattern bitmask


    if (retval != 0) {
      cout << "Cards booting has failed " << retval << " !\n";
      return false;
    }
  
  }

  cout << "Boot all modules ok " << endl;
  cout << "DSPParFile:        " << DSPParFile << endl;

  return true;
  
}

int Detector::Syncronise ()
{
  int retval = 0;
  char pIN_SYNCH[]="IN_SYNCH";
  retval = Pixie16WriteSglModPar (/*"IN_SYNCH"*/pIN_SYNCH, 0, 0);
  if (retval < 0)
    fprintf (stderr, "Failed to write IN_SYNCH");
  return retval;
}

int Detector::StartLSMRun (int continue_run)
{
  int retval = 0;
  char pSYNCH_WAIT[]="SYNCH_WAIT";
  retval = Pixie16WriteSglModPar (/*"SYNCH_WAIT"*/pSYNCH_WAIT, 1, 0);
  if (retval < 0)
    {
      fprintf (stderr, "Failed to write SYNCH_WAIT\n");
      return retval;
    }
  
  for (int count = 0; count < NumModules; count++) {
    if (continue_run == 1)
      retval = Pixie16StartListModeRun (count,0x100, NEW_RUN);
    else
      retval = Pixie16StartListModeRun (count, 0x100,RESUME_RUN);
    if (retval < 0)
      {
	fprintf (stderr, "Failed to start ListMode run in module");
	return retval;
      }
  }
  
  return 1;
}

int Detector::StartRun (int continue_run, int modnum)
{
  int retval = 0;
  int retval2 = 0;
  char pSYNCH_WAIT[]="SYNCH_WAIT";
  char pHOST_RT_PRESET[]="HOST_RT_PRESET";

  retval = Pixie16WriteSglModPar (/*"SYNCH_WAIT"*/pSYNCH_WAIT, 0, modnum);
  if (retval < 0) {
    fprintf (stderr, "Failed to write SYNCH_WAIT\n");
    return retval;
  }

  // Remove the preset run length in both module 0 and the current one
  retval = Pixie16WriteSglModPar( /*"HOST_RT_PRESET"*/pHOST_RT_PRESET, Decimal2IEEEFloating(99999), modnum);
  retval = Pixie16WriteSglModPar( /*"HOST_RT_PRESET"*/pHOST_RT_PRESET, Decimal2IEEEFloating(99999), 0);

  // First stop any runs that are still going ...
  for (int count = 0; count < NumModules; count++) {
    retval = Pixie16CheckRunStatus(count);
    if (retval != 0) {
      retval2 = Pixie16EndRun(count);
      if (retval2 != 0) {
	cout << "Failed to end run in module " << count << endl;
      } else {
	cout << "Run ended in module " << count << endl;
      }
    }
  }

  // Start list mode run
  // retval = Pixie16StartListModeRun(modnum, 0x100, NEW_RUN);
  // Start MCA run
  retval = Pixie16StartHistogramRun(modnum, NEW_RUN);

  if (retval < 0) {
    cout << "Failed to start Histogram run in module " << modnum
	 << endl;
    return retval;
  } else {
    cout << "Run started in module " << modnum << " = " 
	 << retval << endl;
    return retval;
  }
}
      
int Detector::StartBaselineRun (int continue_run, int modnum)
{
  int retval;
  retval = Pixie16AcquireBaselines (modnum);
  if(retval < 0){
    cout << "Failed to acquire baselines in module " << modnum << endl;
    return retval;
  } else {
    return retval;
  }

}

int Detector::RunStatus ()
{
  int *ret=new int[NumModules];
  int sum=0;
  
  for(int i=0;i<NumModules;i++) {
    ret[i]=0;
    ret[i]=Pixie16CheckRunStatus (i);
    sum=sum+ret[i];
  }

  return sum;
  
}

int Detector::Write2FileLSM (char *name)
{
  int *ret=new int[NumModules];
  int sum =0 ;
  for(int i=0;i<NumModules;i++) {    
    ret[i]=0;
//    ret[i]=Pixie16SaveListModeDataToFile (name, i);
    if (ret[i] < 0)
      std::cout << "failed to save to file block from mod "<<i<<" !\n";
    sum=sum+ret[i];
  }
  
  return sum;
}


int Detector::AcquireADCTrace (unsigned short *trace, unsigned long size,
			       unsigned short module, unsigned short ChanNum)
{ 
  int result;
  if(module < NumModules) {
    result = Pixie16AcquireADCTrace(module);
    if (result<0)
      return result;
    result = Pixie16ReadSglChanADCTrace(trace,	// trace data
					size,	// trace length
					module,	// module number
					ChanNum);
    
      
  } else {
    std::cout<<"Wrong module number: "<<module<<std::endl; 
    return -1000;
  }
  
  return result;
}


