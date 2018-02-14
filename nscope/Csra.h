// Csra.h
//
// A class defining the dialogue to enable bit setting
// in the CSRA of Pixie16 modules.
//
// Author: Sean Liddick
// Modified by: Jeromy Tompkins
//

#ifndef CSRA_H_
#define CSRA_H_ 1
#include <iostream>
#include <iomanip>
#include "TGFrame.h"
#include "TGTextEntry.h"
#include "TGLabel.h"
#include "TGButton.h"
#include "TGNumberEntry.h"
#include "MultCoincDialogue.h"

class MultCoincDialogue;

class Csra:public TGTransientFrame
{
 private:
  MultCoincDialogue* fMultDialogue;

 public:
  Csra (const TGWindow * p, const TGWindow * main,int NumModules=13);
  virtual ~ Csra ();

  void RegisterMultCoincDialogue(MultCoincDialogue* mcd);

 private:
  int numModules;
  TGHorizontalFrame * mn, *buttons;
    TGVerticalFrame *mn_vert;
    TGVerticalFrame *column1, *column2, *column3, *column4, *column5,
      *column6, *column7, *column8, *column9, *column10, *column11,
      *column12, *column13, *column14, *column15, *column16, *column17,
      *column18;
    ///////////////////////////////first column//////////////////////
    TGLabel *Labels[17];
    //////////////////////////////second column/////////////////////
    TGCheckButton *ckBtn[17][17] /*  *ckBtn[17], *ckBtn_1[17], *ckBtn_2[17], *ckBtn_3[17],
      *ckBtn_4[17], *ckBtn_5[17], *ckBtn_6[17], *ckBtn_7[17], *ckBtn_8[17],
      *ckBtn_9[17], *ckBtn_10[17], *ckBtn_11[17], *ckBtn_12[17], *ckBtn_13[17],
      *ckBtn_14[17], *ckBtn_15[17]*/;
    
    int make_columns (TGVerticalFrame * column, Int_t c, TGCheckButton * ckBtn_g[18],
		      /*char */ std::string title, /*char */ std::string tooltip, int id);
    
    ///////////////buttons//////////////////
    TGTextButton *LoadButton, *CancelButton;
    TGButton *ApplyButton, *ExitButton;
    TGNumberEntry *numericMod;
 public:
    int load_info (Long_t, Bool_t verbose=true);
 private:
    int change_values (Long_t);
    bool Load_Once;
    Long_t module_number1;
    Bool_t ProcessMessage (Long_t msg, Long_t parm1, Long_t parm2);
    int CheckAll(Int_t, Bool_t);

    void OnBitsChanged();

 public:
    void SetModuleNumber(int moduleNr) {
      module_number1 = (long)moduleNr;
      numericMod->SetIntNumber(module_number1);
      load_info(module_number1, false);
    };
    
    static int GetNBitsOn(unsigned int module, unsigned int bit);
};

#endif /*CSRA_H_ */
