#ifndef MAIN_H_
#define MAIN_H_

#define INITIAL_HEIGHT 500
#define INITIAL_WIDTH 600
#define MCA_SIZE 32768

#include <TGFrame.h>
#include <TGMenu.h>
#include <TGApplication.h>
#include <TApplication.h>
#include <TGClient.h>
#include <TGTab.h>
#include <TGButton.h>
#include <TGNumberEntry.h>
#include <TGLabel.h>
#include <TGTextEntry.h>
#include <TRootEmbeddedCanvas.h>
#include <TH1S.h>
#include <TH1F.h>
#include <TH1D.h>
#include <TGraph.h>
#include <TMultiGraph.h>
#include <TF1.h>
#include <TSystem.h>
#include <TStyle.h>
#include <TGFileDialog.h>
#include <TCanvas.h>
#include <iostream>
#include "PulseShape.h"
#include "EnergyFilter.h"
#include "AnalogSignal.h"
#include "TGStatusBar.h"
#include "Detector.h"
#include "Csra.h"
#include "ModCsrb.h"
#include "TrigConfig.h"
#include "TriggerFilter.h"
//#include "TriggerFilterOr.h"
#include "CFD.h"
//#include "ExtTrig.h"
#include "Baseline.h"
#include "Tau.h"
#include "MaxEvent.h"
#include "ExpertMod.h"
#include "Histogram.h"
#include "ScopedT.h"
#include "Popup.h"
//#include "MyTCanvas.h"
//#include "TROOT.h"
//#include "MyTH1S.h"
#include "TVirtualPad.h"
#include "TObject.h"
#include "lock.h"
#include "MultCoincDialogue.h"
#include "TimingParamDialogue.h"

using namespace std;
enum Commands //commands for the menu bar popups

{

	FILE_EXIT,
	FILE_OPEN,
	BOOT_BUTTON,
	ABOUT,
	READ_WF,
	ANLY_WF,
	MULT_DISPLAY,
	FAST_DISPLAY,
	SAVE_SEC,
	MODULE_NUMBER,
	MODULE_NUMBER_MCA,
	CHANNEL_NUMBER,
	CHANNEL_NUMBER_MCA,
	ASG,
	BASELINE,
	EFILTER,
	TFILTER,
//	EXTTRIG,
//	TFILTEROR,
	CFDS,
	CSRA,
	PULSE,
	DECAY,
	MAXEVENT,
	FILE_SAVE,
	MODVAR,
	CHANVAR,
	CLEARMCA,
	SAVEMCA,
	REFRESHMCA,
	STOPMCA,
	STARTMCA,
	CONTINUEMCA,
	MCACHECKBUTTON,
	HISTOGRAM,
	MAXMIN,
	FIND_WF,
	SCOPEDT,
	MODCSRB,
	TRIGCONFIG0,
	FINDTAU,
	SAVEHISTO,
	STARTRUN,
	BASELINERUN,
	ENDRUN,
    MULT,
    TIMING
	
};

class Detector;

#ifndef __Main__
#define __Main__

class Main:public TGMainFrame
{
 public:
  Main();
  Main(const TGWindow *p);
  ~Main();
  
 private:
  lock *l; // the lock file ("/var/lock/ddas")
  
  /***** Variables *****/

  /* Other classes in NScope */
  PulseShape *pulseshape;
  Popup *about;
  Popup *CFDwarning;
  EnergyFilter *energyfilter;
  AnalogSignal *analogsignal;
  Baseline *baseline;
  Csra *csra;
  ModCsrb *modcsrb;
  TrigConfig *trigconfig;
  TriggerFilter *triggerfilter;
//  TriggerFilterOr *triggerfilteror;
  CFD *cfd;
//  ExtTrig *exttrig;
  Tau *tau;
  ExpertMod *expertmod;
  ScopedT *scopedt;
  Histogram *histogram;
  MaxEvent *maxevent;
  Detector *detector;
  MultCoincDialogue *mult_dialogue;
  TimingParamDialogue *timing_dialogue;
 
  /* ROOT objects */
  TGFileInfo fEventFileOpenInfo;
  TGTextEntry *StateMsgFold1;
  TGTextButton *analyzeB, *acquireB, *saveB;
  TGCheckButton *bmultdisplay, *bfastdisplay;
  TGStatusBar* fStatusBar;
  TGNumberEntry *numericMod, *numericCh;
  
  TH1S  *ftrace_values;
  TH1S  *ftrace;
  TH1S  *fHpx_wave;
  TH1D  *fhisto;
  TH1S  *ffastfilt;
  TH1D  *fenerfilt;
  TH1S  *fcfdfilt;
  TH1S  *taufitHist;
  TCanvas *dCanvasF1;

  TObject *selected;

  Pixel_t color;


  /* Just numbers... */
  int xmin, xmax, ymin, ymax;
  int moduleNr; // Module number we're looking at
  unsigned short NumModules; //total number of modules in system
  int channelNr; // Channel number we're looking at
  int size; // Trace length in scope mode
  int range, separation; // Filter parameters
  float fraction; // Filter parameter
  int NUMBERofTRACES; 
  int RUN_IN_PROGRESS;
  int parts[4];
  unsigned long mca[MCA_SIZE];
  short int modNumber, chanNumber;
  bool wave_once;
  bool hpx_once_wave;

  double *decayconstant;
  double *decayconstantfit;
 
  unsigned int *histdata;
  unsigned short *trace, *filter_trace;
  float *trace_float;
  int *fastfilter, *enerfilter, *cfdfilter;

  string dirname;

  /***** Functions *****/

  virtual Bool_t ProcessMessage(Long_t msg, Long_t parm1, Long_t);
                                                   // Process message queue
  void CloseWindow(); // Close main window
  void CreateMenuBar(void); // Creates menu bar of the main window
  void SetStatusText(const char *txt, Int_t pi);
  void EventInfo(Int_t event, Int_t px, Int_t py, TObject *selected);
  void MakeFold1Panel(TGCompositeFrame *TabPanel);
  void PlotTau ();
  void NewTrace(unsigned long size, unsigned short module,
		unsigned short ChanNum);
  void GetHistogram(unsigned short module, unsigned short ChanNum);
  void GetBaseline(unsigned short int module, unsigned short int ChanNum);
  void AnlyTrace(unsigned long size, unsigned short module,
		 unsigned short ChanNum);
  void writeSpe(const char *filename, float buf[], int dim);
  void writeSpect(const char *filename, unsigned long buf[], int dim);
  void save_setup(char *name);
  int  IdentifyTracePulse (unsigned short *trace, unsigned int traceSize,
			   unsigned int trigLen, unsigned int trigGap,
			   double *trigLeadSum, double *trigTrailSum,
			   double trigThresho, unsigned int *peak,
			   unsigned int *valley);
  int  TauFromMoments (unsigned short *trace, unsigned int traceSize, 
		       double dt, double *tau);
  int  TauFromFit (unsigned short *trace, unsigned int traceSize, 
		   double dt, double *tau);
  int  BinTrace (double *trace, unsigned int traceSize, double *bins,
		 unsigned int binNum, double *binCounts);
  int  BinTraceFit (double *trace, double *tracefit, unsigned int traceSize,
		    double *bins, unsigned int binNum, double *binCounts);
  double ArrayMax (double *a, unsigned int ArraySize, unsigned int *Index);
  double FitGaussian (TGraph *Dist);
  int  FindTau (unsigned short ModNum, unsigned short ChanNum, 
		double *Tau, double *TauFit);
  
};

#endif

void DynamicExec();

#endif /*MAIN_H_*/
