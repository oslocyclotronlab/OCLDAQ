// MultCoincDialogue.h
// 
// A class defining the dialogue to control
// the setup of the multiple coincidence settings. 
//
// Author : Jeromy Tompkins
//

#ifndef MULTCOINCDIALOGUE_H
#define MULTCOINCDIALOGUE_H 1

#include <stdint.h>
#include <vector>
#include "TGFrame.h"
#include "TGButtonGroup.h"
#include "TGButton.h"
#include "TGNumberEntry.h"
#include "TGComboBox.h"
#include "TGLabel.h"
#include "TGMsgBox.h"
#include "TGClient.h"
#include "TGIcon.h"
#include "Detector.h"
#include "MultiplicityState.h"
#include "Csra.h"
#include "TimingParamDialogue.h"
#include "MediaLocator.h"
#include "TGLabel.h"

class Csra;

class MultCoincDialogue : public TGTransientFrame
{

    private:
        enum MULTMODE { OFF=0, 
                        EIGHTBYTWO=1, 
                        FIVEBYTHREE=2,
                        FOURBYFOUR=3,
                        TWOBYEIGHT=4,
                        ONEBYSIXTEEN=5,
                        UNKNOWN=6};

        enum WIDGETID { COINCTIMEENTRY=5011,
                        APPLYBUTTON=5021,
                        CANCELBUTTON=5022,
                        LOADBUTTON=5023,
                        MULTENTRY=5025};

    private:
        enum { NMODES=7 };
        MultiplicityState fMultStates[16];    
        
        // Frames
        TGVerticalFrame *fVFrame;
        TGHorizontalFrame *fHFrameTop;
        TGVerticalFrame *fVFrameTopR;
        TGHorizontalFrame *fHFramePars;
        TGVerticalFrame *fVFrameParsL;
        TGVerticalFrame *fVFrameParsR;
        TGHorizontalFrame *fHFrameButtons;

        // Radio buttons for each coinc mode
        TGButtonGroup* fMultModeButtonGrp;
        TGRadioButton* fMultModeRadioButtons[NMODES];
        TGComboBox *fModuleComboBox;

        // Text Inputs
        TGNumberEntryField* fCoincTimeEntryField;
        TGLabel* fCoincTimeLabel;
        TGNumberEntry* fMultiplicityEntry;
        TGLabel* fMultiplicityLabel;

        // Dialogue buttons
        TGTextButton *fLoadButton;
        TGTextButton *fApplyButton;
        TGTextButton *fCancelButton;
        TGNumberEntry *fModuleNumEntry; 
        TGLabel *fModuleNumEntryLabel; 

        Detector* fDetector;
        MULTMODE fDialogueMode;
        MULTMODE fDetectorMode;
        UInt_t fCurrentModule;
        TGMsgBox* fInvalidModWarning;

        Csra* fCsra;
        TGFrame* fCsraAlert;
        TGIcon* fCsraUnknown;
        TGIcon* fCsraON;
        TGIcon* fCsraOFF;
        TGLabel* fCsraText;
        TGLabel* fLabelON;
        TGLabel* fLabelOFF;
        TGLabel* fLabelUnknown;
		TGFrame* fONAlert;
		TGFrame* fOFFAlert;
		TGFrame* fUnknownAlert;
        TGHorizontalFrame* fCsraStateFrame;
        TGGroupFrame* fCsraStateGroupFrame;

        TimingParamDialogue* fTimingDialogue;

        MediaLocator         m_imageLocator;

    public:
        MultCoincDialogue(const TGWindow* parent, const TGWindow* main, Detector* det);
        virtual ~MultCoincDialogue();
        void SetModuleNumber(int moduleNr);
        int GetModuleNumber() const;

        void RegisterCsra(Csra* csra);
        void UpdateCSRAState(int nbits_set);

        void RegisterTimingParamDialogue(TimingParamDialogue* timing_dialogue);

        
    private:
        void ConstructDialogue();
        void setUpAlertIcons();

        void OnLoad();
        void DoLoad();
        void ReadAndCacheMultiplicityStatesFromModule(Int_t module);
        std::vector<MultiplicityState> ReadMultiplicityStatesFromModule(Int_t module);
        void CacheMultiplicityStates(const std::vector<MultiplicityState>& states);
        void PrintCachedMultiplicityStates();
        void UpdateTimeAndThreshold();

        void OnCancel();

        void OnApply();
        std::vector<MultiplicityState> ConstructNewMultiplicityStates();
        std::vector<MultiplicityState> ConstructOffMultStates();
        std::vector<MultiplicityState> ConstructEightGroupsOfTwoMultStates();
        std::vector<MultiplicityState> ConstructFiveGroupsOfThreeMultStates();
        std::vector<MultiplicityState> ConstructFourGroupsOfFourMultStates();
        std::vector<MultiplicityState> ConstructTwoGroupsOfEightMultStates();
        std::vector<MultiplicityState> ConstructOneGroupOfSixteenMultStates();

        void OnModeChanged(MULTMODE new_mode);
        void SetMultiplicityThreshEntryRange();
        void SetDefaultMultiplicityThreshForMode();
        void SetButtonStatesForMode();
        void SetEntryFieldStatesForMode();
        void SetRadioButtonForMode(MULTMODE new_mode);
        void SetRadioButtonStatesForMode();
        void SetBgColorForDetectorMode();
        
        void OnModuleChanged();

        int ReadMultiplicityState(uint16_t module, uint16_t chn, MultiplicityState& state);
    
        int WriteMultiplicityState(uint16_t module, uint16_t chn, const MultiplicityState& state);
        int WriteNewMultiplicityStatesToModule(uint16_t module, const std::vector<MultiplicityState>& states);

        Int_t GetSelectedModule();

        Bool_t ProcessMessage(Long_t msg, Long_t par1, Long_t par2);

        uint32_t SetSelfChannelMaskBits(uint32_t prevbitpattern, uint32_t newsubpattern);
        uint32_t SetSelfMultThresholdBits(uint32_t prevbitpattern, uint32_t newthresh);
         
        MULTMODE ParseMultiplicityMode();        

};

#endif 
