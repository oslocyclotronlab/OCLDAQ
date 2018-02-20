// The message for the About box
#define MADE "nscope v2.6.2 7/29/2013"

#include "Main.h"
#include "PulseShape.h"
#include "Detector.h"
#include "EnergyFilter.h"
#include "Csra.h"
#include "ModCsrb.h"
#include "TrigConfig.h"
#include "TriggerFilter.h"
#include "pixie16app_export.h"
#include "pixie16sys_export.h"
#include "pixie16app_common.h"
#include "Popup.h"
#include <math.h>
#include "TRint.h"
#include "TLegend.h"

const char *spec_types[] =
{ "Radware spectrum", "*.spe", "all files", "*.*", 0, 0 };

const char *filetypes[] =
{ "Set Files", "*.set", "all files", "*.*", 0, 0 };

const char *filetype[] = { "all files",  "*","set files",  "*.set",0,0};

//ClassImp(Main)
//extern void e();

Main::Main(const TGWindow * p) 
    : csra(0), mult_dialogue(0), timing_dialogue(0)
{
  // Initialize variables
  NumModules = 0;
  moduleNr = 0;
  channelNr = 0;
  size = 8192; // Scope trace size
  
  CreateMenuBar();
  SetWindowName("Pixie16 Oscilloscope");
  MapSubwindows();
  MapWindow();
  Resize(INITIAL_WIDTH, INITIAL_HEIGHT);
  
  fHpx_wave = NULL;
  fhisto = NULL;
  wave_once = false;
  histdata = NULL;
  trace = NULL;
  trace_float = NULL;
  fastfilter = NULL;
  enerfilter = NULL;
  cfdfilter = NULL;
  taufitHist = NULL;

  decayconstant = new double;
  decayconstantfit = new double;
  
  modNumber = 0;
  chanNumber = 0;
  AppendPad(); // foarte important

  xmin = 0;
  xmax = 8192;
  ymin = 0;
  ymax = 4096;
  NUMBERofTRACES = 500; // Number of traces to search for waveforms
  range = 480;
  separation = 480;
  fraction = 0.0002;
  l = NULL;

  RUN_IN_PROGRESS = 0;
  
  detector = new Detector();
  dirname = gSystem->pwd();
  cout << "current working directory " << gSystem->pwd() << endl;
}

Main::~Main()
{
  if (mult_dialogue!=0) { 
    mult_dialogue->DestroyWindow();
    mult_dialogue=0;
  }
  if (csra!=0) { 
    csra->DestroyWindow();
    csra=0;
  }
  if (timing_dialogue!=0) {
      timing_dialogue->DestroyWindow();
      timing_dialogue=0;
  }
  CloseWindow();
    
}

void Main::CreateMenuBar()
{
  TGMenuBar *MenuBar = new TGMenuBar (this, 1, 1, kHorizontalFrame);
  TGPopupMenu *MenuFile = new TGPopupMenu (fClient->GetRoot ());
  MenuFile->AddEntry("&Open", FILE_OPEN);
  MenuFile->AddEntry("E&xit", FILE_EXIT);
  MenuFile->AddSeparator();
  MenuFile->AddEntry("&About", ABOUT);
  MenuFile->Associate(this);
  MenuBar->AddPopup("&File", MenuFile, new TGLayoutHints 
		    (kLHintsTop | kLHintsLeft, 0, 0, 0, 0));
  AddFrame(MenuBar, new TGLayoutHints (kLHintsTop | kLHintsLeft | kLHintsExpandX, 
				       0, 0, 0, 0));
  
  TGPopupMenu *MenuSetup = new TGPopupMenu (fClient->GetRoot ());
  MenuSetup->AddEntry("A&nalog Signal Conditioning", ASG);
  MenuSetup->AddEntry("Ba&selines Setup", BASELINE);
  MenuSetup->AddEntry("&Energy Filter", EFILTER);
  MenuSetup->AddEntry("&Trigger Filter", TFILTER);
//  MenuSetup->AddEntry("Trigger Filter &OR", TFILTEROR); // SNL added
//  MenuSetup->AddEntry("External Trigge&r", EXTTRIG); // SNL added
  MenuSetup->AddEntry("CFD", CFDS); // SNL added
  MenuSetup->AddEntry("&CSRA", CSRA);
  MenuSetup->AddEntry("&Mult Coincidence", MULT);
  MenuSetup->AddEntry("Timing Controls", TIMING);
  MenuSetup->AddEntry("&Pulse Shape", PULSE);
  MenuSetup->AddEntry("&Decay Time", DECAY);
  MenuSetup->AddEntry("&Histogramming", HISTOGRAM);
  MenuSetup->AddSeparator();
  MenuSetup->AddEntry("Save2File", FILE_SAVE);
  MenuSetup->Associate(this);
  MenuBar->AddPopup("&UV_Setup", MenuSetup, new TGLayoutHints 
		    (kLHintsTop | kLHintsLeft, 0, 0, 0, 0));
  
  TGPopupMenu *MenuExpert = new TGPopupMenu (fClient->GetRoot ());
  MenuExpert->AddEntry("Module Variables", MODVAR);
  MenuExpert->AddEntry("&ModCsrb", MODCSRB);
  MenuExpert->AddEntry("&TrigConfig0", TRIGCONFIG0);
  // MenuExpert->AddEntry ("Channel Variables", CHANVAR);
  MenuExpert->AddEntry("&Find Tau", FINDTAU);
  MenuExpert->AddEntry("&Start Run", STARTRUN);  
  MenuExpert->AddEntry("&Start Baseline Run", BASELINERUN);  
  MenuExpert->Associate(this);
  MenuBar->AddPopup("&Expert", MenuExpert, new TGLayoutHints 
		    (kLHintsTop | kLHintsLeft, 0, 0, 0, 0));
  
  TGPopupMenu *MenuScope = new TGPopupMenu (fClient->GetRoot ());
  MenuScope->AddEntry("xy maxmin", MAXMIN);
  MenuScope->AddEntry("dT", SCOPEDT);
  MenuScope->Associate(this);
  MenuBar->AddPopup("&Scope", MenuScope, new TGLayoutHints 
		    (kLHintsTop | kLHintsLeft, 0, 0, 0, 0));
  
  TGTab *TabPanel = new TGTab (this);
  this->AddFrame(TabPanel, new TGLayoutHints (kLHintsBottom | kLHintsExpandX |
					      kLHintsExpandY, 0, 0, 0, 0));
  TGCompositeFrame *Tab1 = TabPanel->AddTab("");
  
  MakeFold1Panel(Tab1);
}

void Main::CloseWindow()
{
  gApplication->Terminate(0);
}

