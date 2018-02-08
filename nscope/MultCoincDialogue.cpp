// MultCoincDialogue.cpp
//
// Author : Jeromy Tompkins

#include "MultCoincDialogue.h"

#include "TGDimension.h"

MultCoincDialogue::MultCoincDialogue(const TGWindow* parent, const TGWindow* main, Detector *det)
    : TGTransientFrame(parent,main,600,600,kVerticalFrame), fDialogueMode(OFF), fDetectorMode(OFF),
    fCsra(0), fCsraUnknown(0), fCsraON(0), fCsraOFF(0), fTimingDialogue(0)
{
    DontCallClose();
    SetMWMHints(kMWMDecorAll,
                kMWMFuncResize|kMWMFuncMaximize|kMWMFuncMinimize|kMWMFuncMove, 0);

    fDetector = det;
    if (fDetector==0) {
        throw "Null pointer passed for detector";
    }

    // Set up the image search path for in-tree builds and deployed
    m_imageLocator.addPath("./resources");
#ifdef PREFIX
    // to properly find the images in both the build tree and the installation path,
    // we expect PREFIX to be defined.
    m_imageLocator.addPath(std::string(PREFIX) + "/share/nscope" );
#endif

    ConstructDialogue();
}

MultCoincDialogue::~MultCoincDialogue()
{
    Cleanup();
}


void MultCoincDialogue::SetModuleNumber(int moduleNr)
{
    if (moduleNr>=0 && moduleNr<fDetector->GetNumberModules()) {
        fModuleNumEntry->SetNumber(moduleNr);
        OnModuleChanged();
    }

}

int MultCoincDialogue::GetModuleNumber() const
{
    return fModuleNumEntry->GetIntNumber();
}

void MultCoincDialogue::RegisterCsra(Csra* csra)
{
   fCsra = csra;
}

void MultCoincDialogue::RegisterTimingParamDialogue(TimingParamDialogue* timing_dialogue)
{
    fTimingDialogue = timing_dialogue;
}

void MultCoincDialogue::UpdateCSRAState(int nbits_on)
{
 
  std::cout << "nbits on = " << nbits_on << std::endl;
  // this is a kludge
  fCsraStateFrame->HideFrame(fONAlert);
  fCsraStateFrame->HideFrame(fOFFAlert);
  fCsraStateFrame->HideFrame(fUnknownAlert);
  fCsraStateFrame->HideFrame(fCsraAlert);

  if (nbits_on==16) { // all channels set to ON
    fCsraAlert = fONAlert;
    fCsraText->SetText("ALL channels are ");

  } else if (nbits_on==0) { // all channels set to OFF
    fCsraAlert = fOFFAlert;
    fCsraText->SetText("ALL channels are");

  } else { // subset of channels are implementing ch validation
    fCsraAlert = fUnknownAlert;
    fCsraText->SetText("INCONSISTENT");
  }

  fCsraStateFrame->ShowFrame(fCsraAlert);
  Resize();
}


