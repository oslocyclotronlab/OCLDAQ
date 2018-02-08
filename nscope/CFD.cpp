#include "CFD.h"

CFD::CFD(const TGWindow * p, const TGWindow * main,
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
  CLabel[0]->SetText("CFD Delay[us]");
  CLabel[0]->SetAlignment(kTextCenterX);
  CLabel[1]->SetText("CFD Scale");
  CLabel[1]->SetAlignment(kTextCenterX);
  CLabel[2]->SetText("CFD Thresh. [ADC u]");
  CLabel[2]->SetAlignment(kTextCenterX);
  //load_info (0);
  modNumber = 0;
  Load_Once = false;

/////////////////Copy Button////////////////////////////////
  TGHorizontal3DLine *ln2 = new TGHorizontal3DLine(mn_vert, 200, 2);
  mn_vert->AddFrame(ln2,
		    new TGLayoutHints(kLHintsCenterX | kLHintsCenterY, 0,
				      0, 10, 10));
  TGHorizontalFrame *CopyButton = new TGHorizontalFrame(mn_vert, 400, 300);
  mn_vert->AddFrame(CopyButton,
		    new TGLayoutHints(kLHintsTop | kLHintsLeft, 0, 0, 0,
				      0));

  TGLabel *Copy = new TGLabel(CopyButton, "Select channel #");

  chanCopy = new TGNumberEntry(CopyButton, 0, 4, MODNUMBER + 1000, TGNumberFormat::kNESInteger, TGNumberFormat::kNEANonNegative, TGNumberFormat::kNELLimitMinMax, 0, 15);
  chanCopy->SetButtonToNum(0);
  chanCopy->IsEditable();
  chanCopy->SetIntNumber(0);
  CopyButton->AddFrame(Copy,
		       new TGLayoutHints(kLHintsCenterX, 5, 10, 3, 0));
  CopyButton->AddFrame(chanCopy,
		       new TGLayoutHints(kLHintsTop | kLHintsLeft, 0, 20,
					 0, 0));

  chanCopy->Associate(this);

  ////////////////////Copy button per se///////////////////
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

  Load_Once = true;
  chanNumber = 0;
  cfddelay = 0;
  cfdscale = 0;
  cfdthresh = 0;


  MapSubwindows();
  Resize();
}

CFD::~CFD()
{
}

Bool_t CFD::ProcessMessage(Long_t msg, Long_t parm1,
				       Long_t parm2)
{
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
	/////////////////////////////
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
	/////////////////////////////////////
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
	  std::cout << "please load once first !\n";
	break;
      case CANCEL:		/// Cancel Button
	DeleteWindow();
	break;
      case (COPYBUTTON + 1000):
	cfddelay = NumEntry[1][chanNumber]->GetNumber();
	cfdscale = NumEntry[2][chanNumber]->GetNumber();
	cfdthresh = NumEntry[3][chanNumber]->GetNumber();
	for (int i = 0; i < 16; i++) {
	  if (i != chanNumber) {
	    sprintf(tmp, "%1.3f", cfddelay);
	    NumEntry[1][i]->SetText(tmp);
        sprintf(tmp, "%d", static_cast<int>(cfdscale));
	    NumEntry[2][i]->SetText(tmp);
	    sprintf(tmp, "%1.3f", cfdthresh);
 	    NumEntry[3][i]->SetText(tmp);
	  }
	}
//                  
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

int CFD::load_info(Long_t module)
{

  double ChanParData = -1;

  int retval;
  char text[20];
  char pCFDDelay[]="CFDDelay";
  char pCFDScale[]="CFDScale";
  char pCFDThresh[]="CFDThresh";

  for (int i = 0; i < 16; i++) {
    retval =
      Pixie16ReadSglChanPar(/*"CFDDelay"*/pCFDDelay, &ChanParData,
			      modNumber, i);

    sprintf(text, "%1.3f", ChanParData);
    NumEntry[1][i]->SetText(text);

    retval =
      Pixie16ReadSglChanPar(/*"CFDScale"*/pCFDScale, &ChanParData,
			      modNumber, i);
    sprintf(text, "%d", static_cast<int>(ChanParData));
    NumEntry[2][i]->SetText(text);

    retval =
      Pixie16ReadSglChanPar(/*"CFDThresh"*/pCFDThresh, &ChanParData,
			      modNumber, i);
    sprintf(text, "%1.3f", ChanParData);
    NumEntry[3][i]->SetText(text);
  }
  std::cout << "loading info for module " << module << std::endl;

  return retval;
}


int CFD::change_values(Long_t module)
{

  double cfddelay;
  double cfdscale;
  double cfdthresh;
  char pCFDDelay[]="CFDDelay";
  char pCFDScale[]="CFDScale";
  char pCFDThresh[]="CFDThresh";

  for (int i = 0; i < 16; i++) {
    cfddelay = NumEntry[1][i]->GetNumber();
    cout << cfddelay << " test cfddelay " << endl;
    Pixie16WriteSglChanPar(/*"CFDDelay"*/pCFDDelay, cfddelay, modNumber, i);
    cfdscale = NumEntry[2][i]->GetNumber();
    Pixie16WriteSglChanPar(/*"CFDScale"*/pCFDScale, cfdscale, modNumber, i);
    cfdthresh = NumEntry[3][i]->GetNumber();
    Pixie16WriteSglChanPar(/*"CFDThresh"*/pCFDThresh, cfdthresh, modNumber, i);
  }


  return 1;
}
