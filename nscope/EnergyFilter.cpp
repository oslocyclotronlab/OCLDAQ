#include "EnergyFilter.h"
#include "pixie16app_export.h"

EnergyFilter::EnergyFilter(const TGWindow * p, const TGWindow * main,
			   /*char **/string name, int columns, int rows,
			   int NumModules):Table(p, main, columns, rows,
						 name, NumModules)
{
  char n[10];
  cl0->SetText("ch #");
  for (int i = 0; i < rows; i++) {
    sprintf(n, "%2d", i);
    Labels[i]->SetText(n);
  }
  
  CLabel[0]->SetText("TPeaking[us]");
  CLabel[0]->SetAlignment(kTextCenterX);
  CLabel[1]->SetText("TGap[us]");
  CLabel[1]->SetAlignment(kTextCenterX);
  
  risetime = 0;
  flattop = 0;
  fRange = 0;
  modNumber = 0;
  Load_Once = true;
  
  TGHorizontalFrame *Details = new TGHorizontalFrame(mn_vert, 400, 300);
  
  /********** Filter range entry **********/

  TGHorizontal3DLine *ln2 = new TGHorizontal3DLine(mn_vert, 60, 2);
  TGLabel *fil = new TGLabel(Details, "Filter Range");

  filterRange = new TGNumberEntry(Details, 0, 4, FILTER, 
				  TGNumberFormat::kNESInteger, 
				  TGNumberFormat::kNEANonNegative, 
				  TGNumberFormat::kNELLimitMinMax, 0, 15);
  filterRange->SetButtonToNum(0);

  mn_vert->AddFrame(ln2,
		    new TGLayoutHints(kLHintsCenterX | kLHintsCenterY, 0,
				      0, 10, 10));
  Details->AddFrame(fil, new TGLayoutHints(kLHintsCenterX, 5, 10, 3, 0));
  Details->AddFrame(filterRange,
		    new TGLayoutHints(kLHintsTop | kLHintsLeft, 0, 20, 0,
				      0));

  filterRange->Associate(this);
  mn_vert->AddFrame(Details,
		    new TGLayoutHints(kLHintsTop | kLHintsLeft, 0, 0, 0,
				      0));

  // Load the current filter range:
  char pSLOW_FILTER_RANGE[]= "SLOW_FILTER_RANGE";

  Pixie16ReadSglModPar(/*"SLOW_FILTER_RANGE"*/pSLOW_FILTER_RANGE, &fRange, modNumber);

  filterRange->SetIntNumber(fRange);

  /********** Copy Button **********/

  TGHorizontal3DLine *ln3 = new TGHorizontal3DLine(mn_vert, 200, 2);
  mn_vert->AddFrame(ln3,
		    new TGLayoutHints(kLHintsCenterX | kLHintsCenterY, 0,
				      0, 10, 10));
  TGHorizontalFrame *CopyButton = new TGHorizontalFrame(mn_vert, 400, 300);
  mn_vert->AddFrame(CopyButton,
		    new TGLayoutHints(kLHintsTop | kLHintsLeft, 0, 0, 0,
				      0));
  
  TGLabel *Copy = new TGLabel(CopyButton, "Select channel #");
  
  chanCopy = new TGNumberEntry(CopyButton, 0, 4, MODNUMBER + 1000, 
			       (TGNumberFormat::EStyle) 0, 
			       (TGNumberFormat::EAttribute) 1, 
			       (TGNumberFormat::ELimit) 3, // kNELLimitMinMax
			       0, 3);
  chanCopy->SetButtonToNum(0);
  chanCopy->IsEditable();

  CopyButton->AddFrame(Copy,
		       new TGLayoutHints(kLHintsCenterX, 5, 10, 3, 0));
  CopyButton->AddFrame(chanCopy,
		       new TGLayoutHints(kLHintsTop | kLHintsLeft, 0, 20,
					 0, 0));
  
  chanCopy->Associate(this);
  
  /********** Copy button per se **********/
  TGTextButton *copyB =
    new TGTextButton(CopyButton, "C&opy", COPYBUTTON + 1000);
  copyB->Associate(this);
  copyB->
    SetToolTipText
    ("Copy the setup of the selected channel to all channels of the module",
     0);
  CopyButton->AddFrame(copyB,
		       new TGLayoutHints(kLHintsTop | kLHintsLeft, 0, 20,
					 0, 0));
  
  chanNumber = 0;
  chanCopy->SetIntNumber(0);
  MapSubwindows();
  Resize();			// resize to default size
}

EnergyFilter::~EnergyFilter()
{
}

