#include "TrigConfig.h"
#include "GuiTypes.h"
#include "TGWidget.h"
#include "TG3DLine.h"
#include "pixie16app_export.h"
#include <iostream>

using namespace std;

TrigConfig::TrigConfig (const TGWindow * p, const TGWindow * main, int NumModules):
TGTransientFrame (p, main, 10, 10, kHorizontalFrame)
{
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
  column19 = new TGVerticalFrame (mn, 400, 300);
  column20 = new TGVerticalFrame (mn, 400, 300);
  column21 = new TGVerticalFrame (mn, 400, 300);
  column22 = new TGVerticalFrame (mn, 400, 300);
  column23 = new TGVerticalFrame (mn, 400, 300);
  column24 = new TGVerticalFrame (mn, 400, 300);
  column25 = new TGVerticalFrame (mn, 400, 300);

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
  mn->AddFrame (column19,
 		new TGLayoutHints (kLHintsTop | kLHintsLeft, 0, 0, 0, 0));
  mn->AddFrame (column20,
 		new TGLayoutHints (kLHintsTop | kLHintsLeft, 0, 0, 0, 0));
  mn->AddFrame (column21,
 		new TGLayoutHints (kLHintsTop | kLHintsLeft, 0, 0, 0, 0));
  mn->AddFrame (column22,
 		new TGLayoutHints (kLHintsTop | kLHintsLeft, 0, 0, 0, 0));
  mn->AddFrame (column23,
 		new TGLayoutHints (kLHintsTop | kLHintsLeft, 0, 0, 0, 0));
  mn->AddFrame (column24,
 		new TGLayoutHints (kLHintsTop | kLHintsLeft, 0, 0, 0, 0));
  mn->AddFrame (column25,
 		new TGLayoutHints (kLHintsTop | kLHintsLeft, 0, 0, 0, 0));


//////////////////////////first column////////////////////////


  TGTextEntry *te = new TGTextEntry (column1, new TGTextBuffer (100), 10000,
				     te->GetDefaultGC ()(),
				     te->GetDefaultFontStruct (),
				     kRaisedFrame | kDoubleBorder,
				     GetWhitePixel ());

  te->SetText ("Mod #");
  te->Resize (35, 20);
  te->SetEnabled (kFALSE);
  te->SetFrameDrawn (kTRUE);
  column1->AddFrame (te, new TGLayoutHints (kLHintsCenterX, 0, 0, 10, 0));

  TGTextEntry *Labels[16];

  // max 13 modules in one crate
  for (int i = 0; i < 13; i++)
    {
      sprintf (name, "%d", i);
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
    }

  make_columns (column2, ckBtn, "GFT", "Select Global Fast Trigger from Local Fast Trigger", 2000);
  make_columns (column3, ckBtn_1, "GFT", "Select Global Fast Trigger from Local Fast Trigger", 3000);
  make_columns (column4, ckBtn_2, "GFT", "Select Global Fast Trigger from Local Fast Trigger", 4000);
  make_columns (column5, ckBtn_3, "RSV", "Select Global Fast Trigger from Local Fast Trigger", 5000);
  make_columns (column6, ckBtn_4, "RSV", "Bit 5:4 - 0:0 3.3V I/O is global fast trigger, 0:1 internal fast trigger from one channel as global fast trigger, 1:0 internal fast trigger OR of 16 channels as global fast trigger, 1:1 front panel LVDS input as global fast trigger", 6000);
  make_columns (column7, ckBtn_5, "RSV", "Bit 5:4 - 0:0 3.3V I/O is global fast trigger, 0:1 internal fast trigger from one channel as global fast trigger, 1:0 internal fast trigger OR of 16 channels as global fast trigger, 1:1 front panel LVDS input as global fast trigger", 7000);
  make_columns (column8, ckBtn_6, "RSV", "Bit 7:6 - 0:0 3.3V I/O as global validation trigger, 0:1 internal fast trigger from one channel as global validation, 1:0 internal fast trigger OR of 16 channels as global validation, 1:1 front panel LVDS input as global validation", 8000);
  make_columns (column9, ckBtn_7, "RSV", "Bit 7:6 - 0:0 3.3V I/O as global validation trigger, 0:1 internal fast trigger from one channel as global validation, 1:0 internal fast trigger OR of 16 channels as global validation, 1:1 front panel LVDS input as global validation", 9000);
  make_columns (column10, ckBtn_8, "GVT", "Select Global Validation Trigger from local fast trigger", 9100);
  make_columns (column11, ckBtn_9, "GVT", "Select Global Validation Trigger from local fast trigger", 9200);
  make_columns (column12, ckBtn_10, "GVT", "Select Global Validation Trigger from local fast trigger", 9300);
  make_columns (column13, ckBtn_11, "GVT", "Select Global Validation Trigger from local fast trigger", 9400);
  make_columns (column14, ckBtn_12, "FpG", "Select group of FiPPI test signals to the system FPGA and then front panel", 9500);
  make_columns (column15, ckBtn_13, "FpG", "Select group of FiPPI test signals to the system FPGA and then front panel", 9600);
  make_columns (column16, ckBtn_14, "FpG", "Select group of FiPPI test signals to the system FPGA and then front panel", 9700);
  make_columns (column17, ckBtn_15, "FP", "connect front panel input/output", 9750);
  make_columns (column18, ckBtn_16, "FpS", "Select which channel local fast trigger signal to be sent to the sysmte FPGA and then to the front pannel on Fo0", 9750);
  make_columns (column19, ckBtn_17, "FpS", "Select which channel local fast trigger signal to be sent to the sysmte FPGA and then to the front pannel on Fo0", 10100);
  make_columns (column20, ckBtn_18, "FpS", "Select which channel local fast trigger signal to be sent to the sysmte FPGA and then to the front pannel on Fo0", 10200);
  make_columns (column21, ckBtn_19, "FpS", "Select which channel local fast trigger signal to be sent to the sysmte FPGA and then to the front pannel on Fo0", 10300);
  make_columns (column22, ckBtn_20, "F07", "Select system FPGA signal to be sent to Fo7, 1000 (decimal 8) sends OR of channel coincidences", 10300);
  make_columns (column23, ckBtn_21, "F07", "Select system FPGA signal to be sent to Fo7, 1000 (decimal 8) sends OR of channel coincidences", 10300);
  make_columns (column24, ckBtn_22, "F07", "Select system FPGA signal to be sent to Fo7, 1000 (decimal 8) sends OR of channel coincidences", 10300);
 
  make_columns (column25, ckBtn_23, "F07", "Select system FPGA signal to be sent to Fo7, 1000 (decimal 8) sends OR of channel coincidences", 10300);



/////////////////////////////module entry///////////////////////////////

  TGHorizontal3DLine *ln1 = new TGHorizontal3DLine (column1, 50, 2);
  // TGLabel *mod = new TGLabel (buttons, "Module #");

  // numericMod = new TGNumberEntry (buttons, 0, 4, 100, (TGNumberFormat::EStyle) 0, (TGNumberFormat::EAttribute) 1, (TGNumberFormat::ELimit) 3,	// kNELLimitMinMax
  // 				  0, 3);
  // numericMod->SetButtonToNum (0);

  column1->AddFrame (ln1,
		     new TGLayoutHints (kLHintsCenterX | kLHintsCenterY, 0, 0,
					10, 10));
  // buttons->AddFrame (mod, new TGLayoutHints (kLHintsCenterX, 5, 10, 3, 0));
  // buttons->AddFrame (numericMod,
  // 		     new TGLayoutHints (kLHintsTop | kLHintsLeft, 0, 20, 0,
  // 					0));

  // numericMod->Associate (this);
  mn_vert->AddFrame (buttons,
		     new TGLayoutHints (kLHintsTop | kLHintsLeft, 0, 0, 0,
					0));
/////////////////////////////////////////////////////////////////////////
////////////////////////////Buttons/////////////////////////////////////
  LoadButton = new TGTextButton (buttons, "&Load", 4000);
  LoadButton->Associate (this);
  ApplyButton = new TGTextButton (buttons, "&Apply", 4001);
  ApplyButton->Associate (this);
  CancelButton = new TGTextButton (buttons, "&Cancel", 4002);
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

  SetWindowName ("TrigConfig0");

  MapWindow ();

}

