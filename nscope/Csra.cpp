// Csra.cpp
//
// Author : Sean Liddick
// Modified by: Jeromy Tompkins
//

#include "Csra.h"
#include "GuiTypes.h"
#include "TGWidget.h"
#include "TG3DLine.h"
#include "pixie16app_export.h"
#include <iostream>
#include <stdint.h>

using namespace std;

Csra::Csra (const TGWindow * p, const TGWindow * main, int NumModules):
TGTransientFrame (p, main, 10, 10, kHorizontalFrame), fMultDialogue(0)
{
  DontCallClose();
  SetMWMHints(kMWMDecorBorder|kMWMDecorTitle,kMWMFuncResize|kMWMFuncMove, 0);
  SetCleanup (kDeepCleanup);
  module_number1 = 0;
  char name[20];

  numModules=NumModules;

  mn_vert = new TGVerticalFrame (this, 200, 300);
  mn = new TGHorizontalFrame (mn_vert, 200, 300);
  mn_vert->AddFrame (mn,
		     new TGLayoutHints (kLHintsTop | kLHintsLeft, 0, 0, 0,
					0));
  AddFrame (mn_vert,
	    new TGLayoutHints (kLHintsTop | kLHintsLeft, 2, 2, 2, 2));
  column1 = new TGVerticalFrame (mn, 200, 300);
  column2 = new TGVerticalFrame (mn, 200, 300);
  column3 = new TGVerticalFrame (mn, 200, 300);
  column4 = new TGVerticalFrame (mn, 200, 300);
  column5 = new TGVerticalFrame (mn, 400, 300);
  column6 = new TGVerticalFrame (mn, 400, 300);
  column7 = new TGVerticalFrame (mn, 400, 300);
  column8 = new TGVerticalFrame (mn, 400, 300);
  column9 = new TGVerticalFrame (mn, 400, 300);
  column10 = new TGVerticalFrame (mn, 400, 300);
  column11 = new TGVerticalFrame (mn, 400, 300);
  column12 = new TGVerticalFrame (mn, 400, 300);
  column13 = new TGVerticalFrame (mn, 400, 300);
  column14 = new TGVerticalFrame (mn, 400, 300);
  column15 = new TGVerticalFrame (mn, 400, 300);
  column16 = new TGVerticalFrame (mn, 400, 300);
  column17 = new TGVerticalFrame (mn, 400, 300);
  column18 = new TGVerticalFrame (mn, 400, 300);

  buttons = new TGHorizontalFrame (mn_vert, 400, 300);

  mn->AddFrame (column1,
		new TGLayoutHints (kLHintsTop | kLHintsLeft, 0, 0, 0, 0));
  mn->AddFrame (column2,
		new TGLayoutHints (kLHintsTop | kLHintsLeft, 0, 0, 0, 0));
  mn->AddFrame (column3,
		new TGLayoutHints (kLHintsTop | kLHintsLeft, 0, 0, 0, 0));
  mn->AddFrame (column4,
		new TGLayoutHints (kLHintsTop | kLHintsLeft, 0, 0, 0, 0));
  mn->AddFrame (column5,
		new TGLayoutHints (kLHintsTop | kLHintsLeft, 0, 0, 0, 0));
  mn->AddFrame (column6,
		new TGLayoutHints (kLHintsTop | kLHintsLeft, 0, 0, 0, 0));
  mn->AddFrame (column7,
		new TGLayoutHints (kLHintsTop | kLHintsLeft, 0, 0, 0, 0));
  mn->AddFrame (column8,
		new TGLayoutHints (kLHintsTop | kLHintsLeft, 0, 0, 0, 0));
  mn->AddFrame (column9,
 		new TGLayoutHints (kLHintsTop | kLHintsLeft, 0, 0, 0, 0));
  mn->AddFrame (column10,
 		new TGLayoutHints (kLHintsTop | kLHintsLeft, 0, 0, 0, 0));
  mn->AddFrame (column11,
 		new TGLayoutHints (kLHintsTop | kLHintsLeft, 0, 0, 0, 0));
  mn->AddFrame (column12,
 		new TGLayoutHints (kLHintsTop | kLHintsLeft, 0, 0, 0, 0));
  mn->AddFrame (column13,
 		new TGLayoutHints (kLHintsTop | kLHintsLeft, 0, 0, 0, 0));
  mn->AddFrame (column14,
 		new TGLayoutHints (kLHintsTop | kLHintsLeft, 0, 0, 0, 0));
  mn->AddFrame (column15,
 		new TGLayoutHints (kLHintsTop | kLHintsLeft, 0, 0, 0, 0));
  mn->AddFrame (column16,
 		new TGLayoutHints (kLHintsTop | kLHintsLeft, 0, 0, 0, 0));
  mn->AddFrame (column17,
 		new TGLayoutHints (kLHintsTop | kLHintsLeft, 0, 0, 0, 0));
  mn->AddFrame (column18,
 		new TGLayoutHints (kLHintsTop | kLHintsLeft, 0, 0, 0, 0));

  cout << "added columns" << endl;
//////////////////////////first column////////////////////////


  TGTextEntry *te = new TGTextEntry (column1, new TGTextBuffer (100), 10000,
				     te->GetDefaultGC ()(),
				     te->GetDefaultFontStruct (),
				     kRaisedFrame | kDoubleBorder,
				     GetWhitePixel ());

  te->SetText ("Ch #");
  te->Resize (35, 20);
  te->SetEnabled (kFALSE);
  te->SetFrameDrawn (kTRUE);
  column1->AddFrame (te, new TGLayoutHints (kLHintsCenterX, 0, 0, 10, 0));

  TGTextEntry *Labels[17];

  for (int i = 0; i < 17; i++)
    {
      if(i != 16){
	sprintf (name, "%d", i);
      }
      else {
	sprintf (name, "All");
      }
      Labels[i] = new TGTextEntry (column1, new TGTextBuffer (100), 10000,
				   Labels[i]->GetDefaultGC ()(),
				   Labels[i]->GetDefaultFontStruct (),
				   kRaisedFrame | kDoubleBorder,
				   GetWhitePixel ());
      Labels[i]->SetText (name);
      Labels[i]->Resize (35, 20);
      Labels[i]->SetEnabled (kFALSE);
      Labels[i]->SetFrameDrawn (kTRUE);

//      Labels[i] = new TGLabel (column1, name);
      column1->AddFrame (Labels[i],
			 new TGLayoutHints (kLHintsCenterX, 0, 3, 0, 0));
      
      if(i == 15){
	//add horizontal line after all channel numbers
	TGHorizontal3DLine *line = new TGHorizontal3DLine(column1);
	column1->AddFrame(line, new TGLayoutHints(kLHintsTop | kLHintsExpandX));
      }
    }

  cout << "added labels " << endl;

  make_columns (column2, 0, ckBtn[0], "TSel", "Fast trigger selection, 0 - local channel trigger, 1 - external trigger",2000);
  make_columns (column3, 1, ckBtn[1], "MIL", "Module validation signal selection, 0 - local validation from system FPGA, 1 - GATE from front panel",3000);
  make_columns (column4, 2, ckBtn[2], "GC", "Good channel", 4000);
  make_columns (column5, 3 ,ckBtn[3], "ChTr", "Channel validation signal selection, 0 - local validation from system FPGA, 1 - GATE from front panel", 5000);
  make_columns (column6, 4, ckBtn[4], "Bl", "Block data acquisition if trace or header DPMs are full", 6000);
  make_columns (column7, 5, ckBtn[5], "PL", "Input signal polarity, 0 - trigger on negative signal, 1 - trigger on a positive signal", 7000);
  make_columns (column8, 6, ckBtn[6], "VE", "Enable channel trigger veto", 8000);
  make_columns (column9, 7, ckBtn[7], "HE", "Histogram Energy", 9000);
  make_columns (column10,8,  ckBtn[8], "TR", "enable trace capture", 9100);
  make_columns (column11,9,  ckBtn[9], "QD", "enable QDC sums capture", 9200);
  make_columns (column12,10, ckBtn[10], "CF", "enable CFD triggering", 9300);
  make_columns (column13,11, ckBtn[11], "GV", "Enable global trigger validation, 1 - coincidence required between fast trigger and external trigger", 9400);
  make_columns (column14,12, ckBtn[12], "RE", "capture raw energies and baselines", 9500);
  make_columns (column15,13, ckBtn[13], "CT", "Enable channel validation trigger, 1 - channels defined by multiplicity mask must be in coincidence ", 9600);
  make_columns (column16,14, ckBtn[14], "GA", "Gain setting", 9700);
  make_columns (column17,15, ckBtn[15], "PR1", "enable normal pileup rejection, PR1 - 0 and PR2 - 0 - record everything for all events, PR1 - 1 and PR2 - 0 - only record everything for single events (reject pileup), PR1 - 0 and PR2 - 1 - record everything for piledup events but do not record trace for single events, PR1 - 1 and PR2 - 1 - only record everything for piledup events", 9750);
  make_columns (column18,16, ckBtn[16], "PR2", "enable inverse pileup rejection,  PR1 - 0 and PR2 - 0 - record everything for all events, PR1 - 1 and PR2 - 0 - only record everything for single events (reject pileup), PR1 - 0 and PR2 - 1 - record everything for piledup events but do not record trace for single events, PR1 - 1 and PR2 - 1 - only record everything for piledup events", 9770);


  cout << "made columns " << endl;
/////////////////////////////module entry///////////////////////////////

  TGHorizontal3DLine *ln1 = new TGHorizontal3DLine (column1, 50, 2);
  TGLabel *mod = new TGLabel (buttons, "Module #");

  numericMod = new TGNumberEntry (buttons, 0, 4, 100, (TGNumberFormat::EStyle) 0, (TGNumberFormat::EAttribute) 1, (TGNumberFormat::ELimit) 3,	// kNELLimitMinMax
				  0, 3);
  numericMod->SetButtonToNum (0);

  column1->AddFrame (ln1,
		     new TGLayoutHints (kLHintsCenterX | kLHintsCenterY, 0, 0,
					10, 10));
  buttons->AddFrame (mod, new TGLayoutHints (kLHintsCenterX, 5, 10, 3, 0));
  buttons->AddFrame (numericMod,
		     new TGLayoutHints (kLHintsTop | kLHintsLeft, 0, 20, 0,
					0));

  numericMod->Associate (this);
  mn_vert->AddFrame (buttons,
		     new TGLayoutHints (kLHintsTop | kLHintsLeft, 0, 0, 0,
					0));
/////////////////////////////////////////////////////////////////////////
////////////////////////////Buttons/////////////////////////////////////
  LoadButton = new TGTextButton (buttons, "&Load", 10000);
  LoadButton->Associate (this);
  ApplyButton = new TGTextButton (buttons, "&Apply", 10001);
  ApplyButton->Associate (this);
  CancelButton = new TGTextButton (buttons, "&Cancel", 10002);
  CancelButton->Associate (this);
  buttons->AddFrame (LoadButton,
		     new TGLayoutHints (kLHintsCenterX, 0, 0, 0, 0));
  buttons->AddFrame (ApplyButton,
		     new TGLayoutHints (kLHintsCenterX, 0, 0, 0, 0));
  buttons->AddFrame (CancelButton,
		     new TGLayoutHints (kLHintsCenterX, 0, 0, 0, 0));

  MapSubwindows ();
  Resize ();			// resize to default size
  CenterOnParent ();

  SetWindowName ("CSRA Register");

  MapWindow ();

}