void MultCoincDialogue::ConstructDialogue()
{
    SetCleanup(kLocalCleanup);

    Pixel_t black;
    fClient->GetColorByName("black",black);

    fVFrame = new TGVerticalFrame(this);
    fVFrame->SetCleanup(kLocalCleanup);
    AddFrame(fVFrame,new TGLayoutHints(kLHintsExpandX | kLHintsExpandY,5,5,5,5));
    
    fHFrameTop = new TGHorizontalFrame(fVFrame,300,500);
    fHFrameTop->SetCleanup(kLocalCleanup);
    fVFrame->AddFrame(fHFrameTop,new TGLayoutHints(kLHintsExpandX | kLHintsTop,2,2,2,2));
    
    fMultModeButtonGrp = new TGButtonGroup(fHFrameTop, "Channel Grouping", kVerticalFrame); 
    fMultModeButtonGrp->SetCleanup(kLocalCleanup);
    fHFrameTop->AddFrame(fMultModeButtonGrp,new TGLayoutHints(kLHintsLeft | kLHintsExpandY,5,5,5,5));
    fMultModeRadioButtons[6] = new TGRadioButton(fMultModeButtonGrp,"Unknown ",UNKNOWN); 
    fMultModeRadioButtons[6]->SetToolTipText("Nonstandard state detected.");
    fMultModeRadioButtons[6]->SetTextColor(black,false);
    fMultModeRadioButtons[6]->SetEnabled(kFALSE);
    fMultModeRadioButtons[6]->Associate(this);
    fMultModeRadioButtons[0] = new TGRadioButton(fMultModeButtonGrp,"Off ",OFF); 
    fMultModeRadioButtons[0]->SetTextColor(black,false);
    fMultModeRadioButtons[0]->Associate(this);
    fMultModeRadioButtons[1] = new TGRadioButton(fMultModeButtonGrp," 8 x 2 ",EIGHTBYTWO); 
    fMultModeRadioButtons[1]->SetTextColor(black,false);
    fMultModeRadioButtons[1]->SetToolTipText("Eight groups of two channels");
    fMultModeRadioButtons[1]->Associate(this);
    fMultModeRadioButtons[2] = new TGRadioButton(fMultModeButtonGrp," 5 x 3 ",FIVEBYTHREE); 
    fMultModeRadioButtons[2]->SetTextColor(black,false);
    fMultModeRadioButtons[2]->SetToolTipText("Five groups of three channels (ch# 0 excluded)");
    fMultModeRadioButtons[2]->Associate(this);
    fMultModeRadioButtons[3] = new TGRadioButton(fMultModeButtonGrp," 4 x 4 ",FOURBYFOUR); 
    fMultModeRadioButtons[3]->SetTextColor(black,false);
    fMultModeRadioButtons[3]->SetToolTipText("Four groups of four channels");
    fMultModeRadioButtons[3]->Associate(this);
    fMultModeRadioButtons[4] = new TGRadioButton(fMultModeButtonGrp," 2 x 8 ",TWOBYEIGHT); 
    fMultModeRadioButtons[4]->SetTextColor(black,false);
    fMultModeRadioButtons[4]->SetToolTipText("Two groups of 8 channels");
    fMultModeRadioButtons[4]->Associate(this);
    fMultModeRadioButtons[5] = new TGRadioButton(fMultModeButtonGrp," 1 x 16",ONEBYSIXTEEN); 
    fMultModeRadioButtons[5]->SetTextColor(black,false);
    fMultModeRadioButtons[5]->SetToolTipText("All channels in one group");
    fMultModeRadioButtons[5]->Associate(this);
    fMultModeButtonGrp->SetButton(0,kTRUE);
    fDialogueMode = OFF;
    fMultModeButtonGrp->SetExclusive(kTRUE);
    fMultModeButtonGrp->Show();
    
    fCsraStateGroupFrame = new TGGroupFrame(fHFrameTop,"CSRA Ch.Trig Validation Bits",kVerticalFrame);
    fCsraStateGroupFrame->SetCleanup(kLocalCleanup);
    fHFrameTop->AddFrame(fCsraStateGroupFrame,new TGLayoutHints(kLHintsRight|kLHintsTop,5,5,5,5));
    fCsraStateFrame = new TGHorizontalFrame(fCsraStateGroupFrame);
    fCsraStateFrame->SetCleanup(kLocalCleanup);
    fCsraStateGroupFrame->AddFrame(fCsraStateFrame,new TGLayoutHints(kLHintsRight|kLHintsTop,0,0,0,0));

    // CSRA alert icon set up
    setUpAlertIcons();

    fHFramePars = new TGHorizontalFrame(fVFrame,200,200);
    fHFramePars->SetCleanup(kLocalCleanup);
    fVFrame->AddFrame(fHFramePars,new TGLayoutHints(kLHintsExpandX,2,2,2,2));
   
    fVFrameParsL = new TGVerticalFrame(fHFramePars);
    fVFrameParsL->SetCleanup(kLocalCleanup);
    fHFramePars->AddFrame(fVFrameParsL, new TGLayoutHints(kLHintsLeft| kLHintsCenterY,2,2,2,2));

    fCoincTimeLabel = new TGLabel(fVFrameParsL,"Ch. Coincidence Width (us)");
    fCoincTimeLabel->SetTextColor(black,false);
    fVFrameParsL->AddFrame(fCoincTimeLabel, new TGLayoutHints(kLHintsCenterX | kLHintsTop,1,1,1,1));
    fCoincTimeEntryField = new TGNumberEntryField(fVFrameParsL,COINCTIMEENTRY,10,
            TGNumberFormat::kNESReal,TGNumberFormat::kNEANonNegative);  
    fCoincTimeEntryField->SetState(kFALSE);
    fVFrameParsL->AddFrame(fCoincTimeEntryField, new TGLayoutHints(kLHintsCenterX | kLHintsBottom,0,0,0,0));

    fVFrameParsR = new TGVerticalFrame(fHFramePars);
    fVFrameParsR->SetCleanup(kLocalCleanup);
    fHFramePars->AddFrame(fVFrameParsR, new TGLayoutHints(kLHintsRight| kLHintsCenterY,2,2,2,2));
    fMultiplicityLabel = new TGLabel(fVFrameParsR,"Min. Multiplicity To Trigger");
    fMultiplicityLabel->SetTextColor(black,false);
    fVFrameParsR->AddFrame(fMultiplicityLabel, new TGLayoutHints(kLHintsCenterX | kLHintsTop,1,1,1,1));
    fMultiplicityEntry = new TGNumberEntry(fVFrameParsR,0,10,MULTENTRY,
            TGNumberFormat::kNESInteger,TGNumberFormat::kNEANonNegative);
    fMultiplicityEntry->SetState(kFALSE);
    fVFrameParsR->AddFrame(fMultiplicityEntry, new TGLayoutHints(kLHintsCenterX | kLHintsBottom,1,1,1,1));

    fHFrameButtons = new TGHorizontalFrame(fVFrame);
    fHFrameButtons->SetCleanup(kLocalCleanup);
    fVFrame->AddFrame(fHFrameButtons,new TGLayoutHints(kLHintsExpandX));

    fCancelButton = new TGTextButton(fHFrameButtons,"&Cancel",CANCELBUTTON);
    fCancelButton->SetTextColor(black,false);
    fCancelButton->SetEnabled(kTRUE);
    fCancelButton->SetMargins(15,15,5,5);
    fHFrameButtons->AddFrame(fCancelButton, new TGLayoutHints(kLHintsRight,0,5,5,5));
    fCancelButton->Associate(this);

    fApplyButton = new TGTextButton(fHFrameButtons,"&Apply",APPLYBUTTON);
    fApplyButton->SetTextColor(black,false);
    fApplyButton->SetEnabled(kTRUE);
    fApplyButton->SetMargins(15,15,5,5);
    fHFrameButtons->AddFrame(fApplyButton, new TGLayoutHints(kLHintsRight,0,0,5,5));
    fApplyButton->Associate(this);

    fLoadButton = new TGTextButton(fHFrameButtons,"&Load",LOADBUTTON);
    fLoadButton->SetTextColor(black,false);
    fLoadButton->SetEnabled(kTRUE);
    fLoadButton->SetMargins(15,15,5,5);
    fHFrameButtons->AddFrame(fLoadButton, new TGLayoutHints(kLHintsRight,5,0,5,5));
    fLoadButton->Associate(this);
   
    fModuleNumEntryLabel = new TGLabel(fHFrameButtons,"Module #");
    fModuleNumEntryLabel->SetTextColor(black,false);
    fHFrameButtons->AddFrame(fModuleNumEntryLabel,new TGLayoutHints(kLHintsLeft,5,2,5,5));
    fModuleNumEntry = new TGNumberEntry(fHFrameButtons,0,5,5024,
            TGNumberEntry::kNESInteger, TGNumberEntry::kNEANonNegative, TGNumberEntry::kNELLimitMinMax);
    if (fDetector->GetNumberModules()>0) {
        fModuleNumEntry->SetLimitValues(0,fDetector->GetNumberModules()-1);
    } else {
        fModuleNumEntry->SetLimitValues(0,1);
    }
    fModuleNumEntry->Associate(this);
    fHFrameButtons->AddFrame(fModuleNumEntry,new TGLayoutHints(kLHintsLeft,2,5,5,5));

    MapSubwindows();
    Resize();
    CenterOnParent();
    
    SetWindowName("Configure Multiplicity Coincidence");
    MapWindow();
    Resize();

}