Bool_t Main::ProcessMessage(Long_t msg, Long_t parm1, Long_t parm2)
{
  int test1 = 0, test2 = 0;
  switch (test1 = GET_MSG(msg))
    {
      
    case kC_COMMAND:
      switch (test2 = GET_SUBMSG(msg))
	{
	  
	  /********** Menu popups **********/            
	case kCM_MENU:
	  switch (parm1)
	    {
	    case FILE_EXIT:
	      if (l != NULL)
		delete l;
		detector->ExitSystem();
	      CloseWindow();
	      break;
	      
	    case FILE_OPEN:
	      {
		//TGFileInfo fEventFileOpenInfo;
		static TString dir2(dirname.c_str());
		//TGFileInfo fInput2;
		//fInput2.fFileTypes = filetypes;
		fEventFileOpenInfo.fIniDir = StrDup(dir2);
		fEventFileOpenInfo.fFileTypes = filetypes;
		new TGFileDialog(fClient->GetRoot(),0, kFDOpen, &fEventFileOpenInfo);
		if(fEventFileOpenInfo.fFilename != NULL){
		  if(detector){
		    char *temp = fEventFileOpenInfo.fFilename;
		    detector->SetSetFile(temp);
		    cout << "File in main " << fEventFileOpenInfo.fFilename << endl;
		  }
		}
		if(detector->IsBooted()){
		  fClient->GetColorByName("red", color);
		  StateMsgFold1->SetTextColor(color, true);
		  StateMsgFold1->SetText("New .set - Reboot!");
		}
	      }
	      break;
	      
	    case ABOUT:
	      about=new Popup(fClient->GetRoot(),0,MADE);
	      break;
	      
	    case PULSE:
	      pulseshape = new PulseShape (fClient->GetRoot (), 0,
					   "Pulse Shape");
	      pulseshape->setModuleNumber(moduleNr);
	      pulseshape->load_info(moduleNr);
	      break;
	      
	    case EFILTER:
	      energyfilter = new EnergyFilter (fClient->GetRoot (), 0,
					       "Energy Filter");
	      energyfilter->setModuleNumber(moduleNr);
	      energyfilter->load_info(moduleNr);
	      break;
	      
	    case ASG:
	      analogsignal = new AnalogSignal (fClient->GetRoot (), 0,
					       "Analog Signal Setup", 2,16,13);
	      analogsignal->setModuleNumber(moduleNr);
	      analogsignal->load_info(moduleNr);
	      break;
	    
        case CSRA:
          if (csra==0)
              csra = new Csra (fClient->GetRoot (), 0);
          else
              csra->MapWindow();

          if (mult_dialogue!=0) {
            csra->RegisterMultCoincDialogue(mult_dialogue);            
            mult_dialogue->RegisterCsra(csra);
          }

	      csra->SetModuleNumber(moduleNr);
	      csra->load_info(moduleNr,false);
	      break;
	    
	    case MULT:
          if (mult_dialogue==0)
              mult_dialogue = new MultCoincDialogue (fClient->GetRoot (),0, detector);
          else 
              mult_dialogue->MapWindow();
        
          if (csra!=0) {
            csra->RegisterMultCoincDialogue(mult_dialogue);            
            mult_dialogue->RegisterCsra(csra);
          } 
          // initialize the value of the CSRA alert if 
          // without the Csra dialogue
          mult_dialogue->UpdateCSRAState(Csra::GetNBitsOn(moduleNr,13));

          if (timing_dialogue!=0) {
            mult_dialogue->RegisterTimingParamDialogue(timing_dialogue);   
          }

	      mult_dialogue->SetModuleNumber(moduleNr);
	      break;

	    case MODCSRB:
	      modcsrb = new ModCsrb (fClient->GetRoot (), 0, NumModules);
	      break;
	    
	    case TRIGCONFIG0:
	      trigconfig = new TrigConfig(fClient->GetRoot (), 0, NumModules);
	    break;

	    case FINDTAU:
	      {
		FindTau(moduleNr, channelNr, decayconstant, decayconstantfit);
		cout << "Tau (max) from moments = " << *decayconstant << endl;
		cout << "Tau (max) from fits =    " 
		     << *decayconstantfit << endl;
		//int retval;
		*decayconstant = 0;
		//retval = Pixie16TauFinder (moduleNr, channelNr, decayconstant);
		std::cout << "Disable Pixie16 TauFinder due to API changes between firmware versions"
			  << std::endl;
		//if(retval<0) std::cout << "Tau finder fail in main" << std::endl;
		cout << "Tau from Pixie = " << *decayconstant << endl;
	      }
	      break;

	      
	    case STARTRUN:
	      {
		detector->StartRun(0, moduleNr);
		RUN_IN_PROGRESS = 1;
		cout << "starting run in module " << moduleNr << endl;
		acquireB->SetText("End Run");
		analyzeB->SetText("Read Histo");
		saveB->SetText("  Save  ");
	      }	     
	      break;

	    case BASELINERUN:
	      {
		acquireB->SetText("DO NOT PRESS");
		analyzeB->SetText("DO NOT PRESS");
		RUN_IN_PROGRESS = 2;
		int val = detector->StartBaselineRun(0, moduleNr);
		if(val == 0){
		  analyzeB->SetText("Get Basline");
		  acquireB->SetText("End Baseline");
		}
		
		saveB->SetText("  Save  ");
	      }	     
	      break;

	    case TFILTER:
	      triggerfilter = new TriggerFilter (fClient->GetRoot (), 0,
						 "Trigger Filter");
	      triggerfilter->SetModuleNumber(moduleNr);
	      triggerfilter->load_info(moduleNr);
	      break;
	      
//	    case TFILTEROR:
//	      triggerfilteror = new TriggerFilterOr (fClient->GetRoot (), 0,
//						     "Trigger Filter OR");
//	      triggerfilteror->SetModuleNumber(moduleNr);
//	      triggerfilteror->load_info(moduleNr);
//	      break;
	    
	    case CFDS:
	      
	      //the 500 MSPS modules do not pay attention to CFD parameters
	      if(detector->GetModuleMSPS(moduleNr) == 500){
		string WarnMsg = "The 500 MSPS modules to not accept user input for CFD paramters. Weight = 1, Delay = 5, B = 5, L = 1";
		CFDwarning = new Popup(fClient->GetRoot(),0,WarnMsg);
	      }
	      else {
		// the other modules are ok
		cfd = new CFD (fClient->GetRoot (), 0,
			       "CFD");
		cfd->SetModuleNumber(moduleNr);
		cfd->load_info(moduleNr);
	      }
	      break;
	    
//	    case EXTTRIG:
//	      exttrig = new ExtTrig(fClient->GetRoot (), 0,
//				    "Channel Trigger Delay and External Trig Width");
//	      exttrig->SetModuleNumber(moduleNr);
//	      exttrig->load_info(moduleNr);
//	      break;
	    
        case TIMING:
          if (timing_dialogue==0)
              timing_dialogue = new TimingParamDialogue(fClient->GetRoot (),0);
          else
            timing_dialogue->MapWindow();

	      timing_dialogue->SetModuleNumber(moduleNr);
	      timing_dialogue->load_info(moduleNr);

          if (mult_dialogue!=0) {
            mult_dialogue->RegisterTimingParamDialogue(timing_dialogue);
          }
	      break;

	    case BASELINE:
	      baseline = new Baseline (fClient->GetRoot (), 0,
				       "Baseline Setup");
	      baseline->SetModuleNumber(moduleNr);
	      baseline->load_info(moduleNr);
	      break;
	    
	    case DECAY:
	      tau = new Tau (fClient->GetRoot (), 0, "Decay Time");
	      tau->SetModuleNumber(moduleNr);
	      tau->load_info(moduleNr);
	      break;
	    
	    case MAXEVENT:
	      maxevent = new MaxEvent (fClient->GetRoot (), 0);
	      break;
	    
	    case MODVAR:
	      expertmod = new ExpertMod (fClient->GetRoot (), 0, "Expert MOD",13);
	      break;
	    
	    case HISTOGRAM:
	      histogram = new Histogram (fClient->GetRoot (), 0,
					 "Histogramming");
	      histogram->SetModuleNumber(moduleNr);
	      histogram->load_info(moduleNr);
	      break;
	    
	    case SCOPEDT:
	      scopedt = new ScopedT (fClient->GetRoot (), 0, "dT");
	      break;
	    
	    case FILE_SAVE:
	      {
		static TString dir2(dirname.c_str());
		TGFileInfo fInput2;
		fInput2.fFileTypes = filetypes;
		fInput2.fIniDir = StrDup(dir2);
		new
		  TGFileDialog (fClient->GetRoot (), 0, kFDSave,
				&fInput2);
		save_setup(fInput2.fFilename);
		//    cout<<"bingo\n\n\n";
	      }
	      break;
	      
	    default:
	      break;
	    }
	
	case kCM_BUTTON:
	  switch (parm1)
	    {
	    case BOOT_BUTTON:
	      if(detector->IsBooted()){
		detector->ExitSystem();
		delete detector;
		
		fClient->GetColorByName("red", color);
		StateMsgFold1->SetTextColor(color, true);
		StateMsgFold1->SetText("Not Booted");
		
	      } else {
		delete detector;
	      }
	      fClient->GetColorByName("red", color);
	      StateMsgFold1->SetTextColor(color, true);
	      StateMsgFold1->SetText("Booting ...");
	      gPad->SetCursor(kWatch);
	      //l = new lock("ddas");
	      gSystem->ChangeDirectory(dirname.c_str());
	      detector = new Detector ();
	      if(fEventFileOpenInfo.fFilename != NULL){
		detector->SetSetFile(fEventFileOpenInfo.fFilename);
	      }
	      detector->Boot();
	      NumModules = detector->GetNumberModules();
	      fClient->GetColorByName("blue", color);
	      StateMsgFold1->SetTextColor(color, true);
	      StateMsgFold1->SetText("System Booted");
	      gPad->SetCursor(kPointer);
	      break;
	    
	    case READ_WF:
	      {
		if (!RUN_IN_PROGRESS) {
		  NewTrace(size, moduleNr, channelNr);
		} else if (RUN_IN_PROGRESS >= 1) {
		  if(RUN_IN_PROGRESS ==1){
		    cout << "ending run in module " << moduleNr << endl;
		    int retval, i=0;
		    retval = Pixie16CheckRunStatus(moduleNr);
		    
		    while ((retval != 0) && (i<10)) {
		      retval = Pixie16EndRun(moduleNr);
		      sleep(1);
		      retval = Pixie16CheckRunStatus(moduleNr);
		      cout << "Run status " << i << " " << retval << endl;
		      i++;
		    }
		    retval = Pixie16CheckRunStatus(moduleNr);
		    if (retval == 0) {
		      cout << "Run ended in module " << moduleNr << endl;
		    } else { 
		      cout << "Run not ended in module " << moduleNr << endl;
		    }

		    //retrieve statistics from module
		    unsigned int Statistics[448];
		    retval = Pixie16ReadStatisticsFromModule(Statistics,moduleNr);
		    if(retval < 0) cout << "failed to read statistics, mod " 
					<< moduleNr << endl;
		    double ICR, OCR, livetime, realtime;
		    for(int z=0;z<16;z++){
		      ICR = Pixie16ComputeInputCountRate(Statistics,moduleNr,z);
		      OCR = Pixie16ComputeOutputCountRate(Statistics,moduleNr,z);
		      livetime = Pixie16ComputeLiveTime(Statistics,moduleNr,z);
		      realtime = Pixie16ComputeRealTime(Statistics,moduleNr);
		      cout << "Mod: " << moduleNr << " Chan: " <<z 
			   << " input rate: " << ICR << " output rate: " << OCR
			   << " livetime: " << livetime / realtime << endl;
		    }

		    acquireB->SetText("Read WF");
		    analyzeB->SetText("Analyze WF");
		    saveB->SetText("  Save  ");
		    RUN_IN_PROGRESS = 0;
		  }
		  if(RUN_IN_PROGRESS == 2){
		    acquireB->SetText("Read WF");
		    analyzeB->SetText("Analyze WF");
		    RUN_IN_PROGRESS = 0;
		  }
		}
	      }
	      break;
	    
	    case ANLY_WF:
	      {
		if (!RUN_IN_PROGRESS) {
		  AnlyTrace(size, moduleNr, channelNr);
		} else if (RUN_IN_PROGRESS >=1) {
		  if(RUN_IN_PROGRESS == 1)
		    GetHistogram(moduleNr, channelNr);
		  if(RUN_IN_PROGRESS == 2)
		    GetBaseline(moduleNr, channelNr);
		}
	      }
	      break;
	    
	    case SAVE_SEC:
	      {
		cout << "Saving to file\n" << flush;
		static TString dir1(".");
		TGFileInfo fInput1;
		fInput1.fFilename = NULL;
		fInput1.fFileTypes = spec_types;
		fInput1.fIniDir = StrDup(dir1);
		new TGFileDialog (fClient->GetRoot (), 0, kFDSave,
				  &fInput1);
		if (fInput1.fFilename != NULL) {
		  if (trace_float != NULL) {
		    delete trace_float;
		  }
		  if (RUN_IN_PROGRESS == 1) {
		    unsigned long *mca_data = new unsigned long[MCA_SIZE];
        for (int i = 0 ; i < MCA_SIZE ; ++i)
          mca_data[i] = fhisto->GetBinContent(i);
        writeSpect(fInput1.fFilename, mca_data, MCA_SIZE);
	 	  } else {
		    trace_float = new float[size];	
		    for (int i = 0; i < size; i++) {
		      trace_float[i] = fHpx_wave->GetBinContent(i);
		    }
		    writeSpe(fInput1.fFilename, trace_float, size);
      }
		} else {
		  cout << "No file name entered !\n" << flush;
		}
	      }
	      break;
	    
	    case MODULE_NUMBER:
	      if (parm2 == 0) {
		if (moduleNr != detector->NumModules-1) {
		  ++moduleNr;
		  numericMod->SetIntNumber(moduleNr);
		}
	      } else {
		if (moduleNr != 0) {
		  if (--moduleNr == 0) {
		    moduleNr = 0;
		  }
		  numericMod->SetIntNumber(moduleNr);
		}
	      }
	      break;
		
	    case CHANNEL_NUMBER:
	      if (parm2 == 0) {
		if (channelNr != 15) {
		  ++channelNr;
		  numericCh->SetIntNumber(channelNr);
		}
	      } else {
		if (channelNr != 0) {
		  if (--channelNr == 0) {
		    channelNr = 0;
		  }
		  numericCh->SetIntNumber(channelNr);
		}
	      }
	      break;

	      break;

	    }
	}
    
    case kC_TEXTENTRY:
      switch (parm1)
	{
	case MODULE_NUMBER:
	  switch (GET_SUBMSG(msg))
	    {
	    case kTE_ENTER:
	      moduleNr = numericMod->GetIntNumber();
	      numericMod->SetIntNumber(moduleNr);
	      break;
	    default:
	      break;
	    }
	case CHANNEL_NUMBER:
	  switch (GET_SUBMSG(msg))
	    {
	    case kTE_ENTER:
	      channelNr = numericCh->GetIntNumber();
	      numericCh->SetIntNumber(channelNr);
	      break;
	    default:
	      break;
	    }
	  break;
	default:
	  
	  break;
	}
      
      /********** Default for the most inclusive switch **********/          
    default:
      selected= gPad->GetSelected();
      break;
    }
  return kTRUE;
}