Bool_t EnergyFilter::ProcessMessage(Long_t msg, Long_t parm1, Long_t parm2)
{
  char pSLOW_FILTER_RANGE[]= "SLOW_FILTER_RANGE";

  switch (GET_MSG(msg)) {
  case kC_COMMAND:
    switch (GET_SUBMSG(msg)) {
    case kCM_BUTTON:
      switch (parm1) {
      case (MODNUMBER):
	if (parm2 == 0) {
	  if (modNumber != numModules - 1) {
	    ++modNumber;
	    numericMod->SetIntNumber(modNumber);
	    load_info(modNumber);
	  }
	} else {
	  if (modNumber != 0) {
	    if (--modNumber == 0)
	      modNumber = 0;
	    numericMod->SetIntNumber(modNumber);
	    load_info(modNumber);
	  }
	}
	break;

	
      case (MODNUMBER + 1000):
	if (parm2 == 0) {
	  if (chanNumber != 15) {
	    ++chanNumber;
	    chanCopy->SetIntNumber(chanNumber);
	  }
	} else {
	  if (chanNumber != 0) {
	    --chanNumber;
	    chanCopy->SetIntNumber(chanNumber);
	  }
	}
	break;

	
      case (FILTER):
	if (parm2 == 0) {
	  if (fRange != 6) {
	    ++fRange;
	    filterRange->SetIntNumber(fRange);
	    cout << fRange << " " << modNumber << "\n" << flush;
	    Pixie16WriteSglModPar(/*"SLOW_FILTER_RANGE"*/pSLOW_FILTER_RANGE, fRange, modNumber);
	  }
	} else {
	  if (fRange != 1) {
	    if (--fRange == 1) {
	      fRange = 1;
	    }
	    cout << fRange << " " << modNumber << "\n" << flush;
	    filterRange->SetIntNumber(fRange);
	    Pixie16WriteSglModPar(/*"SLOW_FILTER_RANGE"*/pSLOW_FILTER_RANGE, fRange, modNumber);
	  }
	}
	break;

	
      case LOAD:
	{
	  Load_Once = true;
	  load_info(modNumber);
	}
	break;


      case APPLY:
	if (Load_Once)
	  change_values(modNumber);
	else
	  std::cout << "Please load once first !\n";
	break;


      case CANCEL:    // Cancel Button
	DeleteWindow();
	break;

	
      case (COPYBUTTON + 1000):
	risetime = NumEntry[1][chanNumber]->GetNumber();
	flattop = NumEntry[2][chanNumber]->GetNumber();
	for (int i = 0; i < 16; i++) {
	  if (i != chanNumber) {
	    char tmp[10];
	    sprintf(tmp, "%1.3f", risetime);
	    NumEntry[1][i]->SetText(tmp);
	    sprintf(tmp, "%1.3f", flattop);
	    NumEntry[2][i]->SetText(tmp);
	  }
	}
	break;

      default:
	break;
      }
      break;
    default:
      break;
    }
    break;
  default:
    break;
  }

  return kTRUE;
}

int EnergyFilter::load_info(Long_t mod)
{
  double ChanParData = -1;

  int retval;
  char text[20];
  char pENERGY_RISETIME[]="ENERGY_RISETIME";
  char pENERGY_FLATTOP[]="ENERGY_FLATTOP";
  char pSLOW_FILTER_RANGE[]="SLOW_FILTER_RANGE";

  for (int i = 0; i < 16; i++) {
    retval =
      Pixie16ReadSglChanPar(/*"ENERGY_RISETIME"*/pENERGY_RISETIME, &ChanParData, modNumber,
			      i);

    sprintf(text, "%1.3f", ChanParData);
    NumEntry[1][i]->SetText(text);

    usleep(10);

    retval =
      Pixie16ReadSglChanPar(/*"ENERGY_FLATTOP"*/pENERGY_FLATTOP, &ChanParData, modNumber,
			      i);
    sprintf(text, "%1.3f", ChanParData);
    NumEntry[2][i]->SetText(text);

    usleep(10);
  }

  usleep(100);
  Pixie16ReadSglModPar(/*"SLOW_FILTER_RANGE"*/pSLOW_FILTER_RANGE, &fRange, modNumber);
  usleep(100);

  filterRange->SetIntNumber(fRange);

  return retval;
}

int EnergyFilter::change_values(Long_t mod)
{
  char pENERGY_RISETIME[]="ENERGY_RISETIME";
  char pENERGY_FLATTOP[]="ENERGY_FLATTOP";
  double length;
  double delay;
  for (int i = 0; i < 16; i++) {
    length = NumEntry[1][i]->GetNumber();
    Pixie16WriteSglChanPar(/*"ENERGY_RISETIME"*/pENERGY_RISETIME, length, modNumber, i);
    delay = NumEntry[2][i]->GetNumber();
    Pixie16WriteSglChanPar(/*"ENERGY_FLATTOP"*/pENERGY_FLATTOP, delay, modNumber, i);
  }

  return 1;
}
