#include "Detector.h"
#include "SystemBooter.h"

#include <string.h>
#include <limits>
#include <iostream>
#include <fstream>

using namespace std;
using namespace DAQ::DDAS;
namespace HR = DAQ::DDAS::HardwareRegistry;

Detector::Detector()
{
    m_newset = false;
    booted = false;
}

Detector::~Detector()
{
}

void Detector::Boot()
{
    std::string setFilePath;
    // make sure that if the user set a new settings file via File > Open...
    // the user's selection gets honored.
    if (m_newset) {
        setFilePath = m_config.getSettingsFilePath();
    }

    m_config = *(Configuration::generate(FIRMWARE_FILE, "cfgPixie16.txt"));

    if (m_newset) {
        m_config.setSettingsFilePath(setFilePath);
        m_newset = false;
    }

    SystemBooter booter;

    if (getenv("DDAS_BOOT_WHEN_REQUESTED") == NULL) {
        booter.boot(m_config, SystemBooter::FullBoot);
    } else {
        booter.boot(m_config, SystemBooter::SettingsOnly);
    }

    // the hardware map is set up during boot time
    NumModules = m_config.getNumberOfModules();
    std::vector<int> hdwrMap = m_config.getHardwareMap();
    for (size_t i=0; i<hdwrMap.size(); ++i) {
        HR::HardwareSpecification spec = HR::getSpecification(hdwrMap.at(i));
        ModADCMSPS[i] = spec.s_adcFrequency;
        ModADCBits[i] = spec.s_adcResolution;
        ModRev[i]     = spec.s_hdwrRevision;
    }

    booted = true;

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
  m_newset = true;
  m_config.setSettingsFilePath(newsetfile);
  cout << "Using new set file " << m_config.getSettingsFilePath() << endl;
}


std::vector<unsigned short> Detector::GetPXISlotMap() const
{
    return m_config.getSlotMap();
}

bool Detector::IsBooted()
{
  if(booted) return true;
  else return false;
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

int Detector::synchronize(int modnum) {

    int retval = 0;
    char pSYNCH_WAIT[]="SYNCH_WAIT";

//    Syncronise();

    retval = Pixie16WriteSglModPar (/*"SYNCH_WAIT"*/pSYNCH_WAIT, 0, modnum);
    if (retval < 0) {
        fprintf (stderr, "Failed to write SYNCH_WAIT\n");
    }
    return retval;
}

int Detector::StartRun (int continue_run, int modnum)
{
  int retval = 0;
  int retval2 = 0;
  char pHOST_RT_PRESET[]="HOST_RT_PRESET";

  if (!booted) {
      throw std::runtime_error("Cannot start a run before the system has been booted");
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