void Main::MakeFold1Panel(TGCompositeFrame * TabPanel)
{
  // Make the buttons frame        
  TGCompositeFrame *ButtonFrame = new TGCompositeFrame (TabPanel, 0, 0, 
							kHorizontalFrame);

  /********** BOOT button **********/    
  TGTextButton *bootB = new TGTextButton (ButtonFrame, "Boot", BOOT_BUTTON);
  bootB->SetFont("-adobe-helvetica-medium-r-*-*-12-*-*-*-*-*-iso8859-1", true);
  fClient->GetColorByName("blue", color);
  bootB->SetTextColor(color, true);
  bootB->Associate(this);
  ButtonFrame->AddFrame(bootB, new TGLayoutHints (kLHintsTop | kLHintsLeft, 5, 10,
						  10, 0));
  
  TabPanel->AddFrame(ButtonFrame, new TGLayoutHints (kLHintsTop | kLHintsLeft, 
						     0, 0, 0, 0));

  /********** Status frame and status TGTextEntry holder **********/
  TGGroupFrame *StateMsgFrame = new TGGroupFrame (ButtonFrame, "Status", 
						  kVerticalFrame);
  
  StateMsgFold1 = new TGTextEntry (StateMsgFrame,
				   new TGTextBuffer (30), 10000,
				   StateMsgFold1->GetDefaultGC ()(),
				   StateMsgFold1->GetDefaultFontStruct (),
				   kRaisedFrame | kDoubleBorder,
				   GetWhitePixel ());
  StateMsgFold1->
    SetFont("-adobe-helvetica-bold-r-*-*-10-*-*-*-*-*-iso8859-1", false);
  
  fClient->GetColorByName("red", color);
  StateMsgFold1->SetTextColor(color, true);
  StateMsgFold1->SetText("System not booted");
  StateMsgFold1->Resize(100, 12);
  StateMsgFold1->SetEnabled(kFALSE);
  StateMsgFold1->SetFrameDrawn(kFALSE);
  
  StateMsgFrame->AddFrame(StateMsgFold1, new TGLayoutHints 
			  (kLHintsTop | kLHintsLeft, 0, 0, 0, 0));
  ButtonFrame->AddFrame(StateMsgFrame, new TGLayoutHints 
			(kLHintsTop | kLHintsLeft, 2, 5, 1, 5));
  
  TGVerticalFrame *fRead = new TGVerticalFrame(ButtonFrame);


  /********** Acquire waveform button **********/
  acquireB = new TGTextButton (/*ButtonFrame*/fRead, "Read WF", READ_WF);
  fClient->GetColorByName("blue", color);
  acquireB->SetTextColor(color, true);
  acquireB->Associate(this);
  /*ButtonFrame*/fRead->AddFrame(acquireB, new TGLayoutHints 
			(kLHintsLeft | kLHintsTop, 3, 10, 10, 0));

  TGHorizontalFrame *fDisplay1 = new TGHorizontalFrame(fRead);

  bmultdisplay = new TGCheckButton(fDisplay1,"",MULT_DISPLAY);
  bmultdisplay->SetToolTipText("Show all 16 channels at once", 0);
  fDisplay1->AddFrame(bmultdisplay, new TGLayoutHints 
			(kLHintsLeft | kLHintsTop, 0, 0, 0, 0));
  TGLabel *ldisplay = new TGLabel(fDisplay1, "all chan");
  ldisplay->SetTextColor(color,true);
  fDisplay1->AddFrame(ldisplay, new TGLayoutHints 
			 (kLHintsLeft | kLHintsTop, 0, 0, 0, 0));
 

  TGHorizontalFrame *fDisplay2 = new TGHorizontalFrame(fRead);

  bfastdisplay = new TGCheckButton(fDisplay2,"",FAST_DISPLAY);
  bfastdisplay->SetToolTipText("Speed up trace display by displaying the first acquired waveform instead of searching for a trigger.", 0);
  fDisplay2->AddFrame(bfastdisplay, new TGLayoutHints 
			(kLHintsLeft | kLHintsTop, 0, 0, 0, 0));
  TGLabel *lbfastdisplay = new TGLabel(fDisplay2, "fast");
  lbfastdisplay->SetTextColor(color,true);
  fDisplay2->AddFrame(lbfastdisplay, new TGLayoutHints 
			 (kLHintsLeft | kLHintsTop, 0, 0, 0, 0));
  
  fRead->AddFrame(fDisplay1, new TGLayoutHints 
			(kLHintsTop | kLHintsLeft, 0, 0, 0, 0));
  fRead->AddFrame(fDisplay2, new TGLayoutHints 
			(kLHintsTop | kLHintsLeft, 0, 0, 0, 0));
  ButtonFrame->AddFrame(fRead, new TGLayoutHints 
			(kLHintsTop | kLHintsLeft, 0, 0, 0, 0));

  /********** Analyze waveform button **********/
  analyzeB = new TGTextButton (ButtonFrame, "Analyze WF", ANLY_WF);
  fClient->GetColorByName("blue", color);
  analyzeB->SetTextColor(color, true);
  analyzeB->Associate(this);
  ButtonFrame->AddFrame(analyzeB, new TGLayoutHints 
			(kLHintsLeft | kLHintsTop, 3, 10, 10, 0));

  /********** Save waveform button **********/
  saveB = new TGTextButton (ButtonFrame, "  Save  ", SAVE_SEC);
  saveB->Associate(this);
  fClient->GetColorByName("blue", color);
  saveB->SetTextColor(color, true);
  saveB->SetToolTipText("Save waveform to radware .sec file", 0);
  ButtonFrame->AddFrame(saveB, new TGLayoutHints 
			(kLHintsTop | kLHintsLeft, 0, 10, 10, 0));
  
    /********** Labels **********/
  TGVerticalFrame *fModChLabels = new TGVerticalFrame (ButtonFrame);
  
  TGLabel *mod = new TGLabel (fModChLabels, "Module #:");
  mod->SetTextColor(color, true);
  fModChLabels->AddFrame(mod, new TGLayoutHints 
			 (kLHintsLeft | kLHintsTop, 10, 3, 4, 0));
  TGLabel *ch = new TGLabel (fModChLabels, "Channel #:");
  fModChLabels->AddFrame(ch, new TGLayoutHints (kLHintsLeft, 10, 3, 4, 0));
  ButtonFrame->AddFrame(fModChLabels, new TGLayoutHints 
			(kLHintsTop | kLHintsLeft, 0, 0, 0, 0));

  /********** Number entries **********/
  TGVerticalFrame *fModCh = new TGVerticalFrame (ButtonFrame);
  numericMod = new TGNumberEntry (fModCh, 0, 4, MODULE_NUMBER, // Module # entry
				  (TGNumberFormat::EStyle) 0, 
				  (TGNumberFormat::EAttribute) 1, 
				  (TGNumberFormat::ELimit) 3, // kNELLimitMinMax
				  0, 3);
  numericMod->SetButtonToNum(0);
  fModCh->AddFrame(numericMod, new TGLayoutHints 
		   (kLHintsTop | kLHintsLeft, 2, 3, 0, 0));
  numericMod->Associate(this);
  
  numericCh = new TGNumberEntry (fModCh, 0, 4, CHANNEL_NUMBER, // Channel # entry
				 (TGNumberFormat::EStyle) 0, 
				 (TGNumberFormat::EAttribute) 1, 
				 (TGNumberFormat::ELimit) 3, // kNELLimitMinMax
				 0, 3);
  numericCh->SetButtonToNum(0);
  
  fModCh->AddFrame(numericCh, new TGLayoutHints 
		   (kLHintsTop | kLHintsLeft, 2, 3, 0, 0));

  numericCh->Associate(this);
  ButtonFrame->AddFrame(fModCh, new TGLayoutHints 
			(kLHintsTop | kLHintsLeft, 0, 0, 0, 0));
  
  /********** Canvas **********/                                  
  TGCompositeFrame *CanvasFrame = new TGCompositeFrame (TabPanel, 60, 60, 
							kHorizontalFrame);
  TGLayoutHints *Hint = new TGLayoutHints (kLHintsExpandX | kLHintsExpandY, 
					   1, 1, 1, 1);
  
  TRootEmbeddedCanvas *eCanvasF1 = new TRootEmbeddedCanvas ("ec1", CanvasFrame, 
							    100, 100);
  
  dCanvasF1 = eCanvasF1->GetCanvas();
  
  dCanvasF1->SetBorderMode(0); // No red frame
  CanvasFrame->AddFrame(eCanvasF1, Hint);
  TabPanel->AddFrame(CanvasFrame, Hint);
  
  parts[0] = 33;
  parts[1] = 10;
  parts[2] = 10;
  parts[3] = 47;
  
  fStatusBar = new TGStatusBar (TabPanel, 10, 10);
  fStatusBar->SetParts(parts, 4);
  
  TabPanel->AddFrame(fStatusBar, new TGLayoutHints (kLHintsBottom | 
						    kLHintsLeft |	
						    kLHintsExpandX, 2, 
						    2, 1, 1));
}