TrigConfig::~TrigConfig ()
{
}

int
TrigConfig::make_columns (TGVerticalFrame * column, TGCheckButton * ckBtn_g[17],
		       /*char **/std::string title, /*char **/std::string tooltip, int id)
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

  // column->AddFrame (ckBtn_g[0] = new TGCheckButton (column, "", id),
  // 		    new TGLayoutHints (kLHintsCenterX, 0, 0, 0, 0));
  // ckBtn_g[0]->Associate (this);

  //max 13 modules
  for (int i = 0; i < 13; i++)
    {
      column->AddFrame (ckBtn_g[i] =
			new TGCheckButton (column, "", id + i),
			new TGLayoutHints (kLHintsCenterX, 0, 0, 3, 0));
      if(i >= numModules) ckBtn_g[i]->SetState(kButtonDisabled);
      ckBtn_g[i]->Associate (this);
    }
  return 1;
}


/////////////////////////process message//////////////////////////////////

Bool_t
TrigConfig::ProcessMessage (Long_t msg, Long_t parm1, Long_t parm2)
{
  switch (GET_MSG (msg))
    {
    case kC_COMMAND:
      switch (GET_SUBMSG (msg))
	{
	case kCM_BUTTON:
	  switch (parm1)
	    {
	    // case (100):
	    //   if (parm2 == 0)
	    // 	{
	    // 	  if (module_number1 != numModules-1)
	    // 	    {
	    // 	      ++module_number1;
	    // 	      numericMod->SetIntNumber (module_number1);
	    // 	    }
	    // 	}
	    //   else
	    // 	{
	    // 	  if (module_number1 != 0)
	    // 	    {
	    // 	      if (--module_number1 == 0)
	    // 		module_number1 = 0;
	    // 	      numericMod->SetIntNumber (module_number1);
	    // 	    }
	    // 	}
	    //   break;
	    case 4000:
	      {
		Load_Once = true;
		load_info (module_number1);
	      }
	      break;
	    case 4001:
	      if (Load_Once)
		change_values (module_number1);
	      else
		std::cout << "please load once first !\n";
	      break;
	    case 4002:		/// Cancel Button
	      DeleteWindow ();
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

int
TrigConfig::load_info (Long_t mod)
{
  unsigned int ModParData;
  int retval;
  unsigned short gt;
  char pTrigConfig[]="TrigConfig0";

  for (int i = 0; i < numModules; i++)
    {
      retval =
	Pixie16ReadSglModPar (pTrigConfig, &ModParData, i/*module_number1*/);
      cout << "Channel " << i << " " <<ModParData << endl;
      //////////////// test gt///////////////
      gt = APP16_TstBit (0, (unsigned long) ModParData);
      if (gt == 0)
	ckBtn[i]->SetState (kButtonUp);
      else
	ckBtn[i]->SetState (kButtonDown);
      gt = 0;
      ///////////////test mil/////////////////////////
      gt = APP32_TstBit (1, (unsigned long) ModParData);
      if (gt == 0)
	ckBtn_1[i]->SetState (kButtonUp);
      else
	ckBtn_1[i]->SetState (kButtonDown);
      gt = 0;
      ////////////////test gc////////////////////////////
      gt = APP32_TstBit (2, (unsigned long) ModParData);
      if (gt == 0)
	ckBtn_2[i]->SetState (kButtonUp);
      else
	ckBtn_2[i]->SetState (kButtonDown);
      gt = 0;
      /////////////////test ra///////////////////////////
      gt = APP32_TstBit (3, (unsigned long) ModParData);
      if (gt == 0)
	ckBtn_3[i]->SetState (kButtonUp);
      else
	ckBtn_3[i]->SetState (kButtonDown);
      gt = 0;
      //////////////////test ea//////////////////////////
      gt = APP32_TstBit (4, (unsigned long) ModParData);
      if (gt == 0)
	ckBtn_4[i]->SetState (kButtonUp);
      else
	ckBtn_4[i]->SetState (kButtonDown);
      gt = 0;
      //////////////////test ha///////////////////////////
      gt = APP32_TstBit (5, (unsigned long) ModParData);
      if (gt == 0)
	ckBtn_5[i]->SetState (kButtonUp);
      else
	ckBtn_5[i]->SetState (kButtonDown);
      gt = 0;
      /////////////////test gt//////////////////////////////
      gt = APP32_TstBit (6, (unsigned long) ModParData);
      if (gt == 0)
	ckBtn_6[i]->SetState (kButtonUp);
      else
	ckBtn_6[i]->SetState (kButtonDown);
      gt = 0;
      /////////////////test gt//////////////////////////////
      gt = APP32_TstBit (7, (unsigned long) ModParData);
      if (gt == 0)
	ckBtn_7[i]->SetState (kButtonUp);
      else
	ckBtn_7[i]->SetState (kButtonDown);
      gt = 0;
      /////////////////test gt//////////////////////////////
      gt = APP32_TstBit (8, (unsigned long) ModParData);
      if (gt == 0)
	ckBtn_8[i]->SetState (kButtonUp);
      else
	ckBtn_8[i]->SetState (kButtonDown);
      gt = 0;
      /////////////////test gt//////////////////////////////
      gt = APP32_TstBit (9, (unsigned long) ModParData);
      if (gt == 0)
	ckBtn_9[i]->SetState (kButtonUp);
      else
	ckBtn_9[i]->SetState (kButtonDown);
      gt = 0;
      /////////////////test gt//////////////////////////////
      gt = APP32_TstBit (10, (unsigned long) ModParData);
      if (gt == 0)
	ckBtn_10[i]->SetState (kButtonUp);
      else
	ckBtn_10[i]->SetState (kButtonDown);
      gt = 0;
      /////////////////test gt//////////////////////////////
      gt = APP32_TstBit (11, (unsigned long) ModParData);
      if (gt == 0)
	ckBtn_11[i]->SetState (kButtonUp);
      else
	ckBtn_11[i]->SetState (kButtonDown);
      gt = 0;
      /////////////////test gt//////////////////////////////
      gt = APP32_TstBit (12, (unsigned long) ModParData);
      if (gt == 0)
	ckBtn_12[i]->SetState (kButtonUp);
      else
	ckBtn_12[i]->SetState (kButtonDown);
      gt = 0;
      /////////////////test gt//////////////////////////////
      gt = APP32_TstBit (13, (unsigned long) ModParData);
      if (gt == 0)
	ckBtn_13[i]->SetState (kButtonUp);
      else
	ckBtn_13[i]->SetState (kButtonDown);
      gt = 0;
      /////////////////test gt//////////////////////////////
      gt = APP32_TstBit (14, (unsigned long) ModParData);
      if (gt == 0)
	ckBtn_14[i]->SetState (kButtonUp);
      else
	ckBtn_14[i]->SetState (kButtonDown);
      gt = 0;
      /////////////////test gt//////////////////////////////
      gt = APP32_TstBit (15, (unsigned long) ModParData);
      if (gt == 0)
	ckBtn_15[i]->SetState (kButtonUp);
      else
	ckBtn_15[i]->SetState (kButtonDown);
      gt = 0;
      /////////////////test gt//////////////////////////////
      gt = APP32_TstBit (16, (unsigned long) ModParData);
      if (gt == 0)
	ckBtn_16[i]->SetState (kButtonUp);
      else
	ckBtn_16[i]->SetState (kButtonDown);
      gt = 0;
      /////////////////test gt//////////////////////////////
      gt = APP32_TstBit (17, (unsigned long) ModParData);
      if (gt == 0)
	ckBtn_17[i]->SetState (kButtonUp);
      else
	ckBtn_17[i]->SetState (kButtonDown);
      gt = 0;
      /////////////////test gt//////////////////////////////
      gt = APP32_TstBit (18, (unsigned long) ModParData);
      if (gt == 0)
	ckBtn_18[i]->SetState (kButtonUp);
      else
	ckBtn_18[i]->SetState (kButtonDown);
      gt = 0;
      /////////////////test gt//////////////////////////////
      gt = APP32_TstBit (19, (unsigned long) ModParData);
      if (gt == 0)
	ckBtn_19[i]->SetState (kButtonUp);
      else
	ckBtn_19[i]->SetState (kButtonDown);
      gt = 0;
      /////////////////test gt//////////////////////////////
      gt = APP32_TstBit (20, (unsigned long) ModParData);
      if (gt == 0)
	ckBtn_20[i]->SetState (kButtonUp);
      else
	ckBtn_20[i]->SetState (kButtonDown);
      gt = 0;
      /////////////////test gt//////////////////////////////
      gt = APP32_TstBit (21, (unsigned long) ModParData);
      if (gt == 0)
	ckBtn_21[i]->SetState (kButtonUp);
      else
	ckBtn_21[i]->SetState (kButtonDown);
      gt = 0;
      /////////////////test gt//////////////////////////////
      gt = APP32_TstBit (22, (unsigned long) ModParData);
      if (gt == 0)
	ckBtn_22[i]->SetState (kButtonUp);
      else
	ckBtn_22[i]->SetState (kButtonDown);
      gt = 0;
      /////////////////test gt//////////////////////////////
      gt = APP32_TstBit (23, (unsigned long) ModParData);
      if (gt == 0)
	ckBtn_23[i]->SetState (kButtonUp);
      else
	ckBtn_23[i]->SetState (kButtonDown);
      gt = 0;


    }

  //std::cout << "loading info\n";
  return retval;
}

int
TrigConfig::change_values (Long_t mod)
{
  unsigned int ModParData;
  //int retval;
  char pTrigConfig[]="TrigConfig0";

  for (int i = 0; i < numModules; i++)
    {
      Pixie16ReadSglModPar (pTrigConfig, &ModParData, /*module_number1*/i);
      if (ckBtn[i]->IsDown())
	ModParData = APP32_SetBit (0, (unsigned short) ModParData);
      else
	ModParData = APP32_ClrBit (0, (unsigned short) ModParData);

      if (ckBtn_1[i]->IsDown())
	ModParData = APP32_SetBit (1, (unsigned short) ModParData);
      else
	ModParData = APP32_ClrBit (1, (unsigned short) ModParData);

      if (ckBtn_2[i] ->IsDown())
	ModParData = APP32_SetBit (2, (unsigned short) ModParData);
      else
	ModParData = APP32_ClrBit (2, (unsigned short) ModParData);
 
     if (ckBtn_3[i] ->IsDown())
	ModParData = APP32_SetBit (3, (unsigned short) ModParData);
      else
	ModParData = APP32_ClrBit (3, (unsigned short) ModParData);

      if (ckBtn_4[i] ->IsDown())
	ModParData = APP32_SetBit (4, (unsigned short) ModParData);
      else
	ModParData = APP32_ClrBit (4, (unsigned short) ModParData);

      if (ckBtn_5[i] ->IsDown())
	ModParData = APP32_SetBit (5, (unsigned short) ModParData);
      else
	ModParData = APP32_ClrBit (5, (unsigned short) ModParData);

    if (ckBtn_6[i] ->IsDown())
	ModParData = APP32_SetBit (6, (unsigned short) ModParData);
      else
	ModParData = APP32_ClrBit (6, (unsigned short) ModParData);

    if (ckBtn_7[i] ->IsDown())
	ModParData = APP32_SetBit (7, (unsigned short) ModParData);
      else
	ModParData = APP32_ClrBit (7, (unsigned short) ModParData);

    if (ckBtn_8[i] ->IsDown())
	ModParData = APP32_SetBit (8, (unsigned short) ModParData);
      else
	ModParData = APP32_ClrBit (8, (unsigned short) ModParData);

    if (ckBtn_9[i] ->IsDown())
	ModParData = APP32_SetBit (9, (unsigned short) ModParData);
      else
	ModParData = APP32_ClrBit (9, (unsigned short) ModParData);

    if (ckBtn_10[i] ->IsDown())
	ModParData = APP32_SetBit (10, (unsigned short) ModParData);
      else
	ModParData = APP32_ClrBit (10, (unsigned short) ModParData);

    if (ckBtn_11[i] ->IsDown())
	ModParData = APP32_SetBit (11, (unsigned short) ModParData);
      else
	ModParData = APP32_ClrBit (11, (unsigned short) ModParData);

    if (ckBtn_12[i] ->IsDown())
	ModParData = APP32_SetBit (12, (unsigned short) ModParData);
      else
	ModParData = APP32_ClrBit (12, (unsigned short) ModParData);

    if (ckBtn_13[i] ->IsDown())
	ModParData = APP32_SetBit (13, (unsigned short) ModParData);
      else
	ModParData = APP32_ClrBit (13, (unsigned short) ModParData);

    if (ckBtn_14[i] ->IsDown())
	ModParData = APP32_SetBit (14, (unsigned short) ModParData);
      else
	ModParData = APP32_ClrBit (14, (unsigned short) ModParData);

    if (ckBtn_15[i] ->IsDown())
	ModParData = APP32_SetBit (15, (unsigned short) ModParData);
      else
	ModParData = APP32_ClrBit (15, (unsigned short) ModParData);

    if (ckBtn_16[i] ->IsDown())
	ModParData = APP32_SetBit (16, (unsigned short) ModParData);
      else
	ModParData = APP32_ClrBit (16, (unsigned short) ModParData);

    if (ckBtn_17[i] ->IsDown())
	ModParData = APP32_SetBit (17, (unsigned short) ModParData);
      else
	ModParData = APP32_ClrBit (17, (unsigned short) ModParData);

    if (ckBtn_18[i] ->IsDown())
	ModParData = APP32_SetBit (18, (unsigned short) ModParData);
      else
	ModParData = APP32_ClrBit (18, (unsigned short) ModParData);

    if (ckBtn_19[i] ->IsDown())
	ModParData = APP32_SetBit (19, (unsigned short) ModParData);
      else
	ModParData = APP32_ClrBit (19, (unsigned short) ModParData);

    if (ckBtn_20[i] ->IsDown())
	ModParData = APP32_SetBit (20, (unsigned short) ModParData);
      else
	ModParData = APP32_ClrBit (20, (unsigned short) ModParData);

    if (ckBtn_21[i] ->IsDown())
	ModParData = APP32_SetBit (21, (unsigned short) ModParData);
      else
	ModParData = APP32_ClrBit (21, (unsigned short) ModParData);

    if (ckBtn_22[i] ->IsDown())
	ModParData = APP32_SetBit (22, (unsigned short) ModParData);
      else
	ModParData = APP32_ClrBit (22, (unsigned short) ModParData);

    if (ckBtn_23[i] ->IsDown())
	ModParData = APP32_SetBit (23, (unsigned short) ModParData);
      else
	ModParData = APP32_ClrBit (23, (unsigned short) ModParData);


    Pixie16WriteSglModPar (pTrigConfig, ModParData, i/*module_number1*/);
    }
  
  return 1;
}