Csra::~Csra ()
{
}
   
void Csra::RegisterMultCoincDialogue(MultCoincDialogue* mcd)
{
    fMultDialogue = mcd;
}


int
Csra::make_columns (TGVerticalFrame * column, Int_t c, TGCheckButton * ckBtn_g[18],
		    /*char *title*/std::string title, /*char *tooltip*/ std::string tooltip, int id)
{

  TGTextEntry *ra = new TGTextEntry (column, new TGTextBuffer (100),
				     10000, ra->GetDefaultGC ()(),
				     ra->GetDefaultFontStruct (),
				     kRaisedFrame | kDoubleBorder,
				     GetWhitePixel ());
  ra->SetText (title.c_str());
  ra->Resize (35, 20);
  ra->SetEnabled (kFALSE);
  ra->SetFrameDrawn (kTRUE);
  ra->SetAlignment (kTextCenterX);
  ra->SetToolTipText (tooltip.c_str(), 0);

  column->AddFrame (ra, new TGLayoutHints (kLHintsCenterX, 0, 0, 10, 3));

  column->AddFrame (ckBtn_g[0] = new TGCheckButton (column, "", id),
		    new TGLayoutHints (kLHintsCenterX, 0, 0, 0, 0));
  ckBtn_g[0]->Associate (this);

  for (int i = 1; i < /*16*/17; i++)
    {
      column->AddFrame (ckBtn_g[i] =
			new TGCheckButton (column, "", id + i),
			new TGLayoutHints (kLHintsCenterX, 0, 0, 3, 0));
      ckBtn_g[i]->Associate (this);

      if(i == 15){
	TGHorizontal3DLine *line = new TGHorizontal3DLine(column);
	column->AddFrame(line, new TGLayoutHints(kLHintsTop | kLHintsExpandX));
      }
    }
  return 1;
}