void Main::NewTrace(unsigned long size, unsigned short module,
		    unsigned short ChanNum)
{

  double retval = 0;
  double dt = -1;
  char pXDT[]="XDT";
  retval = Pixie16ReadSglChanPar(/*"XDT"*/pXDT, &dt, module, ChanNum); /* Time between ADC samples */
  if(retval<0) cout << "failed to read XDT" << endl;

  int multdisplay = 0;
  if(bmultdisplay->IsDown()) multdisplay = 1;

  int ntrace;
  ntrace = NUMBERofTRACES;
  if(bfastdisplay->IsDown()) ntrace = 1;

  if (trace == NULL) {
    trace = new unsigned short[size];
  }
  unsigned short start, stop;

  dCanvasF1->Clear();

  if(multdisplay == 0) {
    start = ChanNum;
    stop = ChanNum+1;
  }
  else{
    start = 0;//ChanNum;
    stop = 16;//ChanNum+1;
    //cout << "dividing " << endl;
    dCanvasF1->Divide(4,4);
    //cout <<  "divided " << endl;
    for(int z=0;z<16;z++) {
      //cout << " draw pad " << z << endl;
      dCanvasF1->GetPad(z+1)->Draw();
    }
    dCanvasF1->Modified();
    dCanvasF1->Update();
    //TH1D *h1 = new TH1D("h1","h1",100,0,100);
    for(int z=0;z<16;z++){
      //cout << "getting pad " << z << endl;
      //dCanvasF1->GetPad(z+1)->cd();
    //   //dCanvasF1->cd(z);
      //cout << "drawing pad" << endl;
      //h1->DrawCopy();
      //cout << "draw histo " << endl;
    }
  }

  //std::cout << "start stop " << ChanNum << " " << start << " " << stop << std::endl; 
  //stop = 0;

  //loop over appropriate number of channels
  for(int ch = start; ch<stop; ch++){

    ymax = 0;
    ymin = 4096;

    memset(trace, 0, size * sizeof(unsigned short));

    if (fHpx_wave == NULL) {
      fHpx_wave = new TH1S ("hpx_wave", "Live trace", size, 0, size);
    }

    int m=0;
    long average = 0;
    int goodtrace = 0;

    do {
      memset(trace, 0, size * sizeof(unsigned short));
      detector->AcquireADCTrace(trace, size, module, /*ChanNum*/ch);
      average = 0;
      for (unsigned int j=0; j < size; j++) {
        average += trace[j];
      }
      average = average/size;
      for (unsigned int j=0; j < size; j++) {
        if ((trace[j] > average+75) || 
            (trace[j] < average-75)) {
          goodtrace = 1;
        } 
      }
      m++;
    } while ((goodtrace!=1) && (m < ntrace));

    //gStyle->SetOptStat(0);

    fHpx_wave->Reset();
    for (unsigned long i = 0; i < size; i++) {
      fHpx_wave->Fill(i, trace[i]);
      if (trace[i] > ymax) {
        ymax = trace[i];
      }
      if (trace[i] < ymin) {
        ymin = trace[i];
      }
    }

    int signalheight = (int)((ymax - ymin)*(0.25));
    ymax += signalheight;
    ymin -= signalheight;

    wave_once = true;
    fHpx_wave->SetMaximum(ymax);
    fHpx_wave->SetMinimum(ymin);
    fHpx_wave->SetAxisRange(0, 8191);



    if(multdisplay==0){
      dCanvasF1->cd();
      fHpx_wave->GetYaxis()->SetTitle("ADC number");
      fHpx_wave->GetYaxis()->SetTitleSize(0.05);
      fHpx_wave->GetYaxis()->SetTitleOffset(1.);
      fHpx_wave->GetYaxis()->CenterTitle();
      fHpx_wave->GetYaxis()->SetNdivisions(505);
      fHpx_wave->GetXaxis()->SetTitle(Form("Time (%.0f ns Bin Width)",dt*1000));
      fHpx_wave->GetXaxis()->CenterTitle();
      fHpx_wave->GetXaxis()->SetTitleSize(0.05);
      fHpx_wave->GetXaxis()->SetTitleOffset(1.0);
      fHpx_wave->GetXaxis()->SetNdivisions(505);
      fHpx_wave->GetXaxis()->CenterTitle();
      fHpx_wave->GetYaxis()->SetLabelSize(0.05);
      fHpx_wave->GetXaxis()->SetLabelSize(0.05);

      // Draw the histogram but do not draw the error bars
      fHpx_wave->DrawCopy("hist c");
    }
    else {
      dCanvasF1->GetPad(ch+1)->cd();
      fHpx_wave->GetYaxis()->CenterTitle();
      fHpx_wave->GetYaxis()->SetNdivisions(505);
      fHpx_wave->GetXaxis()->CenterTitle();
      fHpx_wave->GetXaxis()->SetNdivisions(505);
      fHpx_wave->GetXaxis()->CenterTitle();
      fHpx_wave->GetYaxis()->SetTitle("");
      fHpx_wave->GetXaxis()->SetTitle("");
      fHpx_wave->GetYaxis()->SetTitleSize(0.08);
      fHpx_wave->GetYaxis()->SetLabelSize(0.08);
      fHpx_wave->GetXaxis()->SetTitleSize(0.08);
      fHpx_wave->GetXaxis()->SetLabelSize(0.08);
      //dCanvasF1->GetPad(ch)->Draw();
      //cout << " mult channel display " << ch << endl;

      // draw the histogram but do not draw the error bars
      fHpx_wave->DrawCopy("hist c");
      dCanvasF1->Modified();
      dCanvasF1->Update();
    }

  }

  dCanvasF1->Modified();
  dCanvasF1->Update();

  gSystem->ProcessEvents();

}

void Main::GetHistogram(unsigned short module, unsigned short ChanNum)
{
  
  if (histdata == NULL) {
    histdata = new unsigned int[MCA_SIZE];
  }
  
  ymax = 0;
  ymin = 4096;
  
  memset(histdata, 0, MCA_SIZE * sizeof(/*unsigned long*/uint32_t));
  int xmin=0,xmax=0;
  if (fhisto == NULL) {
    fhisto = new TH1D ("EHist", "Energy histogram", MCA_SIZE, 0, MCA_SIZE);
  } else {
    xmin = fhisto->GetXaxis()->GetXmin();
    xmax = fhisto->GetXaxis()->GetXmax();
  }
  
  int retval;
  retval = Pixie16ReadHistogramFromModule(histdata, MCA_SIZE, module, ChanNum);
  if (retval == 0) {
    cout << "Reading histogram from module " << module << ", channel "
	 << ChanNum << "..." << endl;
  } else {
    cout << "Error reading histogram from module " << module << ", channel "
	 << ChanNum << " = " << retval << endl;
  }
  
  gStyle->SetOptStat(0);
  
  fhisto->Reset();
  for (int i = 0; i < MCA_SIZE; i++) {
    fhisto->Fill(i, histdata[i]);
  }

   


  dCanvasF1->Clear();
  dCanvasF1->cd();
  fhisto->DrawCopy();
  dCanvasF1->Modified();
  dCanvasF1->Update();

  if (xmin != xmax && xmax != 0)
    fhisto->GetXaxis()->SetRange(xmin, xmax);
  dCanvasF1->Update();
  gSystem->ProcessEvents();
}