void MultCoincDialogue::setUpAlertIcons() {

    std::string imagePath = m_imageLocator.locateFile("CSRAON.png");
    if (imagePath.empty()) {
      fLabelON = new TGLabel(fCsraStateFrame, "ON");
      fLabelON->Resize(48,48);
      fCsraStateFrame->AddFrame(fLabelON,new TGLayoutHints(kLHintsRight|kLHintsCenterY,5,0,0,0));
	  fONAlert = fLabelON;
    } else {
      fCsraON = new TGIcon(fCsraStateFrame, imagePath.c_str());
      fCsraON->Resize(48,48);
      fCsraStateFrame->AddFrame(fCsraON,new TGLayoutHints(kLHintsRight|kLHintsCenterY,5,0,0,0));
	  fONAlert = fCsraON;
    }
	fCsraStateFrame->HideFrame(fONAlert);

    imagePath = m_imageLocator.locateFile("CSRAOFF.png");
    if (imagePath.empty()) {
      fLabelOFF = new TGLabel(fCsraStateFrame, "OFF");
      fLabelOFF->Resize(48,48);
      fCsraStateFrame->AddFrame(fLabelOFF,new TGLayoutHints(kLHintsRight|kLHintsCenterY,5,0,0,0));
	  fOFFAlert = fLabelOFF;
    } else {
      fCsraOFF = new TGIcon(fCsraStateFrame, imagePath.c_str());
      fCsraOFF->Resize(48,48);
      fCsraStateFrame->AddFrame(fCsraOFF,new TGLayoutHints(kLHintsRight|kLHintsCenterY,5,0,0,0));
	  fOFFAlert = fCsraOFF;
    }
	fCsraStateFrame->HideFrame(fOFFAlert);

    imagePath = m_imageLocator.locateFile("CSRAunknown.png");
    if (imagePath.empty()) {
      fLabelUnknown = new TGLabel(fCsraStateFrame, "UNKNOWN");
      fLabelUnknown->Resize(48,48);
      fCsraStateFrame->AddFrame(fLabelUnknown,new TGLayoutHints(kLHintsRight|kLHintsCenterY,5,0,0,0));
	  fUnknownAlert = fLabelUnknown;
    } else {
      fCsraUnknown = new TGIcon(fCsraStateFrame,imagePath.c_str());
      fCsraUnknown->Resize(48,48);
      fCsraStateFrame->AddFrame(fCsraUnknown,new TGLayoutHints(kLHintsRight|kLHintsCenterY,5,0,0,0));
      fUnknownAlert = fCsraUnknown;
    }
	fCsraAlert = fUnknownAlert;

    fCsraText = new TGLabel(fCsraStateFrame,"  INCONSISTENT  ");
    fCsraStateFrame->AddFrame(fCsraText,new TGLayoutHints(kLHintsLeft|kLHintsCenterY,0,0,0,0));
	fCsraStateFrame->Resize();
}

void MultCoincDialogue::OnLoad() 
{
      DoLoad();
      PrintCachedMultiplicityStates();
}

void MultCoincDialogue::DoLoad()
{
      ReadAndCacheMultiplicityStatesFromModule(fCurrentModule);
      MULTMODE detected_mode = ParseMultiplicityMode();
      SetRadioButtonForMode(detected_mode);
      OnModeChanged(detected_mode);
      UpdateTimeAndThreshold();

      // Store current status of dialogue
      fDetectorMode = fDialogueMode;
      SetBgColorForDetectorMode();

      // Update the state of the module being changed to
      UpdateCSRAState(Csra::GetNBitsOn(fCurrentModule,13));
}

void MultCoincDialogue::ReadAndCacheMultiplicityStatesFromModule(Int_t module)
{
    std::vector<MultiplicityState> new_states = ReadMultiplicityStatesFromModule(module);
    CacheMultiplicityStates(new_states);
}

std::vector<MultiplicityState> MultCoincDialogue::ReadMultiplicityStatesFromModule(Int_t module)
{
    std::cout << "**** Reading multiplicity states from module " << module << std::endl;
    std::vector<MultiplicityState> new_states(16); 

    for (uint16_t ch=0; ch<16; ++ch) {
        int result = ReadMultiplicityState(module,ch,new_states[ch]);
        if (result<0)
            std::cout << "Failed to read channel#" << ch << " from module#" << module << std::endl;
    }

    return new_states;
}

void MultCoincDialogue::CacheMultiplicityStates(const std::vector<MultiplicityState>& states)
{
    std::cout << "**** Caching new multiplicity states " << std::endl;
    for (uint16_t ch=0; ch<16; ++ch) {
        fMultStates[ch] = states.at(ch);    
    }
    
}

void MultCoincDialogue::PrintCachedMultiplicityStates()
{
    std::cout << "**** Printing cached multiplicity states from previous read" << std::endl;
    for (uint16_t ch=0; ch<16; ++ch) {
        std::cout << Form("---------------- Channel %2d ----------------",ch) << std::endl;
        fMultStates[ch].Print(); 
    }

}

int MultCoincDialogue::ReadMultiplicityState(uint16_t module, uint16_t chn, MultiplicityState& state)
{

    double multiplicityMaskLow;
    double multiplicityMaskHigh;
    double coincTime;

    uint32_t bitMaskLow = -999;
    uint32_t bitMaskHigh = -999;

    int numModules= fDetector->GetNumberModules();

    int resultLow; 
    int resultHigh; 
    int resultCoincTime; 

    char pMultiplicityMaskL[]="MultiplicityMaskL";
    char pMultiplicityMaskH[]="MultiplicityMaskH";
    char pChanTrigStretch[]="ChanTrigStretch";

    if(module < numModules && chn<16) {

      resultLow = Pixie16ReadSglChanPar(/*"MultiplicityMaskL"*/pMultiplicityMaskL,&multiplicityMaskLow,module,chn);
        if (resultLow==0) {
            bitMaskLow = static_cast<uint32_t>(multiplicityMaskLow);
        } else {
            std::cerr << "Pixie16ReadSglChanPar(\"MultiplicityMaskL\",...) returned error code " << resultLow;
            if (resultLow==-1) {
                std::cerr << " (Invalid module number)" << std::endl;
            } else if (resultLow==-2) {
                std::cerr << " (Invalid channel number)" << std::endl;
            } else if (resultLow==-3) {
                std::cerr << " (Invalid parameter name)" << std::endl;
            } else if (resultLow<0) {
                std::cerr << " (Unknown error code)" << std::endl;
            }
            state = MultiplicityState(bitMaskLow,0x0,0.0);
            return resultLow; 
        }

        resultHigh = Pixie16ReadSglChanPar(/*"MultiplicityMaskH"*/pMultiplicityMaskH,&multiplicityMaskHigh,module,chn);
        if (resultHigh==0) {
            bitMaskHigh = static_cast<uint32_t>(multiplicityMaskHigh);
        } else {
            std::cerr << "Pixie16ReadSglChanPar(\"MultiplicityMaskH\",...) returned error code " << resultLow;
            if (resultHigh==-1) {
                std::cerr << " (Invalid module number)" << std::endl;
            } else if (resultHigh==-2) {
                std::cerr << " (Invalid channel number)" << std::endl;
            } else if (resultHigh==-3) {
                std::cerr << " (Invalid parameter name)" << std::endl;
            } else if (resultHigh<0) {
                std::cerr << " (Unknown error code)" << std::endl;
            }
            state = MultiplicityState(bitMaskLow,bitMaskHigh,0.0);
            return resultHigh;
        }
        
        resultCoincTime = Pixie16ReadSglChanPar(/*"ChanTrigStretch"*/pChanTrigStretch,&coincTime,module,chn);
        if (resultCoincTime<0) {
            std::cerr << "Pixie16ReadSglChanPar(\"ChanTrigStretch\",...) returned error code " << resultLow;
            if (resultCoincTime==-1) {
                std::cerr << " (Invalid module number)" << std::endl;
            } else if (resultCoincTime==-2) {
                std::cerr << "(Invalid channel number)" << std::endl;
            } else if (resultCoincTime==-3) {
                std::cerr << " (Invalid parameter name)" << std::endl;
            } else if (resultCoincTime<0) {
                std::cerr << " (Unknown error code)" << std::endl;
            }
            state = MultiplicityState(bitMaskLow,bitMaskHigh,coincTime);
            return resultCoincTime; 
        }

        state = MultiplicityState(bitMaskLow,bitMaskHigh,coincTime);
        
    } else {
        if (module >= numModules) {
            std::cerr << "Invalid module number" << std::endl;
            return -1;
        } else {
            std::cerr << "Invalid channel number" << std::endl;
            return -2;
        }
    }  

    return 0; 
}


