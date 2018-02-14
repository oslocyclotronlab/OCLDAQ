#include "AnalogSignal.h"

AnalogSignal::AnalogSignal(const TGWindow * p, const TGWindow * main,
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
  CLabel[0]->SetText("DCOffset");
  CLabel[0]->SetAlignment(kTextCenterX);

  ColumnGain = new TGVerticalFrame(mn, 200, 300);
  mn->AddFrame(ColumnGain,
	       new TGLayoutHints(kLHintsTop | kLHintsLeft, 0, 0, 0, 0));

  TGTextEntry *LabelGain =
      new TGTextEntry(ColumnGain, new TGTextBuffer(100), 10000,
		      LabelGain->GetDefaultGC()(),
		      LabelGain->GetDefaultFontStruct(),
		      kRaisedFrame | kDoubleBorder, GetWhitePixel());
  LabelGain->
      SetFont("-adobe-helvetica-bold-r-*-*-10-*-*-*-*-*-iso8859-1", false);

  LabelGain->Resize(40, 20);
  LabelGain->SetEnabled(kFALSE);
  LabelGain->SetFrameDrawn(kTRUE);
  ColumnGain->AddFrame(LabelGain,
		       new TGLayoutHints(kLHintsCenterX, 0, 0, 10, 0));
  LabelGain->SetText("Gain");
  for (int i = 0; i < 16; i++) {

    ColumnGain->AddFrame(lstBoxGain[i] =
			 new TGComboBox(ColumnGain, 2755 + i),
			 new TGLayoutHints(kLHintsCenterX, 0, 0, 0, 0));
    lstBoxGain[i]->Associate(this);
    lstBoxGain[i]->AddEntry("4.0", 0);
    lstBoxGain[i]->AddEntry("0.9", 1);
    lstBoxGain[i]->Resize(40, 20);
  }

  ColumnSign = new TGVerticalFrame(mn, 200, 300);
  mn->AddFrame(ColumnSign,
	       new TGLayoutHints(kLHintsTop | kLHintsLeft, 0, 0, 0, 0));

  TGTextEntry *LabelSign =
      new TGTextEntry(ColumnSign, new TGTextBuffer(100), 10000,
		      LabelSign->GetDefaultGC()(),
		      LabelSign->GetDefaultFontStruct(),
		      kRaisedFrame | kDoubleBorder, GetWhitePixel());


  LabelSign->
      SetFont("-adobe-helvetica-bold-r-*-*-10-*-*-*-*-*-iso8859-1", false);

  LabelSign->Resize(40, 20);
  LabelSign->SetEnabled(kFALSE);
  LabelSign->SetFrameDrawn(kTRUE);
  ColumnSign->AddFrame(LabelSign,
		       new TGLayoutHints(kLHintsCenterX, 0, 0, 10, 0));
  LabelSign->SetText("Sign");

  for (int i = 0; i < 16; i++) {

    ColumnSign->AddFrame(lstBox[i] = new TGComboBox(ColumnSign, 2555 + i),
			 new TGLayoutHints(kLHintsCenterX, 0, 0, 0, 0));
    lstBox[i]->Associate(this);

    lstBox[i]->AddEntry("+", 0);
    lstBox[i]->AddEntry("-", 1);
    lstBox[i]->Resize(40, 20);
  }
////////////////Copy Button//////////////////////////////////////////////
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


  chanNumber = 0;

  TGTextButton *AdjOff = new TGTextButton(CopyButton, "Adjust Offsets", COPYBUTTON + 2000);
  AdjOff->Associate(this);
  AdjOff->SetToolTipText("Automatically adjust the offsets in this module",0);
  CopyButton->AddFrame(AdjOff,
		       new TGLayoutHints(kLHintsTop | kLHintsLeft, 0, 20,
					 0, 0));
///////////////////////////////////////////////////////////////////////
  MapSubwindows();
  Resize();			// resize to default size

  modNumber = 0;
  Load_Once = true;
  pol_temp = 0;
  gain_temp = 0;
  offset_temp = 0;
}

/////////////////////////////////////////////////////////////////////////////////

AnalogSignal::~AnalogSignal()
{
}

/////////////////////////////////////////////////////////////////////////////////