void Main::GetBaseline(unsigned short int module, unsigned short int ChanNum)
{
  if (histdata == NULL) {
    histdata = new unsigned int[MCA_SIZE];
  }
  
  ymax = 0;
  ymin = 4096;
  
  memset(histdata, 0, MCA_SIZE * sizeof(/*unsigned long*/uint32_t));
  
  if (fhisto == NULL) {
    fhisto = new TH1D ("EHist", "Energy histogram", MCA_SIZE, 0, MCA_SIZE);
  }


  if(fhisto){
    fhisto->Clear();
  }

  double Baselines[3640], TimeStamps[3640];
  unsigned short NumWords = 3640;

  int retval;
  retval = Pixie16ReadSglChanBaselines(Baselines,TimeStamps,NumWords,module,ChanNum);
  if(retval < 0){
    cout << "failed reading baslines from module " << module << " chan " << ChanNum << endl;
  }

  for(int i=0; i<3640; i++){
    fhisto->Fill(Baselines[i]);
  }

  dCanvasF1->Clear();
  dCanvasF1->cd();
  fhisto->DrawCopy();
  dCanvasF1->Modified();
  dCanvasF1->Update();
  
  gSystem->ProcessEvents();

}


void Main::AnlyTrace(unsigned long size, unsigned short module,
		     unsigned short ChanNum)
{
  if (trace==0) {
    std::cout << "ERROR: No trace data to analyze." << std::endl;
    std::cout << "Press [Read WF] button, then try again." << std::endl;
    return;
  }

  //determine if there really are multiple pads before retrieving trace
  Int_t npads = 0;
  TObject *obj;
  TIter next(dCanvasF1->GetListOfPrimitives());
  while ((obj = next())) {
    if(obj->InheritsFrom(TVirtualPad::Class())) npads++;
  }

  //if there are no subdivided pads reset the all channel display button
  if(npads==0){
    bmultdisplay->SetState(kButtonUp);
  }
  else{
    for(unsigned int z=0; z<size; z++){
      trace[z] = ((TH1D*)(dCanvasF1->GetPad(ChanNum+1)->GetPrimitive("hpx_wave")))->GetBinContent(z+1);
    }
    
  }

  if (fastfilter == NULL) {
    fastfilter = new int[size];
  }
  if (enerfilter == NULL) {
    enerfilter = new int[size];
  }
  if (cfdfilter == NULL) {
    cfdfilter = new int[size];
  }

  memset(fastfilter, 0, size * sizeof(int));
  memset(enerfilter, 0, size * sizeof(int));
  memset(cfdfilter, 0, size * sizeof(int));

  // Get necessary parameters from Pixie16 to perform trace analysis
  // Overlay three separate traces: fast trigger, slow trigger, and CFD

  double retval = 0;
  double trigrise = -1;
  double trigflat = -1;
  double trigthresh = -1;
  double enerrise = -1;
  double enerflat = -1;
  double cfddelay = -1;
  double cfdscale = -1;
  double dt = -1;
  double tau = -1;

  char pTRIGGER_RISETIME[]="TRIGGER_RISETIME";
  char pTRIGGER_FLATTOP[]="TRIGGER_FLATTOP";
  char pTRIGGER_THRESHOLD[]="TRIGGER_THRESHOLD";
  char pENERGY_RISETIME[]="ENERGY_RISETIME";
  char pENERGY_FLATTOP[]="ENERGY_FLATTOP";
  char pCFDDelay[]="CFDDelay";
  char pCFDScale[]="CFDScale";
  char pXDT[]="XDT";
  char pTAU[]="TAU";
    
  retval = Pixie16ReadSglChanPar(/*"TRIGGER_RISETIME"*/pTRIGGER_RISETIME, &trigrise, module, ChanNum);
  retval = Pixie16ReadSglChanPar(/*"TRIGGER_FLATTOP"*/pTRIGGER_FLATTOP, &trigflat, module, ChanNum);
  retval = Pixie16ReadSglChanPar(/*"ENERGY_RISETIME"*/pENERGY_RISETIME, &enerrise, module, ChanNum);
  retval = Pixie16ReadSglChanPar(/*"ENERGY_FLATTOP"*/pENERGY_FLATTOP, &enerflat, module, ChanNum);
  retval = Pixie16ReadSglChanPar(/*"CFDDelay"*/pCFDDelay, &cfddelay, module, ChanNum);
  retval = Pixie16ReadSglChanPar(/*"CFDScale"*/pCFDScale, &cfdscale, module, ChanNum);
  retval = Pixie16ReadSglChanPar(/*"TAU"*/pTAU, &tau, moduleNr, channelNr);
  retval = Pixie16ReadSglChanPar(/*"XDT"*/pXDT, &dt, module, ChanNum); /* Time between ADC samples */

  retval = Pixie16ReadSglChanPar(pTRIGGER_THRESHOLD,&trigthresh, module,ChanNum);

  // std::cout << "trig thresh " << trigthresh << std::endl;

  if(retval < 0 ) std::cout << "reading pixie parameters failed in AnlyTrace in main" << std::endl;

  /* Pixie16 ADC samples are spaced by dt microseconds; filter values 
     are in units of microseconds -- we need in units of dt. */

  cout << "Filter parameters (From Modules):" << endl;
  cout << "    ADC sample deltaT (ns): " << dt*1000 << endl;
  cout << "    Fast rise (ns):         " << trigrise*1000 << endl;
  cout << "    Fast flat (ns):         " << trigflat*1000 << endl;
  cout << "    Energy rise (ns):       " << enerrise*1000 << endl;
  cout << "    Energy flat (ns):       " << enerflat*1000 << endl;
  cout << "    Tau (ns):               " << tau*1000 << endl;
  cout << "    CFD delay:              " << cfddelay*1000 << endl;
  cout << "    CFD scale:              " << cfdscale << endl;

  
  // Need to calculate the number of bins of width dT to use for calculating filters
  // Mapping filter rises and gaps to scope bins 
  if(fmod(trigrise,dt) != 0) {
    cout<<"Trigger Filter Rise Time is NOT an integer number of dT.  Fixing Now. "<<endl;
    trigrise = ceil(trigrise/dt);
  }
  else {
    trigrise = (trigrise)/dt;
  }

  if(trigflat !=0 && fmod(trigflat,dt) !=0) {
    cout<<"Trigger Filter Gap Time is NOT an integer numnber of dT. Fixing Now. "<<endl;
    trigflat = ceil(trigflat/dt);
  }
  else {
    trigflat = (trigflat)/dt;
  }

  if(fmod(enerrise,dt) != 0) {
    cout<<"Energy Filter Rise Time is NOT an integer number of dT.  Fixing Now. "<<endl;
    enerrise = ceil(enerrise/dt);
  }
  else {
    enerrise = (enerrise)/dt;
  }
  
  if(enerflat !=0 && fmod(enerflat,dt) !=0) {
    cout<<"Energy Filter Gap Time is NOT an integer numnber of dT. Fixing Now. "<<endl;
    enerflat = ceil(enerflat/dt);
  }
  else {
    enerflat = (enerflat)/dt;
  }
  
   
  if(fmod(cfddelay,dt) !=0) {
    cout<<"CFD Delay is NOT an integer numnber of dT. Fixing Now. "<<endl;
    cfddelay = ceil(cfddelay/dt);
  }
  else {
    cfddelay = (cfddelay)/dt;
  }
  
  cout << "Filter parameters used for ''Analyze Waveform'' "<<endl;
  cout << "Note: Parameters have NOT been altered for the aquisition" << endl;
  cout << "    Fast rise (ns):         " << trigrise*1000*dt << endl;
  cout << "    Fast flat (ns):         " << trigflat*1000*dt << endl;
  cout << "    Energy rise (ns):       " << enerrise*1000*dt << endl;
  cout << "    Energy flat (ns):       " << enerflat*1000*dt << endl;
  cout << "    CFD delay (ns):         " << cfddelay*1000*dt << endl;
  

  // cout<<dt<<"    "<<trigrise<<"  "<<trigflat<<"  "<<enerrise<<"  "<<enerflat<<"  "<<cfddelay<<endl;

  //Put tau in units of dT
  tau = tau/dt;

  // cout<<"Tau: "<<tau<<endl;
  
  //Calculated the relevant Energy filter matrix parameters
  double b1, a0, ag, a1;
  
  b1 = exp(-1/tau);
  a0 = pow(b1,enerrise)/(pow(b1,enerrise)-1.0);
  ag = 1.0;
  a1 = -1.0/(pow(b1,enerrise)-1.0);

  //  cout<<b1<<"   "<<a0<<"  "<<ag<<"  "<<a1<<endl;

  // Create filter histograms
  if (ftrace == NULL) 
    ftrace = new TH1S ("ftrace", "", size, 0, size);
  if (ffastfilt == NULL)
    ffastfilt = new TH1S ("ffastfilt", "", size, 0, size);
  if (fenerfilt == NULL)
    fenerfilt = new TH1D ("fenerfilt", "", size, 0, size);
  if (fcfdfilt == NULL)
    fcfdfilt = new TH1S ("fcfdfilt", "", size, 0, size);
  if (ftrace_values == NULL) 
    ftrace_values = new TH1S ("ftrace_values", "", 16384, 0,16384);

  ftrace_values->Reset();
  ftrace->Reset();
  ffastfilt->Reset();
  fenerfilt->Reset();
  fcfdfilt->Reset();
   
  double baseline=0;

  for (unsigned long i = 0; i < size; i++){
     ftrace_values->Fill(trace[i]);    
  }

  baseline = ftrace_values->GetMaximumBin()-1;
  cout<<"Baseline: "<<baseline<<endl;
  
  for (unsigned long i = 0; i < size; i++){
    
    // fill the trace histogram
    ftrace->Fill(i,trace[i]);
    
    // Compute fast trigger
    int sum1, sum2, sumg;
    sum1 = sum2 = sumg = 0;

    if((i - 2*trigrise - trigflat + 1) >= 0){
      for(unsigned int a = i - (int)trigrise + 1; a < i + 1; a++) {
	sum1 = sum1 + trace[a];
      }
      for(unsigned int a = i - 2*(int)trigrise - (int)trigflat + 1; 
	  a< i - (int)trigrise - (int)trigflat + 1; a++) {
	sum2 = sum2 + trace[a];
      }

      fastfilter[i] = sum1 - sum2;    
    }

    ffastfilt->Fill(i, fastfilter[i]);

    // Compute energy filter
    sum1 = sum2 = sumg = 0;

    if((i - 2*enerrise - enerflat + 1) >= 0){
      for(unsigned int a = i - (int)enerrise + 1; a < i + 1; a++) {
	sum1 = sum1 + trace[a];
      }
      
      for(unsigned int a = i - (int)enerrise - (int)enerflat + 1; a < i -(int)enerrise + 1; a++) {
	sumg = sumg + trace[a];
      }
            
      for(unsigned int a = i - 2*(int)enerrise - (int)enerflat + 1; a< i - (int)enerrise - (int)enerflat + 1; a++) {
	sum2 = sum2 + trace[a];
      }
      
      enerfilter[i] = a1*sum1 + ag*sumg + a0*sum2 - baseline*(a0*(int)enerrise + ag*(int)enerflat + a1*(int)enerrise);     
    }
    
    fenerfilt->Fill(i, enerfilter[i]);
    
  }

  unsigned int cfdfilterminx = size-1;
  unsigned int cfdfiltermaxx = 0;

  // Now that the fast filter exists compute CFD filter
  
  for (unsigned long i = 0; i < size; i++){
    
    if( ((i - 2*trigrise - trigflat - cfddelay) >= 0)){
      cfdfilter[i] = fastfilter[i] - fastfilter[i-(int)cfddelay]/pow(2.0,(cfdscale+1.0));
 
      if ((cfdfilter[i] < -50) || (cfdfilter[i] > 50)) {
	if (i < cfdfilterminx) {
	  cfdfilterminx = i;
	}
	if (i > cfdfiltermaxx) {
	  cfdfiltermaxx = i;
	}
      }
      
    }

    fcfdfilt->Fill(i, cfdfilter[i]);
  }

  int cfddx = cfdfiltermaxx - cfdfilterminx;
  int cfdx = (cfdfiltermaxx + cfdfilterminx)/2;

  int xmin = cfdx - (5*cfddx);
  int xmax = cfdx + (5*cfddx);

  if (xmin < 0) xmin = 0;
  if (xmax > 8191) xmax = 8191;

  dCanvasF1->Clear();

  gStyle->SetOptStat(0);
  dCanvasF1->Divide(1,3);
  dCanvasF1->cd(1);
  dCanvasF1->SetBorderMode(0);
  TLegend *leg_trace = new TLegend(0.75,0.75,0.85,0.85);
  ftrace->SetAxisRange(xmin, xmax);
  ftrace->GetYaxis()->SetTitle("Trace Amplitude (ADC Units)");
  ftrace->GetYaxis()->SetTitleSize(0.07);
  ftrace->GetYaxis()->SetTitleOffset(0.5);
  ftrace->GetYaxis()->CenterTitle();
  ftrace->GetYaxis()->SetLabelSize(0.07);
  ftrace->GetYaxis()->SetNdivisions(505);
  ftrace->GetXaxis()->SetLabelSize(0.07);
  ftrace->SetLineWidth(2);
  ftrace->DrawCopy("hist c");
  leg_trace->AddEntry("ftrace","Trace","l");
  leg_trace->SetFillColor(0);
  leg_trace->Draw();
  
  
  dCanvasF1->cd(2);
  TLegend *leg_tf = new TLegend(0.75,0.65,0.85,0.85);
  
  fcfdfilt->SetLineColor(2);
  fcfdfilt->SetLineWidth(2);
  fcfdfilt->GetYaxis()->SetTitle("Filter Amplitude (Arb. Units)");
  fcfdfilt->SetAxisRange(xmin, xmax);
  fcfdfilt->GetYaxis()->SetTitleSize(0.07);
  fcfdfilt->GetYaxis()->SetTitleOffset(0.5);
  fcfdfilt->GetYaxis()->CenterTitle();
  fcfdfilt->GetYaxis()->SetLabelSize(0.07);
  fcfdfilt->GetYaxis()->SetNdivisions(505);
  fcfdfilt->GetXaxis()->SetTitle(Form("Time (%.0f ns Bin Width)",dt*1000));
  fcfdfilt->GetXaxis()->CenterTitle();
  fcfdfilt->GetXaxis()->SetTitleSize(0.07);
  fcfdfilt->GetXaxis()->SetTitleOffset(-7.5);
  fcfdfilt->GetXaxis()->CenterTitle();
  fcfdfilt->GetXaxis()->SetLabelSize(0.07);
  fcfdfilt->DrawCopy("hist c");

  ffastfilt->SetLineColor(4);
  ffastfilt->GetYaxis()->CenterTitle();
  ffastfilt->SetLineWidth(2);
  ffastfilt->DrawCopy("same hist c");

  leg_tf->AddEntry("ffastfilt","Trigger Filter","l");  
  leg_tf->AddEntry("fcfdfilt","Digital CFD","l");
  leg_tf->SetFillColor(0);
  leg_tf->Draw();

  dCanvasF1->cd(3);
  TLegend *leg_ef = new TLegend(0.75,0.75,0.85,0.85);
  fenerfilt->SetLineColor(6);
  fenerfilt->SetAxisRange(xmin, xmax);
  fenerfilt->GetYaxis()->SetTitle("Filter Amplitude (Arb. Units)");
  fenerfilt->GetYaxis()->CenterTitle();
  fenerfilt->GetYaxis()->SetTitleSize(0.07);
  fenerfilt->GetYaxis()->SetTitleOffset(0.5);
  fenerfilt->GetYaxis()->CenterTitle();
  fenerfilt->GetYaxis()->SetLabelSize(0.07);
  fenerfilt->GetYaxis()->SetNdivisions(505);
  fenerfilt->GetXaxis()->SetTitle(Form("Time (%.0f ns Bin Width)",dt*1000));
  fenerfilt->GetXaxis()->CenterTitle();
  fenerfilt->GetXaxis()->SetTitleSize(0.07);
  fenerfilt->GetXaxis()->SetTitleOffset(-7.5);
  fenerfilt->GetXaxis()->CenterTitle();
  fenerfilt->GetXaxis()->SetLabelSize(0.07);
  //  fcfdfilt->GetXaxis()->SetNdivisions(505);
  fenerfilt->SetLineWidth(2);
  fenerfilt->DrawCopy("hist c");
  leg_ef->AddEntry("fenerfilt","Energy Filter","l");
  leg_ef->SetFillColor(0);
  leg_ef->Draw();
    
  dCanvasF1->Modified();
  dCanvasF1->Update();
  
  gSystem->ProcessEvents();

}