void MultCoincDialogue::OnCancel()
{
//    DeleteWindow();
    UnmapWindow();
    
}

void MultCoincDialogue::OnApply() 
{
    std::vector<MultiplicityState> new_states = ConstructNewMultiplicityStates();
    int result = WriteNewMultiplicityStatesToModule(fCurrentModule, new_states);

    DoLoad();
    if (result==0)
        PrintCachedMultiplicityStates();

    if (fTimingDialogue!=0)
        fTimingDialogue->UpdateChanCoincWidthColumn();
}

int MultCoincDialogue::WriteNewMultiplicityStatesToModule(uint16_t module, const std::vector<MultiplicityState>& states)
{
    std::cout << "**** Attempting to write new multiplicity states to module " << fCurrentModule << std::endl;
    int result = -1;

    for (uint16_t ch=0; ch<states.size() && ch<16; ++ch) {
        std::cout << "---- Channel " << ch << " ... ";
#ifdef MCD_NOWRITE
        int result = 0;
        states.at(ch).Print();
#else
        result = WriteMultiplicityState(module, ch, states.at(ch));
#endif
        if (result!=0) {
            std::cerr << "Error code " << result << " returned for channel " << ch << std::endl;
            std::cout << "     Failed write aborts writing to subsequent channels" << std::endl;
            break;            
        } else {
            std::cout << "success" << std::endl;
        }
    }

    return result;
}


std::vector<MultiplicityState> MultCoincDialogue::ConstructNewMultiplicityStates()
{
    std::vector<MultiplicityState> new_states;    

    // Handle mode appropriately 
    MULTMODE mode = fDialogueMode;

    switch(mode) {
        case OFF :
            new_states = ConstructOffMultStates(); 
            break;
        case EIGHTBYTWO:
            new_states = ConstructEightGroupsOfTwoMultStates(); 
            break;
        case FIVEBYTHREE:
            new_states = ConstructFiveGroupsOfThreeMultStates(); 
            break;
        case FOURBYFOUR:
            new_states = ConstructFourGroupsOfFourMultStates(); 
            break;
        case TWOBYEIGHT:
            new_states = ConstructTwoGroupsOfEightMultStates(); 
            break;
        case ONEBYSIXTEEN:
            new_states = ConstructOneGroupOfSixteenMultStates(); 
            break;
        default:
            std::cout << "Not implemented" << std::endl;
            break;
    }

    return new_states;
}

std::vector<MultiplicityState> MultCoincDialogue::ConstructOffMultStates()
{
    std::cout << "**** Constructing multiplicity states for OFF mode " << std::endl;

    std::vector<MultiplicityState> new_states;

    for (UInt_t ch=0; ch<16; ++ch) {
        // Self and right channel multiplicity masks set to 0
        uint32_t lowmask = 0x0; 

        uint32_t highmask = fMultStates[ch].GetMultiplicityMaskHigh();
        // Self multiplicity threshold set to 0
        highmask = SetSelfMultThresholdBits(highmask,0);
        // Clear bits [15:0] and [30:25] to remove any preexisting left
        // multiplicity state and also the right and left mult thresholds
        // while leaving the reserved bits untouched 
        highmask = (0x81FF0000 & highmask);    

        double coinctime = fMultStates[ch].GetCoincidenceTime();
        
        MultiplicityState state(lowmask,highmask,coinctime);
        
//        state.Print(); 
        new_states.push_back(state);
    }

    return new_states;
}

std::vector<MultiplicityState> MultCoincDialogue::ConstructEightGroupsOfTwoMultStates()
{
    std::cout << "**** Constructing multiplicity states for 8 x 2 mode " << std::endl;
    std::vector<MultiplicityState> new_states;

    uint32_t multmask = 0x3;

    uint32_t multthresh = fMultiplicityEntry->GetIntNumber();

    double coinctime = fCoincTimeEntryField->GetNumber();
    if (coinctime<0) {
        std::cerr << "Invalid coincidence time ... new multiplicity states cannot be constructed" << std::endl;
        return new_states;
    }

    uint32_t bit_offset = 0;
    for (UInt_t ch=0; ch<16; ++ch) {
        if (ch!=0 && ch%2==0) bit_offset += 2;
        uint32_t channelmask = (multmask << bit_offset);

        uint32_t lowmask = fMultStates[ch].GetMultiplicityMaskLow();
        lowmask = SetSelfChannelMaskBits(lowmask, channelmask);
        // Clear the upper bits [31:16] of lowmask to clear any residual 
        // multiplicity right state
        lowmask = (0xFFFF & lowmask);    
    
        uint32_t highmask = fMultStates[ch].GetMultiplicityMaskHigh();
        highmask = SetSelfMultThresholdBits(highmask,multthresh);
        // Clear bits [15:0] and [30:25] to remove dependence of left
        // multiplicity state and also the right and left mult thresholds
        highmask = (0x81FF0000 & highmask);    

        MultiplicityState state(lowmask,highmask,coinctime);
        
//        state.Print(); 
        new_states.push_back(state);
    }

    return new_states;
}

