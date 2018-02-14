// TimingParamDialogue.cpp
//
// Author : Jeromy Tompkins

#include "TimingParamDialogue.h"

TimingParamDialogue::TimingParamDialogue(const TGWindow * p, const TGWindow * main, int NumModules)
: TGTransientFrame(p,main,600,600,kHorizontalFrame)
{ 
    DontCallClose();
    SetMWMHints(kMWMDecorBorder|kMWMDecorTitle,kMWMFuncResize|kMWMFuncMove, 0);
    SetCleanup(kDeepCleanup);

    //Int_t rows = 16;
    //Int_t cols = 5;

    // Remove all frames to begin setting a new layout
    // Create a new horizontal layout
    fHFrame = new TGHorizontalFrame(this);

    // Set up the search path for the images
    m_imageLocator.addPath("./resources");
#ifdef PREFIX
    m_imageLocator.addPath(std::string(PREFIX) + "/share/nscope");
#endif

    std::string imagePath = m_imageLocator.locateFile("TimingParamDialogueDiagram.png");
    if (!imagePath.empty()) {
      fPic = new TGIcon(fHFrame, imagePath.c_str());
      fPic->Resize(614,398);
      fHFrame->AddFrame(fPic,new TGLayoutHints(kLHintsLeft,5,5,5,5));
    }

    // build mn_vert
    MakeTable(fHFrame, 6, 16, NumModules);
    fHFrame->AddFrame(mn_vert, new TGLayoutHints(kLHintsRight|kLHintsExpandY,5,5,5,5));

    AddFrame(fHFrame, new TGLayoutHints(kLHintsExpandX|kLHintsExpandY,5,5,5,5));

//    TList* list = GetList();
//    std::cout << "This ..." << std::endl;
//    for (UInt_t i=0; i<list->GetEntries(); ++i) {
//        std::cout << "entry" << i << " @ " << ((TGFrameElement*) list->At(i))->fFrame << std::endl;
//    }
//    list = fHFrame->GetList();
//    std::cout << "fHFrame ..." << std::endl;
//    for (UInt_t i=0; i<list->GetEntries(); ++i) {
//        std::cout << "entry" << i << " @ " << ((TGFrameElement*) list->At(i))->fFrame << std::endl;
//    }
//    list = mn_vert->GetList();
//    std::cout << "mn_vert ..." << std::endl;
//    for (UInt_t i=0; i<list->GetEntries(); ++i) {
//        std::cout << "entry" << i << " @ " << ((TGFrameElement*) list->At(i))->fFrame << std::endl;
//    }

    Load_Once = true;
    chanNumber = 0;
    tdelay = 0;
    twidth = 0;
    //thresh = 0;

    MapSubwindows();
    Resize();
    CenterOnParent();

    SetWindowName("Controls for Pixie16 Timing Parameters");
    MapWindow();
}

void TimingParamDialogue::UpdateChanCoincWidthColumn()
{
    
    double ChanParData = -1;

    int retval;
    char text[20];
    char pChanTrigStretch[]="ChanTrigStretch";

    for (int i = 0; i < 16; i++) {
        retval =
	  Pixie16ReadSglChanPar(/*"ChanTrigStretch"*/pChanTrigStretch, &ChanParData,
                    modNumber, i);
	if(retval < 0) std::cout << "Error in TimingParamDialogue UpdateChanCoin" << std::endl;
        sprintf(text, "%1.3f", ChanParData);
        NumEntry[4][i]->SetText(text);

    }
}