void Main::writeSpe(const char *filename, float buf[], int dim)
{
  struct spePrefix
  {
    int reclA; /* 24 */
    unsigned titleA;
    unsigned titleB;
    int dim;
    int a1; /*  1 */
    int a2; /*  1 */
    int a3; /*  1 */
    int reclB; /* 24 */
  } x = { 24, 0, 0, 0, 1, 1, 1, 24 };
  int recl;
  
  ofstream out(filename, ios::out | ios::binary);
  x.dim = dim;
  recl = sizeof(float) * dim;
  out.write(reinterpret_cast < char *>(&x), sizeof(struct spePrefix));
  out.write(reinterpret_cast < char *>(&recl), sizeof (recl));
  out.write(reinterpret_cast < char *>(buf), sizeof(float) * dim);
  out.write(reinterpret_cast < char *>(&recl), sizeof (recl));
  out.close();
}

void Main::writeSpect(const char *filename, unsigned long buf[], int dim)
{
  char tmp[1024];
  ofstream out(filename, ios::out);
  for (int i = 0 ; i < dim ; ++i){
    out << buf[i] << endl;
  }
  out.close();
}



void Main::save_setup(char *name)
{
  Pixie16SaveDSPParametersToFile(name);
  cout << "Saving setup to file: " << name << endl;
}