std::vector<MultiplicityState> MultCoincDialogue::ConstructFiveGroupsOfThreeMultStates()
{
    std::cout << "**** Constructing multiplicity states for 5 x 3 mode " << std::endl;
    std::vector<MultiplicityState> new_states;
    
    uint32_t multthresh = fMultiplicityEntry->GetIntNumber();

    double coinctime = fCoincTimeEntryField->GetNumber();
    if (coinctime<0) {
        std::cerr << "Invalid coincidence time ... new multiplicity states cannot be constructed" << std::endl;
        return new_states;
    }

    uint16_t bit_offset = 0;
    uint32_t bit_pattern;
    for (UInt_t ch=0; ch<16; ++ch) {
        uint32_t channelmask;
        if (ch!=15) {
            bit_pattern = 0x7;
            if (ch!=0 && ch%3==0) bit_offset += 3;
        } else {
            bit_pattern = 0x1;
            bit_offset = 15;
        }

        channelmask = (bit_pattern << bit_offset);

        uint32_t lowmask = fMultStates[ch].GetMultiplicityMaskLow();
        lowmask = SetSelfChannelMaskBits(lowmask, channelmask);
        // Clear the upper bits [31:16] of lowmask to clear any residual 
        // multiplicity right state
        lowmask = (0xFFFF & lowmask);    
    
        uint32_t highmask = fMultStates[ch].GetMultiplicityMaskHigh();
        if (ch<15)
            highmask = SetSelfMultThresholdBits(highmask,multthresh);
        else
            highmask = SetSelfMultThresholdBits(highmask,1);
        // Clear bits [15:0] and [30:25] to remove dependence of left
        // multiplicity state and also the right and left mult thresholds
        highmask = (0x81FF0000 & highmask);    

        MultiplicityState state(lowmask,highmask,coinctime);
        
//        state.Print(); 
        new_states.push_back(state);
    }


    return new_states;
}

std::vector<MultiplicityState> MultCoincDialogue::ConstructFourGroupsOfFourMultStates()
{
    std::cout << "**** Constructing multiplicity states for 4 x 4 mode " << std::endl;
    std::vector<MultiplicityState> new_states;
    
    uint32_t multmask = 0xF;

    uint32_t multthresh = fMultiplicityEntry->GetIntNumber();

    double coinctime = fCoincTimeEntryField->GetNumber();
    if (coinctime<0) {
        std::cerr << "Invalid coincidence time ... new multiplicity states cannot be constructed" << std::endl;
        return new_states;
    }

    uint32_t bit_offset = 0;
    for (UInt_t ch=0; ch<16; ++ch) {
        if (ch!=0 && ch%4==0) bit_offset += 4;
        uint32_t channelmask = (multmask << bit_offset);

        uint32_t lowmask = fMultStates[ch].GetMultiplicityMaskLow();
        lowmask = SetSelfChannelMaskBits(lowmask, channelmask);
        // Clear the upper bits [31:16] of lowmask to clear any residual 
        // multiplicity right state
        lowmask = (0xFFFF & lowmask);    
    
        uint32_t highmask = fMultStates[ch].GetMultiplicityMaskHigh();
        highmask = SetSelfMultThresholdBits(highmask,multthresh);
        // Clear bits [15:0] and [30:25] to remove dependence of left
        // multiplicity state and also the right and left mult thresholds
        highmask = (0x81FF0000 & highmask);    

        MultiplicityState state(lowmask,highmask,coinctime);
        
//        state.Print(); 
        new_states.push_back(state);
    }


    return new_states;
}

std::vector<MultiplicityState> MultCoincDialogue::ConstructTwoGroupsOfEightMultStates()
{
    std::cout << "**** Constructing multiplicity states for 2 x 8 mode " << std::endl;
    std::vector<MultiplicityState> new_states;
    
    uint32_t multmask = 0xFF;

    uint32_t multthresh = fMultiplicityEntry->GetIntNumber();

    double coinctime = fCoincTimeEntryField->GetNumber();
    if (coinctime<0) {
        std::cerr << "Invalid coincidence time ... new multiplicity states cannot be constructed" << std::endl;
        return new_states;
    }

    uint32_t bit_offset = 0;
    for (UInt_t ch=0; ch<16; ++ch) {
        if (ch!=0 && ch%8==0) bit_offset += 8;
        uint32_t channelmask = (multmask << bit_offset);

        uint32_t lowmask = fMultStates[ch].GetMultiplicityMaskLow();
        lowmask = SetSelfChannelMaskBits(lowmask, channelmask);
        // Clear the upper bits [31:16] of lowmask to clear any residual 
        // multiplicity right state
        lowmask = (0xFFFF & lowmask);    
    
        uint32_t highmask = fMultStates[ch].GetMultiplicityMaskHigh();
        highmask = SetSelfMultThresholdBits(highmask,multthresh);
        // Clear bits [15:0] and [30:25] to remove dependence of left
        // multiplicity state and also the right and left mult thresholds
        highmask = (0x81FF0000 & highmask);    

        MultiplicityState state(lowmask,highmask,coinctime);
        
//        state.Print(); 
        new_states.push_back(state);
    }



    return new_states;
}

std::vector<MultiplicityState> MultCoincDialogue::ConstructOneGroupOfSixteenMultStates()
{
    std::cout << "**** Constructing multiplicity states for 1 x 16 mode " << std::endl;
    std::vector<MultiplicityState> new_states;
    
    uint32_t multmask = 0xFFFF;

    uint32_t multthresh = fMultiplicityEntry->GetIntNumber();

    double coinctime = fCoincTimeEntryField->GetNumber();
    if (coinctime<0) {
        std::cerr << "Invalid coincidence time ... new multiplicity states cannot be constructed" << std::endl;
        return new_states;
    }

    for (UInt_t ch=0; ch<16; ++ch) {
        uint32_t channelmask = multmask;

        uint32_t lowmask = fMultStates[ch].GetMultiplicityMaskLow();
        lowmask = SetSelfChannelMaskBits(lowmask, channelmask);
        // Clear the upper bits [31:16] of lowmask to clear any residual 
        // multiplicity right state
        lowmask = (0xFFFF & lowmask);    
    
        uint32_t highmask = fMultStates[ch].GetMultiplicityMaskHigh();
        highmask = SetSelfMultThresholdBits(highmask,multthresh);
        // Clear bits [15:0] and [30:25] to remove dependence of left
        // multiplicity state and also the right and left mult thresholds
        highmask = (0x81FF0000 & highmask);    

        MultiplicityState state(lowmask,highmask,coinctime);
        
//        state.Print(); 
        new_states.push_back(state);
    }

    return new_states;
}

uint32_t MultCoincDialogue::SetSelfChannelMaskBits(uint32_t prevbitpattern, uint32_t newsubpattern)
{
    // mask for the first 16 bits
    uint32_t bitmask = 0xFFFF;

    // Clear the first 16 bits and use this to initialize the 
    // bit pattern to return.
    uint32_t pattern = prevbitpattern & ~bitmask;

    // ensure that no bits about the first 16 have been set
    newsubpattern = newsubpattern & bitmask; 

    // set the bits
    return pattern | newsubpattern;
}

