#include "ExpertMod.h"


ExpertMod::ExpertMod (const TGWindow * p, const TGWindow * main, /*char **/string name,int NumModules=13)
{
  SetCleanup (kDeepCleanup);

  numModules=NumModules;

  mn_vert = new TGVerticalFrame (this, 200, 300);
  TGHorizontalFrame *Median = new TGHorizontalFrame (mn_vert, 400, 300);

  AddFrame (mn_vert,
	    new TGLayoutHints (kLHintsTop | kLHintsLeft, 2, 2, 2, 2));
  Median->AddFrame (lstBox =
		    new TGComboBox (mn_vert, 2755),
		    new TGLayoutHints (kLHintsCenterX, 0, 0, 0, 0));
  lstBox->Associate (this);
  lstBox->AddEntry ("SET MASTER", 0);
  lstBox->AddEntry ("MAX EVENTS/BUFFER", 1);
  lstBox->AddEntry ("CoinWindLen", 2);
  lstBox->AddEntry ("WinDelayLen", 3);
  lstBox->AddEntry ("NumMWModules", 4);
  lstBox->AddEntry ("MWModAddress", 5);
  lstBox->AddEntry ("CRATE ID", 6);
  lstBox->Select (0);
  lstBox->Resize (150, 20);
  NumEntry = new TGNumberEntryField (Median, 1000, 0,
				     TGNumberFormat::kNESReal);
  NumEntry->Resize (90, 20);

  Median->AddFrame (NumEntry, new TGLayoutHints (kLHintsCenterX, 0, 0, 0, 0));

  Buttons = new TGHorizontalFrame (mn_vert, 400, 300);
/////////////////////////////module entry///////////////////////////////

  TGHorizontal3DLine *ln1 = new TGHorizontal3DLine (mn_vert, 150, 2);
  TGLabel *mod = new TGLabel (Buttons, "Module #");

  numericMod = new TGNumberEntry (Buttons, 0, 4, MODNUMBER, (TGNumberFormat::EStyle) 0, (TGNumberFormat::EAttribute) 1, (TGNumberFormat::ELimit) 3,	// kNELLimitMinMax
				  0, 3);
  numericMod->SetButtonToNum (0);
  numericMod->IsEditable ();
  mn_vert->AddFrame (Median,
		     new TGLayoutHints (kLHintsTop | kLHintsLeft, 0, 0, 0,
					0));
  mn_vert->AddFrame (ln1,
		     new TGLayoutHints (kLHintsCenterX | kLHintsCenterY, 0, 0,
					10, 10));
  Buttons->AddFrame (mod, new TGLayoutHints (kLHintsCenterX, 5, 10, 3, 0));
  Buttons->AddFrame (numericMod,
		     new TGLayoutHints (kLHintsTop | kLHintsLeft, 0, 20, 0,
					0));

  numericMod->Associate (this);
  mn_vert->AddFrame (Buttons,
		     new TGLayoutHints (kLHintsTop | kLHintsLeft, 0, 0, 0,
					0));

  ////////////////////////////Buttons/////////////////////////////////////
  TGTextButton *LoadButton = new TGTextButton (Buttons, "&Load", LOAD);
  LoadButton->Associate (this);
  TGTextButton *ApplyButton = new TGTextButton (Buttons, "&Apply", APPLY);
  ApplyButton->Associate (this);
  TGTextButton *CancelButton = new TGTextButton (Buttons, "&Cancel", CANCEL);
  CancelButton->Associate (this);
  Buttons->AddFrame (LoadButton,
		     new TGLayoutHints (kLHintsCenterX, 0, 0, 0, 0));
  Buttons->AddFrame (ApplyButton,
		     new TGLayoutHints (kLHintsCenterX, 0, 0, 0, 0));
  Buttons->AddFrame (CancelButton,
		     new TGLayoutHints (kLHintsCenterX, 0, 0, 0, 0));
		     
modNumber=0;
  MapSubwindows ();
  Resize ();			// resize to default size
  CenterOnParent ();

  SetWindowName (name.c_str());

  MapWindow ();
}

ExpertMod::~ExpertMod ()
{
}