void TimingParamDialogue::MakeTable(TGFrame* parent, int columns, int rows, int NumModules)
{

    numModules=NumModules;

    mn_vert = new TGVerticalFrame (parent, 200, 300);
    mn = new TGHorizontalFrame (mn_vert, 200, 300);
    mn_vert->AddFrame (mn,
            new TGLayoutHints (kLHintsTop | kLHintsLeft, 0, 0, 0,
                0));

    Buttons = new TGHorizontalFrame (mn_vert, 400, 300);


    Column = new TGVerticalFrame *[columns];
    for (int i = 0; i < columns; i++)
    {
        Column[i] = new TGVerticalFrame (mn, 200, 300);
        mn->AddFrame (Column[i],
                new TGLayoutHints (kLHintsTop | kLHintsLeft, 0, 0, 0, 0));
    }


    Int_t colwidth=95;

    //////////////////labels column///////////////////////////////////////////  
    cl0 =
        new TGTextEntry (Column[0], new TGTextBuffer (100), 10000,
                cl0->GetDefaultGC ()(),
                cl0->GetDefaultFontStruct (),
                kRaisedFrame | kDoubleBorder, GetWhitePixel ());
    cl0->SetFont ("-adobe-helvetica-bold-r-*-*-10-*-*-*-*-*-iso8859-1", false);

    //cl0->SetText ("Ch #");
    cl0->Resize (35, 40);
    cl0->SetEnabled (kFALSE);
    cl0->SetFrameDrawn (kTRUE);
    Column[0]->AddFrame (cl0, new TGLayoutHints (kLHintsCenterX, 0, 0, 10, 0));

    Labels = new TGTextEntry *[rows];

    for (int i = 0; i < rows; i++)
    {
        Labels[i] = new TGTextEntry (Column[0], new TGTextBuffer (100), 10000,
                Labels[i]->GetDefaultGC ()(),
                Labels[i]->GetDefaultFontStruct (),
                kRaisedFrame | kDoubleBorder,
                GetWhitePixel ());
        Labels[i]->Resize (35, 20);
        Labels[i]->SetEnabled (kFALSE);
        Labels[i]->SetFrameDrawn (kTRUE);

        Column[0]->AddFrame (Labels[i],
                new TGLayoutHints (kLHintsCenterX, 0, 3, 0, 0));
    }


    /////////////////////////////////////////////////

    CLabel = new TGTextEntry *[columns - 1];
    for (int i = 0; i < columns - 1; i++)
    {
        CLabel[i] =
            new TGTextEntry (Column[i + 1], new TGTextBuffer (100), 10000,
                    CLabel[i]->GetDefaultGC ()(),
                    CLabel[i]->GetDefaultFontStruct (),
                    kRaisedFrame | kDoubleBorder, GetWhitePixel ());

        CLabel[i]->
            SetFont ("-adobe-helvetica-bold-r-*-*-10-*-*-*-*-*-iso8859-1", false);
        //->SetText ("Trace Length[us]");
        CLabel[i]->Resize (colwidth, 40);
        CLabel[i]->SetEnabled (kFALSE);
        CLabel[i]->SetFrameDrawn (kTRUE);
        Column[i + 1]->AddFrame (CLabel[i],
                new TGLayoutHints (kLHintsCenterX|kLHintsCenterY, 0, 0, 10,
                    0));
    }

    /////////////////////////////////////////////////
    NumEntry = new TGNumberEntryField **[columns];
    for (int i = 0; i < columns; i++)
        NumEntry[i] = new TGNumberEntryField *[rows];

    for (int i = 1; i < columns; i++)
    {

        for (int j = 0; j < rows; j++)
        {
            NumEntry[i][j] = new TGNumberEntryField (Column[i], i, 0,
                    TGNumberFormat::kNESReal);
            NumEntry[i][j]->Resize (colwidth, 20);
            NumEntry[i][j]->Associate (this);
            Column[i]->
                AddFrame (NumEntry[i][j],
                        new TGLayoutHints (kLHintsCenterX, 0, 0, 0, 0));
        }

    }

    // Disable the final column for channel trig 
    for (int i=0; i<rows; ++i) {
        NumEntry[4][i]->SetState(kFALSE);

    }

    /////////////////////////////////////////////////////////////////
    ///////////////////////Buttons making ...///////////////////////

    /////////////////////////////module entry///////////////////////////////

//    TGHorizontal3DLine *ln1 = new TGHorizontal3DLine (Column[0], 30, 2);
    TGLabel *mod = new TGLabel (Buttons, "Module #");
    TGLabel* comment = new TGLabel(mn_vert,"* All numbers in units of us");
    mn_vert->AddFrame(comment,new TGLayoutHints(kLHintsRight,2,2,2,2));

    numericMod = new TGNumberEntry (Buttons, 0, 4, MODNUMBER, (TGNumberFormat::EStyle) 0, (TGNumberFormat::EAttribute) 1, (TGNumberFormat::ELimit) NumModules,	// kNELLimitMinMax
            0, NumModules);
    numericMod->SetButtonToNum (0);
    numericMod->IsEditable();
//    Column[0]->AddFrame (ln1,
//            new TGLayoutHints (kLHintsCenterX | kLHintsCenterY, 0, 0,
//                10, 10));
    Buttons->AddFrame (mod, new TGLayoutHints (kLHintsCenterX, 5, 10, 3, 0));
    Buttons->AddFrame (numericMod,
            new TGLayoutHints (kLHintsTop | kLHintsLeft, 0, 20, 0,
                0));

    numericMod->Associate (this);
    mn_vert->AddFrame (Buttons,
            new TGLayoutHints (kLHintsTop | kLHintsLeft, 0, 0, 0,
                0));
    /////////////////////////////////////////////////////////////////////////
    ////////////////////////////Buttons/////////////////////////////////////
    TGTextButton* LoadButton = new TGTextButton (Buttons, "&Load", LOAD);
    LoadButton->Associate (this);
    TGTextButton* ApplyButton = new TGTextButton (Buttons, "&Apply", APPLY);
    ApplyButton->Associate (this);
    TGTextButton* CancelButton = new TGTextButton (Buttons, "&Cancel", CANCEL);
    CancelButton->Associate (this);
    Buttons->AddFrame (LoadButton,
            new TGLayoutHints (kLHintsCenterX, 0, 0, 0, 0));
    Buttons->AddFrame (ApplyButton,
            new TGLayoutHints (kLHintsCenterX, 0, 0, 0, 0));
    Buttons->AddFrame (CancelButton,
            new TGLayoutHints (kLHintsCenterX, 0, 0, 0, 0));


    ///////////////////////////////////////////////////////////////////
    char n[10];
    cl0->SetText("ch #");
    for (int i = 0; i < rows; i++) {
        sprintf(n, "%2d", i);
        Labels[i]->SetText(n);
    }
    CLabel[0]->SetText("FastTrig Delay*");
    CLabel[0]->SetToolTipText("ExternDelayLen");
    CLabel[0]->SetAlignment(kTextCenterX);
    CLabel[1]->SetText("OR Delay*");
    CLabel[1]->SetToolTipText("FtrigoutDelay");
    CLabel[1]->SetAlignment(kTextCenterX);
    CLabel[2]->SetText("OR Width*");
    CLabel[2]->SetToolTipText("FASTTRIGBACKLEN");
    CLabel[2]->SetAlignment(kTextCenterX);
    CLabel[3]->SetText("Ch. Coinc Width*");
    CLabel[3]->SetToolTipText("ChanTrigStretch");
    CLabel[3]->SetAlignment(kTextCenterX);
    CLabel[4]->SetText("ExtTrig Width*");
    CLabel[4]->SetToolTipText("ExtTrigStretch");
    CLabel[4]->SetAlignment(kTextCenterX);
    modNumber = 0;
    Load_Once = false;

    /////////////////Copy Button////////////////////////////////
    TGHorizontal3DLine *ln2 = new TGHorizontal3DLine(mn_vert, 200, 2);
    mn_vert->AddFrame(ln2,
            new TGLayoutHints(kLHintsLeft | kLHintsCenterY, 0,
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

}


TimingParamDialogue::~TimingParamDialogue()
{
    
}

Bool_t TimingParamDialogue::ProcessMessage(Long_t msg, Long_t parm1,
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
                            if (Load_Once) {
                                change_values(modNumber);
                                load_info(modNumber);
                            } else {
                                std::cout << "please load once first !\n";
                            }
                            break;
                        case CANCEL:		/// Cancel Button
                            //DeleteWindow();
                            UnmapWindow();
                            break;
                        case (COPYBUTTON + 1000):
                            DoCopy();
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

void TimingParamDialogue::DoCopy()
{
    
    // Get the most up-to-date channel number
    int chan = chanCopy->GetNumber();

    // Get the values associated with that channel number
    double extDelayLen = NumEntry[1][chan]->GetNumber();
    double fTrigoutDelay = NumEntry[2][chan]->GetNumber();
    double fTrigBLen = NumEntry[3][chan]->GetNumber();
//    double chanTrigStretch = NumEntry[4][chan]->GetNumber();
    double extTrigStretch = NumEntry[5][chan]->GetNumber();
    
    // Fill the other channels with the values retrieved from desired ch.
    for (int i = 0; i < 16; i++) {
        if (i != chanNumber) {
            sprintf(tmp, "%1.3f", extDelayLen);
            NumEntry[1][i]->SetText(tmp);
            sprintf(tmp, "%1.3f", fTrigoutDelay);
            NumEntry[2][i]->SetText(tmp);
            sprintf(tmp, "%1.3f", fTrigBLen);
            NumEntry[3][i]->SetText(tmp);

            // skip NumEntry[4] for chan. trig stretch

            sprintf(tmp, "%1.3f", extTrigStretch);
            NumEntry[5][i]->SetText(tmp);
        }
    }

}

int TimingParamDialogue::load_info(Long_t module)
{

    double ChanParData = -1;

    int retval;
    char text[20];
    char pExternDelayLen[]="ExternDelayLen";
    char pFtrigoutDelay[]="FtrigoutDelay";
    char pFASTTRIGBACKLEN[]="FASTTRIGBACKLEN";
    char pChanTrigStretch[]="ChanTrigStretch";
    char pExtTrigStretch[]="ExtTrigStretch";

    for (int i = 0; i < 16; i++) {
        retval =
	  Pixie16ReadSglChanPar(/*"ExternDelayLen"*/pExternDelayLen, &ChanParData,
                    modNumber, i);
        sprintf(text, "%1.3f", ChanParData);
        NumEntry[1][i]->SetText(text);

        retval =
	  Pixie16ReadSglChanPar(/*"FtrigoutDelay"*/pFtrigoutDelay, &ChanParData,
                    modNumber, i);
        sprintf(text, "%1.3f", ChanParData);
        NumEntry[2][i]->SetText(text);

        retval =
	  Pixie16ReadSglChanPar(/*"FASTTRIGBACKLEN"*/pFASTTRIGBACKLEN, &ChanParData,
                    modNumber, i);
        sprintf(text, "%1.3f", ChanParData);
        NumEntry[3][i]->SetText(text);

        retval =
	  Pixie16ReadSglChanPar(/*"ChanTrigStretch"*/pChanTrigStretch, &ChanParData,
                    modNumber, i);
        sprintf(text, "%1.3f", ChanParData);
        NumEntry[4][i]->SetText(text);

        retval =
	  Pixie16ReadSglChanPar(/*"ExtTrigStretch"*/pExtTrigStretch, &ChanParData,
                    modNumber, i);
        sprintf(text, "%1.3f", ChanParData);
        NumEntry[5][i]->SetText(text);

    }
    std::cout << "loading info for module " << module << std::endl;

    return retval;
}


int TimingParamDialogue::change_values(Long_t module)
{

    double extDelayLen;
    double fTrigoutDelay;
    double fTrigBLen;
    //double chanTrigStretch;
    double extTrigStretch;
    char pExternDelayLen[]="ExternDelayLen";
    char pFtrigoutDelay[]="FtrigoutDelay";
    char pFASTTRIGBACKLEN[]="FASTTRIGBACKLEN";
    //char pChanTrigStretch[]="ChanTrigStretch";
    char pExtTrigStretch[]="ExtTrigStretch";

    using std::setw;
    std::ios::fmtflags flags = std::cout.flags(std::ios::fixed);
    std::streamsize prec = std::cout.precision(2);

    std::cout << "Applying the following settings to module " << modNumber << std::endl;

    // Write the header for the table of values to be printed
    std::cout << "\n" << setw(4) << "Ch#" << " "
        << setw(12) << "FTrig Delay" << " " 
        << setw(12) << "OR Delay" << " "
        << setw(12) << "OR Width" << " "
        << setw(16) << "ExtTrig Width"
        << std::endl;
    std::cout << std::setfill('-');
    std::cout << setw(4) << "-" << " "
        << setw(12) << "-" << " "
        << setw(12) << "-" << " "
        << setw(12) << "-" << " "
        << setw(16) << "-" << std::endl;
    std::cout << std::setfill(' ');
    
    for (int i = 0; i < 16; i++) {
        std::cout << setw(4) << i << " ";

        extDelayLen = NumEntry[1][i]->GetNumber();
        Pixie16WriteSglChanPar(/*"ExternDelayLen"*/pExternDelayLen, extDelayLen, modNumber, i);
        std::cout << setw(12) << extDelayLen << " ";
    
        fTrigoutDelay = NumEntry[2][i]->GetNumber();
        Pixie16WriteSglChanPar(/*"FtrigoutDelay"*/pFtrigoutDelay, fTrigoutDelay, modNumber, i);
        std::cout << setw(12) << fTrigoutDelay << " ";

        fTrigBLen = NumEntry[3][i]->GetNumber();
        Pixie16WriteSglChanPar(/*"FASTTRIGBACKLEN"*/pFASTTRIGBACKLEN, fTrigBLen, modNumber, i);
        std::cout << setw(12) << fTrigBLen << " ";

        // Note that this skips NumEntry[4] (channel trig stretch)

        extTrigStretch = NumEntry[5][i]->GetNumber();
        Pixie16WriteSglChanPar(/*"ExtTrigStretch"*/pExtTrigStretch, extTrigStretch, modNumber, i);
        std::cout << setw(16) << extTrigStretch << " ";

        std::cout << std::endl;
    }

    std::cout.flags(flags);
    std::cout.precision(prec);

    return 1;
}