uint32_t MultCoincDialogue::SetSelfMultThresholdBits(uint32_t prevbitpattern, uint32_t newthresh)
{
    // mask to specify bits [24:22]
    uint32_t bitmask = (0x7 << 22);

    // Clear the first 16 bits and use this to initialize the 
    // bit pattern to return.
    uint32_t pattern = prevbitpattern & ~bitmask;

    if (newthresh>7) {
        std::cout << "WARNING! Threshold setting must be less than 8."
            << "Value will be truncated to first 3 bits : It was " 
            << newthresh
            << std::endl;    
    }
    
    // ensure that no bits about the first 16 have been set
    uint32_t newsubpattern = (newthresh & 0x7) << 22; 
    
    // set the bits
    return pattern | newsubpattern;
}

int MultCoincDialogue::WriteMultiplicityState(uint16_t module, uint16_t channel, const MultiplicityState& state)
{
    char pMultiplicityMaskL[]="MultiplicityMaskL";
    char pMultiplicityMaskH[]="MultiplicityMaskH";
    char pChanTrigStretch[]="ChanTrigStretch";

    Int_t result;
    if (module < fDetector->GetNumberModules()) {
        if (state.IsValid()) {

            double data = state.GetMultiplicityMaskLow();
//            std::ios::fmtflags flags = std::cout.flags(std::ios::fixed);
//            std::streamsize prec = std::cout.precision(0);
//            std::cout << "Lowmask (double) = " <<  data << std::endl;
//            std::cout.flags(flags);
//            std::cout.precision(prec);
            result = Pixie16WriteSglChanPar(/*"MultiplicityMaskL"*/pMultiplicityMaskL, data, module, channel);
            if (result<0) {
                std::cerr << "FAILURE lower mask. Reason: ";
                if (result==-1) {
                    std::cerr << "Invalid module number" << std::endl;
                } else if (result==-2) {
                    std::cerr << "Invalid channel number" << std::endl;
                } else if (result==-3) {
                    std::cerr << "Invalid parameter name" << std::endl;
                } else if (result==-4) {
                    std::cerr << "Programming Fippi failed to download channel parameter. Manual hint: Reboot module" << std::endl;
                } else if (result==-5) {
                    std::cerr << "Failed to find BLcut after downloading channel parameter. Manual hint: Reboot module" << std::endl;
                } else if (result==-6) {
                    std::cerr << "SetDACs failed downloading channel parameter. Manual hint: Reboot module" << std::endl;
                } else {
                    std::cerr << "Unknown" << std::endl;
                }
                // don't continue!!;
                return result;
            }

            data = state.GetMultiplicityMaskHigh();
//            flags = std::cout.flags(std::ios::fixed);
//            prec = std::cout.precision(0);
//            std::cout << "Highmask (double) = " <<  data << std::endl;
//            std::cout.flags(flags);
//            std::cout.precision(prec);
            result = Pixie16WriteSglChanPar(/*"MultiplicityMaskH"*/pMultiplicityMaskH, data, module, channel);
            if (result<0) {
                std::cerr << "FAILURE upper mask. Reason: ";
                if (result==-1) {
                    std::cerr << "Invalid module number" << std::endl;
                } else if (result==-2) {
                    std::cerr << "Invalid channel number" << std::endl;
                } else if (result==-3) {
                    std::cerr << "Invalid parameter name" << std::endl;
                } else if (result==-4) {
                    std::cerr << "Programming Fippi failed to download channel parameter. Manual hint: Reboot module" << std::endl;
                } else if (result==-5) {
                    std::cerr << "Failed to find BLcut after downloading channel parameter. Manual hint: Reboot module" << std::endl;
                } else if (result==-6) {
                    std::cerr << "SetDACs failed downloading channel parameter. Manual hint: Reboot module" << std::endl;
                } else {
                    std::cerr << "Unknown" << std::endl;
                }
                // don't continue!!;
                return result;
            }
//
            data = state.GetCoincidenceTime();
//            flags = std::cout.flags(std::ios::fixed);
//            prec = std::cout.precision(0);
//            std::cout << "Coinctime (double) = " << data << std::endl;
//            std::cout.flags(flags);
//            std::cout.precision(prec);
            result = Pixie16WriteSglChanPar(/*"ChanTrigStretch"*/pChanTrigStretch, data, module, channel);
            if (result<0) {
                std::cerr << "FAILURE upper mask. Reason: ";
                if (result==-1) {
                    std::cerr << "Invalid module number" << std::endl;
                } else if (result==-2) {
                    std::cerr << "Invalid channel number" << std::endl;
                } else if (result==-3) {
                    std::cerr << "Invalid parameter name" << std::endl;
                } else if (result==-4) {
                    std::cerr << "Programming Fippi failed to download channel parameter. Manual hint: Reboot module" << std::endl;
                } else if (result==-5) {
                    std::cerr << "Failed to find BLcut after downloading channel parameter. Manual hint: Reboot module" << std::endl;
                } else if (result==-6) {
                    std::cerr << "SetDACs failed downloading channel parameter. Manual hint: Reboot module" << std::endl;
                } else {
                    std::cerr << "Unknown" << std::endl;
                }
                // don't continue!!;
                return result;
            }

        } else {
            std::cerr << "Invalid multiplicity state selected. Write aborted" << std::endl;
        }

    } else {
        std::cerr << "Invalid module number" << std::endl;
    }

    return result;
}

void MultCoincDialogue::OnModeChanged(MultCoincDialogue::MULTMODE new_mode) 
{
    // Two situations result in this being called. 
    // 1. User presses new radiobutton.
    // 2. User selects different module, which then loads the
    //    new module's multiplicity states, which updates the 
    //    multiplicity mode of the dialog.

    // Only the first case should set the values of the multiplicity
    // threshold to a default value.
    // 
    if (fDialogueMode!=new_mode) {
//        std::cout << "Mode changed from " << fDialogueMode << " to " << new_mode << std::endl;
        fDialogueMode = new_mode;
        SetDefaultMultiplicityThreshForMode();
    } 
    SetMultiplicityThreshEntryRange();
    SetButtonStatesForMode();
    SetEntryFieldStatesForMode();
    SetRadioButtonStatesForMode(); 

}