int Main::IdentifyTracePulse (unsigned short *trace, // Trace array
			     unsigned int traceSize, // Size of trace array
			     unsigned int trigLen, // Trigger filter length
			     unsigned int trigGap, // Trigger filter gap
			     double *trigLeadSum, // Trigger filter lead sum
			     double *trigTrailSum, // Trigger filter trail sum
			     double trigThresh, // Trigger filter threshold
			     unsigned int *peak, // Pulse maximum location
			     unsigned int *valley) // Pulse minimum location
{
  /* Finds a pulse in an ADC trace for finding the value of tau.  Returns 
     0 for successfully finding a peak, 1 if peak not found in trace */
  
  unsigned int FIT_TRACE_SIZE=8191;
  
  /* Given a trace identify the first pulse from the start */
  
  unsigned int   i = 0; // Used to track the current position in the trace
  unsigned int   PrevMin; // A variable to store a previously found minimum
  unsigned int   PrevMax; // A variable to store a previously found maximum
  unsigned int   MaxOccur; // Max occurrence counter
  unsigned int   MinOccur; // Min occurrence counter
  unsigned short PulseMax = 0; // Pulse maximum to find the maximum location
  unsigned short PulseMin = 65535; // Pulse minimum to find the minimum location
  
  /* Accumulate TrigTrailSum over the first TrigLen points. Report an 
     error if end of trace is encountered. */
  *trigTrailSum = 0;
  while (i < trigLen && i < traceSize) {
    *trigTrailSum += (double)trace[i++];
  }
  if (i == traceSize) { return (1); }
  
  /* Step over the gap. Report an error if end of trace is encountered. */
  i += trigGap;
  if (i >= traceSize) { return (1); }
  
  /* Accumulate TrigLeadSum over the next TrigLen points. Report an 
     error if end of trace is encountered. */
  *trigLeadSum = 0;
  while (i < (2 * trigLen + trigGap) && i < traceSize) {
    *trigLeadSum += (double)trace[i++];
  }
  if (i == traceSize) { return (1); }
  
  /* Determine the trigger point i of the pulse using the running trigger 
     filter sums. The condition for triggering is the exceeding the value 
     TrigThresh by the difference (TrigLeadSum - TrigTrailSum). Report an 
     error if end of trace is encountered. */
  while ((*trigLeadSum - *trigTrailSum) < trigThresh && i < traceSize) {
    *trigLeadSum  += ((double)trace[i] - (double)trace[i - trigLen]);
    *trigTrailSum += ((double)trace[i - trigLen - trigGap] - 
		      (double)trace[i - 2 * trigLen - trigGap]);
    i++;
  }
  if (i == traceSize) { return (1); }
  
  /* Step back at the beginning of the trailing sum to make sure the 
     trigger point does not happen beyond the maximum of the pulse */
  i -= (trigLen + trigGap); 
  
  /* Find the maximum location of the pulse *Peak by slowly extending
     the right limit of the maximization interval. A maximum found repeatedly 
     4*(trigger width) times is reported as the pulse maximum. Report an error 
     if end of trace is encountered. */
  MaxOccur = 0;
  while (MaxOccur < 10 * (2 * trigLen + trigGap) && i < traceSize) {
    PrevMax = *peak;
    if (PulseMax < trace[i]) { 
      PulseMax = trace[i]; 
      *peak = i;
      MaxOccur = 0;
    } else {
      MaxOccur++;
    }
    i++;
  }
  if (i == traceSize) { return (1); }
  
  /* Find the minimum location of the pulse *Valley by slowly extending 
     the right limit of the minimization interval. A minimum found 
     repeatedly 4*(trigger width) times is reported as the pulse minimum. 
     The pulse minimum is the minimum point between the pulses or the last 
     point of the pulse determined by the maximum fitting range (set by 
     FIT_TRACE_SIZE). Report an error if end of trace is encountered. */
  MinOccur = 0;
  i = *peak;
  while (MinOccur < 10 * (2 * trigLen + trigGap) && 
	 i < *peak + FIT_TRACE_SIZE && i < traceSize) {
    PrevMin = *valley;
    if (PulseMin > trace[i]) { 
      PulseMin = trace[i]; 
      *valley = i;
      MinOccur = 0; 
    } else {
      MinOccur++;
    }
    i++;
  }

  if(PrevMin < 0) std::cout << "Something appears to be very wrong with Min" << std::endl;
  if(PrevMax < 0) std::cout << "Something appears to be bery wrong with Max" <<std::endl;
  if (i == traceSize) { return (1); }
  
  return (0);
}

int Main::TauFromMoments(unsigned short *trace, unsigned int traceSize,
			double dt, double *tau) {
  double sp1, sp2;
  double N = double(traceSize); // Number of samples in the sum
  double I;

  sp1 = sp2 = I = 0;
  while (I < N) {
    sp1 += (double)trace[(unsigned int)I] * ((I+1)*N*N - 3.0*(I+1)*(I+1)*N
					     + 2.0*(I+1)*(I+1)*(I+1));
    sp2 += (double)trace[(unsigned int)I] * (I*N*N - 3.0*I*I*N
					     + 2.0*I*I*I);
    I++;
  }
  /* Compute tau from the analytic formula and report in units of dt */
  *tau = 1.0 / log(sp1/sp2) * dt;
  return (0);
}
 
double fitfunction(double *x, double *par) {
  return par[0] + par[1]*(exp(-x[0]*par[2]));
}

int Main::TauFromFit(unsigned short *trace, unsigned int traceSize,
		    double dt, double *tau) {

  taufitHist = new TH1S("taufit", "Fitting Tau", traceSize, 0, traceSize);
  for (unsigned int i=0; i<traceSize; i++) {
    taufitHist->Fill(i, trace[i]);
  } 

  TF1 *taufitFcn = new TF1("fit", fitfunction, 0, traceSize, 3);
  taufitFcn->SetParameter(0, 500);
  int retval;
  double tau_start = -1;
  char pTAU[]="TAU";
  retval = Pixie16ReadSglChanPar(/*"TAU"*/pTAU, &tau_start, moduleNr, channelNr);
  if(retval < 0) std::cout << "Error reading tau from TauFromFit in main" << std::endl;
  tau_start = tau_start/dt;
  tau_start = (1e6)/tau_start;
  taufitFcn->SetParameter(2, tau_start);
  taufitFcn->SetParLimits(0, 300, 800);
  taufitFcn->SetParLimits(2, 0, 100);

  taufitHist->Fit("fit", "Q");
  double tau_par[3];
  taufitFcn->GetParameters(&tau_par[0]);
  double fit_tau = 1/tau_par[2];
 
   *tau = fit_tau*dt;

  dCanvasF1->Clear();
  dCanvasF1->cd();
  dCanvasF1->SetBorderMode(0);
  taufitHist->DrawCopy();
  taufitFcn->DrawCopy("SAME");
  dCanvasF1->Modified();
  dCanvasF1->Update();
  
  gSystem->ProcessEvents();

  delete taufitHist;
  return(1);
}
 
int Main::BinTrace (double *trace, unsigned int traceSize, double *bins,
		   unsigned int binNum, double *binCounts) 
{
  unsigned int i = 1;
  unsigned int j = 0;
  double minTrace = 32768;
  double maxTrace = 0;
  double h; // Bin half-width

  while (j < traceSize) {
    if (maxTrace < trace[j]) { maxTrace = trace[j]; }
    if (minTrace > trace[j]) { minTrace = trace[j]; }
    j++;
  }
  h = (maxTrace - minTrace) / binNum / 2;
  bins[0] = minTrace + h;
  bins[binNum - 1] = maxTrace - h;
  while (i < (binNum - 1)) {
    bins[i] = bins[0] + 2*h*i;
    i++;
  }
  i = 0;
  while (i < binNum) {
    j = 0;
    binCounts[i] = 0;
    while (j < traceSize) {
      if (trace[j] > (bins[i]-h) && trace[j] <= (bins[i]+h)) {
	binCounts[i]++;
      }
      j++;
    }
    i++;
  }
  return 0;

}

int Main::BinTraceFit (double *trace, double *tracefit, 
		       unsigned int traceSize, double *bins,
		       unsigned int binNum, double *binCounts) 
{
  unsigned int i = 1;
  unsigned int j = 0;
  double minTrace = 32768;
  double maxTrace = 0;
  double h; // Bin half-width

  while (j < traceSize) {
    if (maxTrace < trace[j]) { maxTrace = trace[j]; }
    if (minTrace > trace[j]) { minTrace = trace[j]; }
    j++;
  }
  h = (maxTrace - minTrace) / binNum / 2;
  bins[0] = minTrace + h;
  bins[binNum - 1] = maxTrace - h;
  while (i < (binNum - 1)) {
    bins[i] = bins[0] + 2*h*i;
    i++;
  }
  i = 0;
  while (i < binNum) {
    j = 0;
    binCounts[i] = 0;
    while (j < traceSize) {
      if (tracefit[j] > (bins[i]-h) && tracefit[j] <= (bins[i]+h)) {
	binCounts[i]++;
      }
      j++;
    }
    i++;
  }
  return 0;

}

double Main::ArrayMax (double *a, unsigned int arraySize, unsigned int *index)
{
   double *a_last = a + arraySize;
   double *a_first = a;
   double *maxpos = a;
   while (a < a_last) { if (*maxpos < *a) maxpos = a; a++; }
   *index = maxpos - a_first;
   return *maxpos;
}

double Main::FitGaussian(TGraph *Dist) 
{

  Dist->Fit("gaus", "Q");
  //double par[3];
  TF1 *fitresult = Dist->GetFunction("gaus");

  double mean;
  mean = fitresult->GetParameter(1);
  
  return mean;
}