Bool_t AnalogSignal::ProcessMessage(Long_t msg, Long_t parm1, Long_t parm2)
{
  int val = 0;

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
	    load_info(chanNumber);
	  }
	} else {
	  if (modNumber != 0) {
	    if (--modNumber == 0)
	      modNumber = 0;
	    numericMod->SetIntNumber(modNumber);
	    load_info(chanNumber);
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
	pol_temp = lstBox[chanNumber]->GetSelected();
	gain_temp = lstBoxGain[chanNumber]->GetSelected();
	offset_temp = NumEntry[1][chanNumber]->GetNumber();
	cout << pol_temp << " " << gain_temp << " " << offset_temp << endl;
	for (int i = 0; i < 16; i++) {
	  if (i != chanNumber) {
	    lstBox[i]->Select(pol_temp);
	    lstBoxGain[i]->Select(gain_temp);
	    char tmp[10];
	    sprintf(tmp, "%1.3f", offset_temp);
	    NumEntry[1][i]->SetText(tmp);
	  }
	}
	break;

      case (COPYBUTTON + 2000):
	val = Pixie16AdjustOffsets(modNumber);
	if(val < 0) cout << "Addjust offsets failed " << val << endl;
	load_info(modNumber);
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

////////////////////////////////////////////////////////////////////////////////
int AnalogSignal::load_info(Long_t mod)
{
  double ChanParData = -1;

  int retval;
  char text[20];
  unsigned short gain;
  unsigned short pol;
  char param_CHANNEL_CSRA[] = "CHANNEL_CSRA";
  char param_VOFFSET[] = "VOFFSET";

  //? Check to see if cards are booted here -- DTM (1/09)

  for (int i = 0; i < 16; i++) {
    retval =
      Pixie16ReadSglChanPar(/*"CHANNEL_CSRA"*/param_CHANNEL_CSRA, &ChanParData, modNumber, i);
    gain = APP32_TstBit(14, (unsigned long) ChanParData);
    if (gain == 1)
      lstBoxGain[i]->Select(0);
    else if (gain == 0)
      lstBoxGain[i]->Select(1);

    pol = APP32_TstBit(5, (unsigned long) ChanParData);
    if (pol == 1)
      lstBox[i]->Select(0);
    else if (pol == 0)
      lstBox[i]->Select(1);

    retval = Pixie16ReadSglChanPar(/*"VOFFSET"*/param_VOFFSET, &ChanParData, modNumber, i);
    sprintf(text, "%1.3f", ChanParData);
    NumEntry[1][i]->SetText(text);

  }
  return retval;
}

////////////////////////////////////////////////////////////////////////////////
int AnalogSignal::change_values(Long_t mod)
{

  double offset;
  double ChanParData = -1;
  double gain;
  int pol = 0;
  char param_CHANNEL_CSRA[] = "CHANNEL_CSRA";
  char param_VOFFSET[] = "VOFFSET";

  for (int i = 0; i < 16; i++) {
    offset = NumEntry[1][i]->GetNumber();

    Pixie16WriteSglChanPar(/*"VOFFSET"*/param_VOFFSET, offset, modNumber, i);
    Pixie16ReadSglChanPar(/*"CHANNEL_CSRA"*/param_CHANNEL_CSRA, &ChanParData, modNumber, i);
    pol = lstBox[i]->GetSelected();
    if (pol == 0) {
      ChanParData = APP32_SetBit(5, (unsigned long) ChanParData);
      Pixie16WriteSglChanPar(/*"CHANNEL_CSRA"*/param_CHANNEL_CSRA, ChanParData, modNumber, i);
    } else {
      ChanParData = APP32_ClrBit(5, (unsigned long) ChanParData);
      Pixie16WriteSglChanPar(/*"CHANNEL_CSRA"*/param_CHANNEL_CSRA, ChanParData, modNumber, i);
    }

    Pixie16ReadSglChanPar(/*"CHANNEL_CSRA"*/param_CHANNEL_CSRA, &ChanParData, modNumber, i);

    gain = lstBoxGain[i]->GetSelected();
    if (gain == 1) {
      ChanParData = APP32_ClrBit(14, (unsigned long) ChanParData);
      Pixie16WriteSglChanPar(/*"CHANNEL_CSRA"*/param_CHANNEL_CSRA, ChanParData, modNumber, i);
    } else {
      ChanParData = APP32_SetBit(14, (unsigned long) ChanParData);
      Pixie16WriteSglChanPar(/*"CHANNEL_CSRA"*/param_CHANNEL_CSRA, ChanParData, modNumber, i);
    }
  }
  return 1;
}