/////////////////////////process message//////////////////////////////////

Bool_t
Csra::ProcessMessage (Long_t msg, Long_t parm1, Long_t parm2)
{

  switch (GET_MSG (msg))
    {
    case kC_COMMAND:

      switch (GET_SUBMSG (msg))
	{
	case kC_TEXTENTRY:
	  switch(parm1)
	    {
	    case 2016:

	      CheckAll(0,ckBtn[0][16]->IsDown());
	      break;
	    case 3016:
	      CheckAll(1,ckBtn[1][16]->IsDown());
	      break;
	    case 4016:
	      CheckAll(2,ckBtn[2][16]->IsDown());
	      break;
	    case 5016:
	      CheckAll(3,ckBtn[3][16]->IsDown());
	      break;
	    case 6016:
	      CheckAll(4,ckBtn[4][16]->IsDown());
	      break;
	    case 7016:
	      CheckAll(5,ckBtn[5][16]->IsDown());
	      break;
	    case 8016:
	      CheckAll(6,ckBtn[6][16]->IsDown());
	      break;
	    case 9016:
	      CheckAll(7,ckBtn[7][16]->IsDown());
	      break;
	    case 9116:
	      CheckAll(8,ckBtn[8][16]->IsDown());
	      break;
	    case 9216:
	      CheckAll(9,ckBtn[9][16]->IsDown());
	      break;
	    case 9316:
	      CheckAll(10,ckBtn[10][16]->IsDown());
	      break;
	    case 9416:
	      CheckAll(11,ckBtn[11][16]->IsDown());
	      break;
	    case 9516:
	      CheckAll(12,ckBtn[12][16]->IsDown());
	      break;
	    case 9616:
	      CheckAll(13,ckBtn[13][16]->IsDown());
	      break;
	    case 9716:
	      CheckAll(14,ckBtn[14][16]->IsDown());
	      break;
	    case 9766:
	      CheckAll(15,ckBtn[15][16]->IsDown());
	      break;
	    case 9786:
	      CheckAll(16,ckBtn[16][16]->IsDown());
	      break;
	    default:
	      break;
	    }
	case kCM_BUTTON:

	  switch (parm1)
	    {
	    case (100):
	      if (parm2 == 0)
		{
		  if (module_number1 != numModules-1)
		    {
		      ++module_number1;
		      numericMod->SetIntNumber (module_number1);
              load_info(module_number1, false);
		    }
		}
	      else
		{
		  if (module_number1 != 0)
		    {
		      if (--module_number1 == 0)
			module_number1 = 0;
		      numericMod->SetIntNumber (module_number1);
              load_info(module_number1, false);
		    }
		}
	      break;
	    case 10000:
	      {
		Load_Once = true;
		load_info (module_number1);
	      }
	      break;
	    case 10001:
	      if (Load_Once)
		change_values (module_number1);
	      else
		std::cout << "please load once first !\n";

          load_info(module_number1, false);
          OnBitsChanged();
	      break;
	    case 10002:		/// Cancel Button
          //    DeleteWindow ();
          UnmapWindow();
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

void Csra::OnBitsChanged()
{
    if (fMultDialogue!=0) {
        // If the Csra changed the bits for the module displayed by
        // fMultDialogue, send an update signal
        if (fMultDialogue->GetModuleNumber()==module_number1) {
            // check how many of the 14th bit (i.e. bit 13)
            // that are set.
            int nset_channels = GetNBitsOn(module_number1,13);

            // Send the number of channels set to the MultDialogue
            fMultDialogue->UpdateCSRAState(nset_channels);
        }
    }
}


int Csra::CheckAll(Int_t column, Bool_t isdown)
{

  for(Int_t i=0; i<16;i++){
    if(isdown){
      ckBtn[column][i]->SetState(kButtonDown);
    }
    else{
      ckBtn[column][i]->SetState(kButtonUp);
    }

  }
  return 1;
}

int Csra::GetNBitsOn(unsigned int module, unsigned int bit)
{
  double ChanParData = -1;
  int retval;

  uint32_t bitpattern = (0x1<<bit);
  //string param("CHANNEL_CSRA");
  //vector<char> CHANNEL_CSRA(param.begin(),param.end());
  //CHANNEL_CSRA.push_back('\0');

  char param[] = "CHANNEL_CSRA";

  int n=0;
  for(Int_t i=0; i<16;i++){	
     retval = Pixie16ReadSglChanPar (param, &ChanParData, module,i);

     if(retval<0) std::cout << "Reading " << param << " from GetNBitsON in Csra failed" << std::endl;
      
     uint32_t chanbits = static_cast<uint32_t>(ChanParData);
     if ( (chanbits&bitpattern) != 0) {
         ++n;
     }
    
  }
  return n;

}

int
Csra::load_info (Long_t mod, Bool_t verbose)
{
  std::cout << "Loading CSRA data from module " << module_number1 << std::endl;
 
  double ChanParData = -1;
  //uint32_t ChanParData = 0;
  int retval;
  unsigned short gt;
  char param[] = "CHANNEL_CSRA";

  for (int i = 0; i < 16; i++)
    {
      retval =
	Pixie16ReadSglChanPar (param, &ChanParData, module_number1,
			       i);
    if (verbose)
        cout << "Channel " << i << " " <<ChanParData << endl;

      //////////////// test gt///////////////
      gt = APP16_TstBit (0, (unsigned long) ChanParData);
      if (gt == 0)
	ckBtn[0][i]->SetState (kButtonUp);
      else
	ckBtn[0][i]->SetState (kButtonDown);
      gt = 0;
      ///////////////test mil/////////////////////////
      gt = APP32_TstBit (1, (unsigned long) ChanParData);
      if (gt == 0)
	ckBtn[1][i]->SetState (kButtonUp);
      else
	ckBtn[1][i]->SetState (kButtonDown);
      gt = 0;
      ////////////////test gc////////////////////////////
      gt = APP32_TstBit (2, (unsigned long) ChanParData);
      if (gt == 0)
	ckBtn[2][i]->SetState (kButtonUp);
      else
	ckBtn[2][i]->SetState (kButtonDown);
      gt = 0;
      /////////////////test ra///////////////////////////
      gt = APP32_TstBit (3, (unsigned long) ChanParData);
      if (gt == 0)
	ckBtn[3][i]->SetState (kButtonUp);
      else
	ckBtn[3][i]->SetState (kButtonDown);
      gt = 0;
      //////////////////test ea//////////////////////////
      gt = APP32_TstBit (4, (unsigned long) ChanParData);
      if (gt == 0)
	ckBtn[4][i]->SetState (kButtonUp);
      else
	ckBtn[4][i]->SetState (kButtonDown);
      gt = 0;
      //////////////////test ha///////////////////////////
      gt = APP32_TstBit (5, (unsigned long) ChanParData);
      if (gt == 0)
	ckBtn[5][i]->SetState (kButtonUp);
      else
	ckBtn[5][i]->SetState (kButtonDown);
      gt = 0;
      /////////////////test gt//////////////////////////////
      gt = APP32_TstBit (6, (unsigned long) ChanParData);
      if (gt == 0)
	ckBtn[6][i]->SetState (kButtonUp);
      else
	ckBtn[6][i]->SetState (kButtonDown);
      gt = 0;
      /////////////////test gt//////////////////////////////
      gt = APP32_TstBit (7, (unsigned long) ChanParData);
      if (gt == 0)
	ckBtn[7][i]->SetState (kButtonUp);
      else
	ckBtn[7][i]->SetState (kButtonDown);
      gt = 0;
      /////////////////test gt//////////////////////////////
      gt = APP32_TstBit (8, (unsigned long) ChanParData);
      if (gt == 0)
	ckBtn[8][i]->SetState (kButtonUp);
      else
	ckBtn[8][i]->SetState (kButtonDown);
      gt = 0;
      /////////////////test gt//////////////////////////////
      gt = APP32_TstBit (9, (unsigned long) ChanParData);
      if (gt == 0)
	ckBtn[9][i]->SetState (kButtonUp);
      else
	ckBtn[9][i]->SetState (kButtonDown);
      gt = 0;
      /////////////////test gt//////////////////////////////
      gt = APP32_TstBit (10, (unsigned long) ChanParData);
      if (gt == 0)
	ckBtn[10][i]->SetState (kButtonUp);
      else
	ckBtn[10][i]->SetState (kButtonDown);
      gt = 0;
      /////////////////test gt//////////////////////////////
      gt = APP32_TstBit (11, (unsigned long) ChanParData);
      if (gt == 0)
	ckBtn[11][i]->SetState (kButtonUp);
      else
	ckBtn[11][i]->SetState (kButtonDown);
      gt = 0;
      /////////////////test gt//////////////////////////////
      gt = APP32_TstBit (12, (unsigned long) ChanParData);
      if (gt == 0)
	ckBtn[12][i]->SetState (kButtonUp);
      else
	ckBtn[12][i]->SetState (kButtonDown);
      gt = 0;
      /////////////////test gt//////////////////////////////
      gt = APP32_TstBit (13, (unsigned long) ChanParData);
      if (gt == 0)
	ckBtn[13][i]->SetState (kButtonUp);
      else
	ckBtn[13][i]->SetState (kButtonDown);
      gt = 0;
      /////////////////test gt//////////////////////////////
      gt = APP32_TstBit (14, (unsigned long) ChanParData);
      if (gt == 0)
	ckBtn[14][i]->SetState (kButtonUp);
      else
	ckBtn[14][i]->SetState (kButtonDown);
      gt = 0;
      /////////////////test gt//////////////////////////////
      gt = APP32_TstBit (15, (unsigned long) ChanParData);
      if (gt == 0)
	ckBtn[15][i]->SetState (kButtonUp);
      else
	ckBtn[15][i]->SetState (kButtonDown);
      gt = 0;
      /////////////////test gt//////////////////////////////
      gt = APP32_TstBit (16, (unsigned long) ChanParData);
      if (gt == 0)
	ckBtn[16][i]->SetState (kButtonUp);
      else
	ckBtn[16][i]->SetState (kButtonDown);
      gt = 0;

      if (verbose)
          cout << "finished bit checking " << endl;
 
    }

  // given that this function is completed, Load_Once is now true.
  Load_Once = true;

  //std::cout << "loading info\n";
  return retval;
}

int
Csra::change_values (Long_t mod)
{
  char param[] = "CHANNEL_CSRA";
  double ChanParData = -1;
  //int retval;

  std::cout << "Applying the following CSRA bits to module " << module_number1 << std::endl;
  std::cout << "\n" << std::setw(4) << "Ch#" << " " << std::setw(12) << "Bit Mask" << std::endl;
  std::cout << std::setfill('-');
  std::cout << std::setw(4) << '-' << " " << std::setw(12) << '-' << std::endl;
  std::cout << std::setfill(' ');

  for (int i = 0; i < 16; i++)
    {
      Pixie16ReadSglChanPar (param/*"CHANNEL_CSRA"*/, &ChanParData, module_number1, i);
      if (ckBtn[0][i]->IsDown())
	ChanParData = APP32_SetBit (0, (unsigned short) ChanParData);
      else
	ChanParData = APP32_ClrBit (0, (unsigned short) ChanParData);

      if (ckBtn[1][i]->IsDown())
	ChanParData = APP32_SetBit (1, (unsigned short) ChanParData);
      else
	ChanParData = APP32_ClrBit (1, (unsigned short) ChanParData);

      if (ckBtn[2][i] ->IsDown())
	ChanParData = APP32_SetBit (2, (unsigned short) ChanParData);
      else
	ChanParData = APP32_ClrBit (2, (unsigned short) ChanParData);
 
     if (ckBtn[3][i] ->IsDown())
	ChanParData = APP32_SetBit (3, (unsigned short) ChanParData);
      else
	ChanParData = APP32_ClrBit (3, (unsigned short) ChanParData);

      if (ckBtn[4][i] ->IsDown())
	ChanParData = APP32_SetBit (4, (unsigned short) ChanParData);
      else
	ChanParData = APP32_ClrBit (4, (unsigned short) ChanParData);

      if (ckBtn[5][i] ->IsDown())
	ChanParData = APP32_SetBit (5, (unsigned short) ChanParData);
      else
	ChanParData = APP32_ClrBit (5, (unsigned short) ChanParData);

    if (ckBtn[6][i] ->IsDown())
	ChanParData = APP32_SetBit (6, (unsigned short) ChanParData);
      else
	ChanParData = APP32_ClrBit (6, (unsigned short) ChanParData);

    if (ckBtn[7][i] ->IsDown())
	ChanParData = APP32_SetBit (7, (unsigned short) ChanParData);
      else
	ChanParData = APP32_ClrBit (7, (unsigned short) ChanParData);

    if (ckBtn[8][i] ->IsDown())
	ChanParData = APP32_SetBit (8, (unsigned short) ChanParData);
      else
	ChanParData = APP32_ClrBit (8, (unsigned short) ChanParData);

    if (ckBtn[9][i] ->IsDown())
	ChanParData = APP32_SetBit (9, (unsigned short) ChanParData);
      else
	ChanParData = APP32_ClrBit (9, (unsigned short) ChanParData);

    if (ckBtn[10][i] ->IsDown())
	ChanParData = APP32_SetBit (10, (unsigned short) ChanParData);
      else
	ChanParData = APP32_ClrBit (10, (unsigned short) ChanParData);

    if (ckBtn[11][i] ->IsDown())
	ChanParData = APP32_SetBit (11, (unsigned short) ChanParData);
      else
	ChanParData = APP32_ClrBit (11, (unsigned short) ChanParData);

    if (ckBtn[12][i] ->IsDown())
	ChanParData = APP32_SetBit (12, (unsigned short) ChanParData);
      else
	ChanParData = APP32_ClrBit (12, (unsigned short) ChanParData);

    if (ckBtn[13][i] ->IsDown())
	ChanParData = APP32_SetBit (13, (unsigned short) ChanParData);
      else
	ChanParData = APP32_ClrBit (13, (unsigned short) ChanParData);

    if (ckBtn[14][i] ->IsDown())
	ChanParData = APP32_SetBit (14, (unsigned short) ChanParData);
      else
	ChanParData = APP32_ClrBit (14, (unsigned short) ChanParData);

    if (ckBtn[15][i] ->IsDown())
	ChanParData = APP32_SetBit (15, (unsigned short) ChanParData);
      else
	ChanParData = APP32_ClrBit (15, (unsigned short) ChanParData);

    if (ckBtn[16][i] ->IsDown())
	ChanParData = APP32_SetBit (16, (unsigned short) ChanParData);
      else
	ChanParData = APP32_ClrBit (16, (unsigned short) ChanParData);

      std::cout << std::setw(4) << i << " " 
          << std::hex << std::showbase   << std::setw(12) << (unsigned short) ChanParData 
          << std::dec << std::noshowbase << std::endl;
      Pixie16WriteSglChanPar (param/*"CHANNEL_CSRA"*/, ChanParData, module_number1, i);
    }
  
  return 1;
}