int Main::FindTau (unsigned short ModNum, unsigned short ChanNum, 
		   double *Tau, double *TauFit) 
{

  double dt; // Time in seconds between trace samples
  unsigned short trace[RANDOMINDICES_LENGTH];
  unsigned int traceSize = RANDOMINDICES_LENGTH;
  int retval;
  unsigned int pulseFindAttempt; // Pulse finding attempt counter
  unsigned int pulseCounter; // Pulse counter
  unsigned int traceCounter; // Trace counter
  unsigned int start; // Running starting point for peak finding analysis in trace
  unsigned int peak; // Current peak location in trace
  unsigned int valley; // Current vally location in trace
  unsigned int dummy; // Dummy variable
  double trigLen; // Trigger filter length
  double trigGap; // Trigger filter gap
  double trigLeadSum; // Trigger filter leading sum
  double trigTrailSum; // Trigger filter trailing sum
  double trigThresh; // Trigger filter threshold
  double tauArray[RANDOMINDICES_LENGTH]; // Reasonable tau values from analyzed peaks
  double tauArrayFit[RANDOMINDICES_LENGTH]; // Reasonable tau values from fits
  double tauBins[RANDOMINDICES_LENGTH]; // Array of tau intervals for binning
  double tauBinsFit[RANDOMINDICES_LENGTH];
  double tauBinCounts[RANDOMINDICES_LENGTH]; // Counts in tau bins
  double tauBinCountsFit[RANDOMINDICES_LENGTH];

  char pXDT[]="XDT";
  char pTRIGGER_RISETIME[]="TRIGGER_RISETIME";
  char pTRIGGER_FLATTOP[]="TRIGGER_FLATTOP";
  char pTRIGGER_THRESHOLD[]="TRIGGER_THRESHOLD";

  /* Check if module number is valid */
  if ( ModNum >= 10 ) {
    cout << "***Error*** TauFind: invalid Pixie module number " << ModNum << endl;
    return(-1);
  }
  
  /* Check if channel number is valid */
  if (ChanNum >= 16) {
    cout << "***Error*** TauFind: invalid Pixie channel number " << ChanNum << endl;
    return(-1);
  }

  /* Get time between ADC samples */
  retval = Pixie16ReadSglChanPar(/*"XDT"*/pXDT, &dt, ModNum, ChanNum);
  if (retval == 0) {
    dt /= 1.0e6;
  } else {
    cout << "Error reading XDT parameter value for module " << ModNum
	 << ", channel " << ChanNum << endl;
  }

  /* Get trigger filter length */
  retval = Pixie16ReadSglChanPar(/*"TRIGGER_RISETIME"*/pTRIGGER_RISETIME, &trigLen, ModNum, ChanNum);
  if (retval != 0) {
    cout << "Error reading TRIGGER_RISETIME parameter value for module " << ModNum
	 << ", channel " << ChanNum << endl;
  } 
  
  /* Get trigger filter gap */
  retval = Pixie16ReadSglChanPar(/*"TRIGGER_FLATTOP"*/pTRIGGER_FLATTOP, &trigGap, ModNum, ChanNum);
  if (retval != 0) {
    cout << "Error reading TRIGGER_FLATTOP parameter value for module " << ModNum
	 << ", channel " << ChanNum << endl;
  }

  /* Rescale trigger filter parameters */
  trigLen = (unsigned int)(((trigLen*1000)/10)/ dt*1e-8);
  trigGap = (unsigned int)(((trigGap*1000)/10)/ dt*1e-8);
  
  /* Get trigger filter threshold */
  retval = Pixie16ReadSglChanPar(/*"TRIGGER_THRESHOLD"*/pTRIGGER_THRESHOLD, &trigThresh, ModNum, ChanNum);
  if (retval != 0) {
    cout << "Error reading TRIGGER_THRESHOLD parameter value for module " << ModNum 
	 << ", channel " << ChanNum << endl;
  }
  trigThresh *= (dt/1e-8);
  
  if (trigLen < 5) trigLen = 5;

  cout << "Trigger length    " << trigLen << endl
       << "Trigger gap       " << trigGap << endl
       << "Trigger threshold " << trigThresh << endl
       << "dt                " << dt << endl;

  /* Analyze 200 pulses using no more than 5000 traces to 
     avoid an infinite loop on an empty channel */
  
  pulseCounter = traceCounter = 0;
  
  while (pulseCounter < 200 && traceCounter < 5000) {
    /* Get a trace */
    if ((retval = Pixie16AcquireADCTrace(ModNum)) < 0) {
      cout << "Failed to get ADC traces in module " << ModNum << endl;
      return(-3);
    }
    
    /* Read a trace */
    if ((retval = Pixie16ReadSglChanADCTrace(trace, RANDOMINDICES_LENGTH, 
					     ModNum, ChanNum)) < 0) {
      cout << "Failed to read ADC traces from module " << ModNum 
	   << ", channel " << ChanNum << endl;
      return(-4);
    }

    /* Find pulses in trace one by one using Identify_Trace_Pulse function, 
       until the function returns an error.  Make no more than 100 attempts to 
       find pulses.  The variable start is a running starting point in the 
       trace from which pulse identification analysis begins -- start is not 
       allowed to go over the trace length.  Identify_Trace_Pulse function 
       reports baseline, peak and valley variables which isolate a pulse in the 
       trace.  Peak and valley are unsigned integer absolute shifts from the 
       start of the trace, while baseline is a double precision number. */

    start = pulseFindAttempt = 0;
    while (pulseFindAttempt < 100 && start < traceSize && 
	   IdentifyTracePulse(&trace[start], traceSize-start, trigLen, 
				trigGap, &trigLeadSum, &trigTrailSum, 
				trigThresh, &peak, &valley) == 0) {
      peak += start; // Peak and valley are reported relative to the start
      valley += start;

      /* The points between peak and valley are used to find values of tau.  
	 No less than 5*(2*trigLen+trigGap) points are allowed.  Tau is 
	 determined using a procedure from moments of the data with Legendre 
	 polynomials.  */
      if (valley - peak > 5*(2*trigLen + trigGap)) {
	TauFromMoments( &trace[peak + (valley-peak) / 10],
			valley - peak - (valley-peak) / 10,
			dt, &tauArray[pulseCounter]);
	TauFromFit( &trace[peak + (valley-peak) / 10],
		    valley - peak - (valley-peak) / 10,
		    dt, &tauArrayFit[pulseCounter]);
	/* Check for reasonable values of tau */
	if (tauArray[pulseCounter] > 1e-8) {
        tauArray[pulseCounter] *= 1.0e6;
	}
	if ((tauArrayFit[pulseCounter] > 1e-8)) {
	  tauArrayFit[pulseCounter] *= 1.0e6;
	}
	pulseCounter++;
      }
      /* Next starting point for analysis is the previous valley minus 
	 the total trigger filter width unless valley-peak is too small. */
      if (valley-peak > 2*trigLen - trigGap) {
	start = (unsigned int)(valley - 2*trigLen - trigGap);
      } else {
	start = valley;
      }
      pulseFindAttempt++;
    }
    traceCounter++;
  }

  /* Plot the distributions of tau values */
  double x[pulseCounter], y[pulseCounter], y2[pulseCounter];
  for (unsigned int i=0; i<pulseCounter; i++) {
    x[i] = i;
    y[i] = tauArray[i];
    y2[i] = tauArrayFit[i];
  }
  TGraph *tauGraph = new TGraph(pulseCounter, x, y);
  TGraph *tauGraphFit = new TGraph(pulseCounter, x, y2);
  TMultiGraph *mg1 = new TMultiGraph();

  mg1->SetTitle("Tau vs. Fit#");

  dCanvasF1->Clear();
  dCanvasF1->Divide(2,1);
  dCanvasF1->cd(1);
  dCanvasF1->SetBorderMode(0);
  tauGraph->SetMarkerStyle(3);
  tauGraph->SetMarkerSize(1);
  tauGraph->SetMarkerColor(1);
  mg1->Add(tauGraph);
  tauGraphFit->SetMarkerStyle(24);
  tauGraphFit->SetMarkerSize(1);
  tauGraphFit->SetMarkerColor(2);
  mg1->Add(tauGraphFit);
  mg1->Draw("AP");
 
  /* Further process the array of tau values by binning into a 
     distribution.  The maximum of the distribution is reported as
     the best estimate for tau. */
  if (pulseCounter > 5) {
    BinTrace(tauArray, pulseCounter, tauBins, 200, tauBinCounts);
    ArrayMax(tauBinCounts, 200, &dummy);
    *Tau = tauBins[dummy];
    cout << "pulseCounter " << pulseCounter << " traceCounter " 
	 << traceCounter << endl;
    BinTrace(tauArrayFit, pulseCounter, tauBinsFit, 200, 
		tauBinCountsFit);
    ArrayMax(tauBinCountsFit, 200, &dummy);
    *TauFit = tauBinsFit[dummy];
    TGraph *tauDist = new TGraph(200, tauBins, tauBinCounts);
    TGraph *tauDistFit = new TGraph(200, tauBinsFit, tauBinCountsFit);
    double tau, taufit;
    tau = FitGaussian(tauDist);
    taufit = FitGaussian(tauDistFit);
    cout << "Tau (gaussian) from moments: " << tau << endl;
    cout << "Tau (gaussian) from fits:    " << taufit << endl;
    BinTraceFit(tauArray, tauArrayFit, pulseCounter, tauBinsFit, 200, 
		tauBinCountsFit);
    
    TGraph *tauDistFit2 = new TGraph(200, tauBinsFit, tauBinCountsFit);
    tauDist->SetTitle("Tau distribution");
    dCanvasF1->cd(2);
    dCanvasF1->SetBorderMode(0);
    tauDist->Draw("AB");
    tauDistFit2->SetFillColor(2);
    tauDistFit2->Draw("B");
    dCanvasF1->Modified();
    dCanvasF1->Update();

    gSystem->ProcessEvents();
    
  } else {
    cout << "Failed to find sufficient number of pulses in channel " 
	 << ChanNum << endl;
    cout << "pulseCounter " << pulseCounter << " traceCounter " 
	 << traceCounter << endl;
    return(-5);
  }
  
  return(0);
  
}
