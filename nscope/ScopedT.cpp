#include "ScopedT.h"

ScopedT::~ScopedT()
{
}

ScopedT::ScopedT(const TGWindow * p, const TGWindow * main, /*char **/string name,
		 int columns, int rows):Table(p, main, columns, rows, name)
{
  char n[10];
  cl0->SetText("ch #");
  for (int i = 0; i < rows; i++) {
    sprintf(n, "%2d", i);
    Labels[i]->SetText(n);
  }
  CLabel[0]->SetText("dT [us]");
  CLabel[0]->SetAlignment(kTextCenterX);


  /********** Copy Button **********/
  TGHorizontal3DLine *ln2 = new TGHorizontal3DLine(mn_vert, 200, 2);
  mn_vert->AddFrame(ln2,
		    new TGLayoutHints(kLHintsCenterX | kLHintsCenterY, 0,
				      0, 10, 10));
  TGHorizontalFrame *CopyButton = new TGHorizontalFrame(mn_vert, 400, 300);
  mn_vert->AddFrame(CopyButton,
		    new TGLayoutHints(kLHintsTop | kLHintsLeft, 0, 0, 0, 0));
  
  TGLabel *Copy = new TGLabel(CopyButton, "Select channel #");
  
  chanCopy = new TGNumberEntry(CopyButton, 0, 4, MODNUMBER + 1000, 
			       TGNumberFormat::kNESReal, 
			       TGNumberFormat::kNEANonNegative, 
			       TGNumberFormat::kNELLimitMinMax, 0, 3);
  chanCopy->SetButtonToNum(0);
  chanCopy->IsEditable();
  chanCopy->SetIntNumber(0);
  CopyButton->AddFrame(Copy,
		       new TGLayoutHints(kLHintsCenterX, 5, 10, 3, 0));
  CopyButton->AddFrame(chanCopy,
		       new TGLayoutHints(kLHintsTop | 
					 kLHintsLeft, 0, 20, 0, 0));
  
  chanCopy->Associate(this);
  
  /********** Copy button per se **********/
  TGTextButton *copyB = new TGTextButton(CopyButton, "C&opy", 
					 COPYBUTTON + 1000);
  copyB->Associate(this);
  copyB->SetToolTipText
    ("Copy the setup of the selected channel to all channels of the module",
     0);
  CopyButton->AddFrame(copyB,
		       new TGLayoutHints(kLHintsTop | 
					 kLHintsLeft, 0, 20, 0, 0));
  modNumber = 0;
  chanNumber = 0;
  decay = 0;
  

  MapSubwindows();
  Resize(); // Resize to default size

}

Bool_t ScopedT::ProcessMessage(Long_t msg, Long_t parm1, Long_t parm2)
{
  switch (GET_MSG(msg)) {
  case kC_COMMAND:
    switch (GET_SUBMSG(msg)) {
    case kCM_BUTTON:
      switch (parm1) {
      case (MODNUMBER):
	if (parm2 == 0) {
	  if (modNumber != 13) {
	    ++modNumber;
	    numericMod->SetIntNumber(modNumber);
	  }
	} else {
	  if (modNumber != 0) {
	    if (--modNumber == 0) {
	      modNumber = 0;
	    }
	    numericMod->SetIntNumber(modNumber);
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


      case LOAD:
	{
	  Load_Once = true;
	  load_info(modNumber);
	}
	break;
      

      case APPLY:
	if (Load_Once) {
	  change_values(modNumber);
	} else {
	  std::cout << "Please load once first !\n";
	}
	break;
     

      case CANCEL: // Cancel Button
	DeleteWindow();
	break;

	
      case (COPYBUTTON + 1000):
	decay = NumEntry[1][chanNumber]->GetNumber();
	for (int i = 0; i < 16; i++) {
	  if (i != chanNumber - 1) {
	    sprintf(tmp, "%1.3f", decay);
	    NumEntry[1][i]->SetText(tmp);
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

int ScopedT::load_info(Long_t mod)
{
  double ChanParData = -1;
  int retval;
  char text[20];
  char pXDT[]="XDT";
  for (int i = 0; i < 16; i++) {
    retval = Pixie16ReadSglChanPar(/*"XDT"*/pXDT, &ChanParData, modNumber, i);
    sprintf(text, "%1.3f", ChanParData);
    NumEntry[1][i]->SetText(text);
  }
  return retval;
}

int ScopedT::change_values(Long_t mod)
{
  char pXDT[]="XDT";
  double d;
  for (int i = 0; i < 16; i++) {
    d = NumEntry[1][i]->GetNumber();
    Pixie16WriteSglChanPar(/*"XDT"*/pXDT, d, modNumber, i);
  }
  return 1;
}