Bool_t
ExpertMod::ProcessMessage (Long_t msg, Long_t parm1, Long_t parm2)
{
  switch (GET_MSG (msg))
    {
      case kC_COMMAND:
	switch (GET_SUBMSG (msg))
	  {
	    case kCM_BUTTON:
	      switch (parm1)
		{
		  case (MODNUMBER):
		    if (parm2 == 0)
		      {
			if (modNumber != numModules-1)
			  {
			    ++modNumber;
			    numericMod->SetIntNumber (modNumber);
			  }
		      }
		    else
		      {
			if (modNumber != 0)
			  {
			    if (--modNumber == 0)
			      modNumber = 0;
			    numericMod->SetIntNumber (modNumber);
			  }
		      }
		    break;
		  case LOAD:
		    {
		      Load_Once = true;
		      
		      load_info (modNumber,lstBox->GetSelected());
		    }
		    break;
		  case APPLY:
		    if (Load_Once)
		      change_values (modNumber,lstBox->GetSelected());
		    else
		      std::cout << "please load once first !\n";
		    break;
		  case CANCEL:	/// Cancel Button
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
ExpertMod::load_info (Long_t mod, int param)
{
  int retval=0;
  unsigned int ChanParData = 0;
  char text[20];
  char pMODULE_CSRB[]="MODULE_CSRB";
  char pMAX_EVENTS[]="MAX_EVENTS";
  char pCOINWINDLEN[]="COINWINDLEN";
  char pWINDELAYLEN[]="WINDELAYLEN";
  char pNUMMWMODULES[]="NUMMWMODULES";
  char pMWMODADDR[]="MWMODADDR";
  char pCrateID[]="CrateID";

  if (param == 0 )
    {
      retval = Pixie16ReadSglModPar (/*"MODULE_CSRB"*/pMODULE_CSRB, &ChanParData, modNumber);
      sprintf (text, "%u", ChanParData);
      NumEntry->SetText (text);
    }
  
  
  if (param == 1 )
    {
      retval = Pixie16ReadSglModPar (/*"MAX_EVENTS"*/pMAX_EVENTS, &ChanParData, modNumber);
      sprintf (text, "%u", ChanParData);
      NumEntry->SetText (text);    
    }
  
  
  if (param == 2 )
    {
      retval = Pixie16ReadSglModPar (/*"COINWINDLEN"*/pCOINWINDLEN, &ChanParData, mod);
      sprintf (text, "%u", ChanParData);
      NumEntry->SetText (text);    
    }
  
  
  if (param == 3 )
    {
      retval = Pixie16ReadSglModPar (/*"WINDELAYLEN"*/pWINDELAYLEN, &ChanParData, mod);
      sprintf (text, "%u", ChanParData);
      NumEntry->SetText (text);
    }
  

  if (param == 4 )
    {
      retval = Pixie16ReadSglModPar (/*"NUMMWMODULES"*/pNUMMWMODULES, &ChanParData, mod);
      sprintf (text, "%u", ChanParData);
      NumEntry->SetText (text);
    }
    

  if (param == 5 )
    {
      retval = Pixie16ReadSglModPar (/*"MWMODADDR"*/pMWMODADDR, &ChanParData, mod);
      sprintf (text, "%u", ChanParData);
      NumEntry->SetText (text);
    }

  if (param == 6 )
    {
      retval = Pixie16ReadSglModPar (/*"CrateID"*/pCrateID, &ChanParData, mod);
      sprintf (text, "%u", ChanParData);
      NumEntry->SetText (text);
    }
    

  return retval;
}

int
ExpertMod::change_values (Long_t mod, int param)
{
  double value;
  int retval=0;
  value = NumEntry->GetNumber ();
  char pMODULE_CSRB[]="MODULE_CSRB";
  char pMAX_EVENTS[]="MAX_EVENTS";
  char pCOINWINDLEN[]="COINWINDLEN";
  char pWINDELAYLEN[]="WINDELAYLEN";
  char pNUMMWMODULES[]="NUMMWMODULES";
  char pMWMODADDR[]="MWMODADDR";
  char pCrateID[]="CrateID";


  if (param == 0 )
    {
      retval=Pixie16WriteSglModPar (/*"MODULE_CSRB"*/pMODULE_CSRB,(long unsigned int)value,modNumber );    
    }
  
  if (param == 1 )
    {
      retval=Pixie16WriteSglModPar (/*"MAX_EVENTS"*/pMAX_EVENTS,(long unsigned int)value,modNumber );    
    }

  if(param==2 )
    {
      retval=Pixie16WriteSglModPar (/*"COINWINDLEN"*/pCOINWINDLEN,(long unsigned int)value,modNumber );    
    }

  if(param==3)
    {
      retval=Pixie16WriteSglModPar (/*"WINDELAYLEN"*/pWINDELAYLEN,(long unsigned int)value,modNumber );    
    }
  
  if(param==4)
    {
      retval=Pixie16WriteSglModPar (/*"NUMMWMODULES"*/pNUMMWMODULES,(long unsigned int)value,modNumber );    
    }

  if(param==5)
    {
      retval=Pixie16WriteSglModPar (/*"MWMODADDR"*/pMWMODADDR,(long unsigned int)value,modNumber );    
    }
  if(param==6)
    {
      retval=Pixie16WriteSglModPar (/*"CrateID"*/pCrateID,(long unsigned int)value,modNumber );    
    }

  return retval;
  
}