void MultCoincDialogue::SetMultiplicityThreshEntryRange()
{
    switch (fDialogueMode) {
        case OFF:
            fMultiplicityEntry->SetLimits(TGNumberFormat::kNELLimitMinMax,1,0);
            break;
        case EIGHTBYTWO:
            fMultiplicityEntry->SetLimits(TGNumberFormat::kNELLimitMinMax,1,2);
            break;
        case FIVEBYTHREE:
            fMultiplicityEntry->SetLimits(TGNumberFormat::kNELLimitMinMax,1,3);
            break;
        case FOURBYFOUR:
            fMultiplicityEntry->SetLimits(TGNumberFormat::kNELLimitMinMax,1,4);
            break;
        case TWOBYEIGHT:
            fMultiplicityEntry->SetLimits(TGNumberFormat::kNELLimitMinMax,1,7);
            break;
        case ONEBYSIXTEEN:
            fMultiplicityEntry->SetLimits(TGNumberFormat::kNELLimitMinMax,1,7);
            break;
        case UNKNOWN:
            fMultiplicityEntry->SetLimits(TGNumberFormat::kNELLimitMinMax,1,1);
            break;
        default:
            break;
    }
}

void MultCoincDialogue::SetDefaultMultiplicityThreshForMode()
{
    switch (fDialogueMode) {
        case OFF:
            fMultiplicityEntry->SetNumber(0);
            break;
        case EIGHTBYTWO:
            fMultiplicityEntry->SetNumber(2);
            break;
        case FIVEBYTHREE:
            fMultiplicityEntry->SetNumber(3);
            break;
        case FOURBYFOUR:
            fMultiplicityEntry->SetNumber(4);
            break;
        case TWOBYEIGHT:
            fMultiplicityEntry->SetNumber(7);
            break;
        case ONEBYSIXTEEN:
            fMultiplicityEntry->SetNumber(7);
            break;
        case UNKNOWN:
            fMultiplicityEntry->SetNumber(0);
            break;
        default:
            break;
    }
}

void MultCoincDialogue::SetButtonStatesForMode()
{
    switch (fDialogueMode) {
        case OFF:
            fApplyButton->SetEnabled(kTRUE);
            break;
        case EIGHTBYTWO:
            fApplyButton->SetEnabled(kTRUE);
            break;
        case FIVEBYTHREE:
            fApplyButton->SetEnabled(kTRUE);
            break;
        case FOURBYFOUR:
            fApplyButton->SetEnabled(kTRUE);
            break;
        case TWOBYEIGHT:
            fApplyButton->SetEnabled(kTRUE);
            break;
        case ONEBYSIXTEEN:
            fApplyButton->SetEnabled(kTRUE);
            break;
        case UNKNOWN:
            fApplyButton->SetEnabled(kFALSE);
            break;
        default:
            break;
    }
}

void MultCoincDialogue::SetEntryFieldStatesForMode()
{
    switch (fDialogueMode) {
        case OFF:
            fMultiplicityEntry->SetState(kFALSE);
            fCoincTimeEntryField->SetState(kFALSE);
            break;
        case EIGHTBYTWO:
            fMultiplicityEntry->SetState(kTRUE);
            fCoincTimeEntryField->SetState(kTRUE);
            break;
        case FIVEBYTHREE:
            fMultiplicityEntry->SetState(kTRUE);
            fCoincTimeEntryField->SetState(kTRUE);
            break;
        case FOURBYFOUR:
            fMultiplicityEntry->SetState(kTRUE);
            fCoincTimeEntryField->SetState(kTRUE);
            break;
        case TWOBYEIGHT:
            fMultiplicityEntry->SetState(kTRUE);
            fCoincTimeEntryField->SetState(kTRUE);
            break;
        case ONEBYSIXTEEN:
            fMultiplicityEntry->SetState(kTRUE);
            fCoincTimeEntryField->SetState(kTRUE);
            break;
        case UNKNOWN:
            fMultiplicityEntry->SetState(kFALSE);
            fCoincTimeEntryField->SetState(kFALSE);
            break;
        default:
            break;
    }
}

void MultCoincDialogue::OnModuleChanged()
{
    Int_t module = fModuleNumEntry->GetIntNumber();
    //    std::cout << "Module changed from " << fCurrentModule << " to " << module << std::endl;
    // Check to see if there are changes
    //
    Bool_t needs_confirmation = false; 
    // Bool_t needs_confirmation = DialogueAndSavedMultStateDiffer();
    Bool_t ok_to_continue = !needs_confirmation;
    //
    // if ( needs_confirmation ) {
    //      std::cout << "Settings have been changed but not applied. Confirm to continue without applying them" << std::endl;
    //      // ... update ok_to_continue based on response
    // }  
    //  
    if (ok_to_continue) {
        fCurrentModule = module;
        DoLoad();
        
    }

}

void MultCoincDialogue::UpdateTimeAndThreshold()
{
    // Use the first multiplicity state to set these values
    fCoincTimeEntryField->SetNumber(fMultStates[0].GetCoincidenceTime());

    fMultiplicityEntry->SetNumber(fMultStates[0].GetSelfMultiplicityThreshold());

}


MultCoincDialogue::MULTMODE MultCoincDialogue::ParseMultiplicityMode()
{
    // Test each mode
    //
    // Test Off
    
    std::cout << "     ";

    Bool_t good=true;
    for (UInt_t ch=0; ch<16; ++ch) {
        good = good && (fMultStates[ch].GetMultiplicityMaskLow()==0x0);
    } 
    if (good) {
        std::cout << "Detected multiplicity mode = Off" << std::endl;
        std::cout << std::endl;
        return OFF;
    }

    // Test 8x2
    good=true;
    UInt_t bit_offset=0;
    for (UInt_t ch=0; ch<16; ++ch) {
        if (ch!=0 && ch%2==0) bit_offset += 2;
        uint32_t channelmask = (0x3 << bit_offset);
        good = good && (fMultStates[ch].GetMultiplicityMaskLow()==channelmask);
    } 
    if (good) {
        std::cout << "Detected multiplicity mode = 8 x 2" << std::endl;
        std::cout << std::endl;
        return EIGHTBYTWO;
    }

    // Test 5x3
    good=true;
    bit_offset=0;
    for (UInt_t ch=0; ch<16; ++ch) {
        if (ch!=15) {
            if (ch!=0 && ch%3==0) bit_offset += 3;

            uint32_t channelmask = (0x7 << bit_offset);
            good = good && (fMultStates[ch].GetMultiplicityMaskLow()==channelmask);
        }
        else { 
            good = good && (fMultStates[ch].GetMultiplicityMaskLow()==(0x1<<15));
        }
    } 
    if (good) {
        std::cout << "Detected multiplicity mode = 5 x 3" << std::endl;
        std::cout << std::endl;
        return FIVEBYTHREE;
    }

    // Test 4x4
    good=true;
    bit_offset=0;
    for (UInt_t ch=0; ch<16; ++ch) {
        if (ch!=0 && ch%4==0) bit_offset += 4;
        uint32_t channelmask = (0xF << bit_offset);
        good = good && (fMultStates[ch].GetMultiplicityMaskLow()==channelmask);
    } 
    if (good) {
        std::cout << "Detected multiplicity mode = 4 x 4" << std::endl;
        std::cout << std::endl;
        return FOURBYFOUR;
    }

    // Test 2x8
    good=true;
    bit_offset=0;
    for (UInt_t ch=0; ch<16; ++ch) {
        if (ch!=0 && ch%8==0) bit_offset += 8;
        uint32_t channelmask = (0xFF << bit_offset);
        good = good && (fMultStates[ch].GetMultiplicityMaskLow()==channelmask);
    } 
    if (good) {
        std::cout << "Detected multiplicity mode = 2 x 8" << std::endl;
        std::cout << std::endl;
        return TWOBYEIGHT;
    }

    // Test 1x16
    good=true;
    bit_offset=0;
    for (UInt_t ch=0; ch<16; ++ch) {
        if (ch!=0 && ch%16==0) bit_offset += 16;
        uint32_t channelmask = (0xFFFF << bit_offset);
        good = good && (fMultStates[ch].GetMultiplicityMaskLow()==channelmask);
    } 
    if (good) {
        std::cout << "Detected multiplicity mode = 1 x 16" << std::endl;
        std::cout << std::endl;
        return ONEBYSIXTEEN;
    }

    std::cout << "Multiplicity mode in an unsupported state!" << std::endl;
    std::cout << std::endl;

    return UNKNOWN; 
}

