// TimingParamDialogue.h
//
// A class defining the dialogue to control the timing
// parameters in the Pixie16 modules.
//
// Author : Jeromy Tompkins
//

#ifndef TIMINGPARAMDIALOGUE_H_
#define TIMINGPARAMDIALOGUE_H_

#include <iostream>
#include <iomanip>
#include "TImage.h"
#include "TGIcon.h"
#include "TGFrame.h"
#include "TGButton.h"
#include "TGNumberEntry.h"
#include "TGTextEntry.h"
#include "TGLabel.h"
#include "TG3DLine.h"
#include "MediaLocator.h"

#include "pixie16app_export.h"

class TimingParamDialogue: public TGTransientFrame
{
    private:
        enum Commands
        {
            LOAD,
            APPLY,
            CANCEL,
            MODNUMBER,
            FILTER,
            COPYBUTTON
        };
		 
    private: 
        TGHorizontalFrame* fHFrame;

        // Image data
        TImage* fPicData;
        TGIcon* fPic;

        // Frames and such for the Table
        TGVerticalFrame* mn_vert;
        TGHorizontalFrame* mn;
        int Rows;
        TGVerticalFrame **Column;
        TGTextEntry *cl0;//label for the title of the column[0] 
        TGTextEntry **CLabel;//labels for the numeric columns
        TGNumberEntryField ***NumEntry;//numeric entries [column][row], 
        //column[0] has the labels
        TGTextEntry **Labels; //labels in the left most column

        int numModules;
        TGNumberEntry *numericMod;
        TGHorizontalFrame* Buttons ;

        MediaLocator   m_imageLocator;

    public:
        TimingParamDialogue(const TGWindow * p, const TGWindow * main, int NumModules=13);
        virtual ~TimingParamDialogue();
        Bool_t ProcessMessage (Long_t msg, Long_t parm1, Long_t parm2);
        int change_values (Long_t mod);
        int load_info (Long_t mod);
        void SetModuleNumber(int moduleNr) {
            modNumber = (short)moduleNr;
            numericMod->SetIntNumber(modNumber);
            load_info(modNumber);
        };

        short int chanNumber;
        short int modNumber; 
        TGNumberEntry* chanCopy;
        bool Load_Once;
        char tmp[10];
        float tdelay,twidth;

        void UpdateChanCoincWidthColumn();

    protected:
        void MakeTable(TGFrame* parent, int columns, int rows, int NumModules);
        void DoCopy();

};

#endif /*TIMINGPARAMDIALOGUE_H_*/