void MultCoincDialogue::SetRadioButtonForMode(MultCoincDialogue::MULTMODE new_mode)
{
    switch (new_mode) {
        case OFF:
        fMultModeButtonGrp->SetButton(0,kTRUE);
        break;
        case EIGHTBYTWO:
        fMultModeButtonGrp->SetButton(1,kTRUE);
        break;
        case FIVEBYTHREE:
        fMultModeButtonGrp->SetButton(2,kTRUE);
        break;
        case FOURBYFOUR:
        fMultModeButtonGrp->SetButton(3,kTRUE);
        break;
        case TWOBYEIGHT:
        fMultModeButtonGrp->SetButton(4,kTRUE);
        break;
        case ONEBYSIXTEEN:
        fMultModeButtonGrp->SetButton(5,kTRUE);
        break;
        case UNKNOWN:
        fMultModeButtonGrp->SetButton(6,kTRUE);
        break;
    }
}

void MultCoincDialogue::SetRadioButtonStatesForMode()
{
    Pixel_t red, black;
    fClient->GetColorByName("red",red);
    fClient->GetColorByName("black",black);

    switch (fDialogueMode) {
        case OFF:
        fMultModeRadioButtons[6]->SetEnabled(kFALSE);
        fMultModeRadioButtons[6]->SetTextColor(black,false);
        break;
        case EIGHTBYTWO:
        fMultModeRadioButtons[6]->SetEnabled(kFALSE);
        fMultModeRadioButtons[6]->SetTextColor(black,false);
        break;
        case FIVEBYTHREE:
        fMultModeRadioButtons[6]->SetEnabled(kFALSE);
        fMultModeRadioButtons[6]->SetTextColor(black,false);
        break;
        case FOURBYFOUR:
        fMultModeRadioButtons[6]->SetEnabled(kFALSE);
        fMultModeRadioButtons[6]->SetTextColor(black,false);
        break;
        case TWOBYEIGHT:
        fMultModeRadioButtons[6]->SetEnabled(kFALSE);
        fMultModeRadioButtons[6]->SetTextColor(black,false);
        break;
        case ONEBYSIXTEEN:
        fMultModeRadioButtons[6]->SetEnabled(kFALSE);
        fMultModeRadioButtons[6]->SetTextColor(black,false);
        break;
        case UNKNOWN:
        fMultModeButtonGrp->SetButton(6,kTRUE);
        fMultModeRadioButtons[6]->SetTextColor(red,false);
        break;
    }
}

void MultCoincDialogue::SetBgColorForDetectorMode()
{
    Pixel_t red,gray;
    fClient->GetColorByName("red",red);
    fClient->GetColorByName("gray",gray);    

    if (fDetectorMode==UNKNOWN)
        ChangeBackground(red);
    else
        ChangeBackground(gray);
}

Bool_t MultCoincDialogue::ProcessMessage(Long_t msg, Long_t par1, Long_t par2)
{

//    std::cout << "kC_COMMAND      = " << kC_COMMAND << std::endl;
//    std::cout << "kCM_RADIOBUTTON = " << kCM_RADIOBUTTON << std::endl;
//    std::cout << "GET_MSG(msg)    = " << GET_MSG(msg) << std::endl;
//    std::cout << "GET_SUBMSG(msg) = " << GET_MSG(msg) << std::endl;
//    std::cout << "par1            = " << par1 << std::endl;
//    std::cout << "par2            = " << par2 << std::endl;

    MULTMODE newmode=OFF;
    Bool_t goodmode=true;
    switch (GET_MSG (msg))
    {
        case kC_COMMAND:

            switch (GET_SUBMSG (msg)) {
                case kCM_BUTTON:
                    switch(par1) {
                        case APPLYBUTTON:
                            OnApply();
                            break;
                        case CANCELBUTTON:
                            OnCancel();
                            break;
                        case LOADBUTTON:
                            OnLoad();
                            break;
                        default:
                            break;
                    }
                    break; 

                case kCM_RADIOBUTTON:
                    if (par1==0) newmode=OFF;
                    else if (par1==1) newmode=EIGHTBYTWO;
                    else if (par1==2) newmode=FIVEBYTHREE;
                    else if (par1==3) newmode=FOURBYFOUR;
                    else if (par1==4) newmode=TWOBYEIGHT;
                    else if (par1==5) newmode=ONEBYSIXTEEN;
                    else if (par1==6) newmode=UNKNOWN;
                    else goodmode=false;
                    if (goodmode)
                        OnModeChanged(newmode);
                    else 
                        std::cout << "Undefined mode " << par1 << std::endl;

                    break;
                default:
                    break;

            } // end of submsg switch
            break;
        case kC_TEXTENTRY:
            switch (GET_SUBMSG (msg)) {
                case kTE_TEXTCHANGED:
                    OnModuleChanged();            
                    break;
                default:
                    break;
            } // end of submsg switch
            break;
        default:
            break;

    } // end of message switch 

    return kTRUE;

}




