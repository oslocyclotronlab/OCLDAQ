
#include "rupdate.h"

#include "net_control.h"
#include "io_root.h"
#include "sort_spectra.h"

#include <TApplication.h>
#include <TCanvas.h>
#include <TDatime.h>
#include <TFile.h>
#include <TG3DLine.h>
#include <TGButton.h>
#include <TGButtonGroup.h>
#include <TGClient.h>
#include <TGComboBox.h>
#include <TGLabel.h>
#include <TH1.h>
#include <TH2.h>
#include <TGNumberEntry.h>
#include <TRootEmbeddedCanvas.h>
#include <TGStatusBar.h>
#include <TGTextEntry.h>
#include <TStyle.h>
#include <TSystem.h>
#include <TText.h>
#include <TTimer.h>

#include <cmath>
#include <iostream>

#include <stdlib.h>
#include <stdio.h>
#include <sys/time.h>
#include <unistd.h>

#define NDEBUG
#include "debug.h"

ClassImp(MyMainFrame)

// ########################################################################
// ########################################################################

class FrameCB : public line_callback {
public:
    typedef void (MyMainFrame::*cb_t)();

    FrameCB(MyMainFrame* _mmf, cb_t _cb)
        : mmf( _mmf ), cb( _cb ) { }

    virtual void run(line_channel*)
        { (mmf->*cb)(); }
private:
    MyMainFrame* mmf;
    cb_t cb;
};

// ########################################################################

MyMainFrame::MyMainFrame(int maxupdate)
   : TGMainFrame(gClient->GetRoot(), 400, 500)
   , ior( new io_root() )
   , lc_sort( 0 )
   , nasp( 0 )
   , h_matrix( 0 )
   , h_sum( 0 )
   , update_rate( maxupdate )
   , real_update_rate( update_rate - 0.05 )
   , time_last( 0 )
{
    SetCleanup(kDeepCleanup);

    // ============================================================
    // -------------------- button bar
    TGHorizontalFrame *bbar = new TGHorizontalFrame(this);//, 100, 10);

    // -------------------- combo box for spectrum selection
    TGLabel *label = new TGLabel(bbar, "Matrix");
    bbar->AddFrame(label, new TGLayoutHints(kLHintsLeft|kLHintsCenterY));

    cmb_spectra = new TGComboBox(bbar);
    //cmb_spectra->GetTextEntry()->SetToolTipText("Select the matrix to display");
    cmb_spectra->Connect("Selected(Int_t)","MyMainFrame",this,"OnComboSpectraSelect(Int_t)");
    bbar->AddFrame(cmb_spectra, new TGLayoutHints(kLHintsLeft|kLHintsCenterY,2,2,2,2));

    TGVertical3DLine *separator = new TGVertical3DLine(bbar);
    bbar->AddFrame(separator, new TGLayoutHints(kLHintsLeft|kLHintsExpandY));

    // -------------------- projection buttons etc.
    b_project = new TGCheckButton(bbar, "Project", 71);
    b_project->SetToolTipText("Project the selected matrix.");
    b_project->Connect("Clicked()","MyMainFrame",this,"OnButtonProject()");
    bbar->AddFrame(b_project, new TGLayoutHints(kLHintsLeft|kLHintsCenterY,2,2,2,2));

    label = new TGLabel(bbar, "row(s)");
    bbar->AddFrame(label, new TGLayoutHints(kLHintsLeft|kLHintsCenterY));

    n_from = new TGNumberEntry(bbar, 0, 3, -1, TGNumberFormat::kNESInteger);
    //n_from->SetToolTipText("First row for projection (0 based).");
    n_from->Connect("ValueSet(Long_t)", "MyMainFrame", this, "OnProjectFrom(Long_t)");
    bbar->AddFrame(n_from, new TGLayoutHints(kLHintsLeft|kLHintsCenterY,2,2,2,2));

    label = new TGLabel(bbar, "to");
    bbar->AddFrame(label, new TGLayoutHints(kLHintsLeft|kLHintsCenterY));

    n_to = new TGNumberEntry(bbar, 0, 3, -1, TGNumberFormat::kNESInteger);
    //n_to->SetToolTipText("Last row for projection (0 based).");
    n_to->Connect("ValueSet(Long_t)", "MyMainFrame", this, "OnProjectTo(Long_t)");
    bbar->AddFrame(n_to, new TGLayoutHints(kLHintsLeft|kLHintsCenterY,2,2,2,2));

    label = new TGLabel(bbar, "show");
    bbar->AddFrame(label, new TGLayoutHints(kLHintsLeft|kLHintsCenterY));

    r_sum  = new TGRadioButton(bbar, new TGHotString("&sum"));
    r_sum->SetToolTipText("Show the sum of all projected rows.");
    r_sum->Connect("Clicked()","MyMainFrame",this,"OnProjectSum()");
    bbar->AddFrame(r_sum, new TGLayoutHints(kLHintsLeft|kLHintsCenterY,2,2,2,2));

    r_single = new TGRadioButton(bbar, new TGHotString("s&ingle"));
    r_single->SetToolTipText("Show all projected rows in one plot.");
    r_single->Connect("Clicked()","MyMainFrame",this,"OnProjectSingle()");
    bbar->AddFrame(r_single, new TGLayoutHints(kLHintsLeft|kLHintsCenterY,2,2,2,2));

    r_multi = new TGRadioButton(bbar, new TGHotString("&multi"));
    r_multi->SetToolTipText("Show one plot for each of the projected rows.");
    r_multi->Connect("Clicked()","MyMainFrame",this,"OnProjectMulti()");
    bbar->AddFrame(r_multi, new TGLayoutHints(kLHintsLeft|kLHintsCenterY,2,2,2,2));

    separator = new TGVertical3DLine(bbar);
    bbar->AddFrame(separator, new TGLayoutHints(kLHintsLeft|kLHintsExpandY));

    // -------------------- buttons to enable automatic update
    b_autoupdate = new TGCheckButton(bbar,"&Update");
    b_autoupdate->SetToolTipText("Automatic update when sorting, (disable when zooming etc.!)");
    b_autoupdate->Connect("Clicked()","MyMainFrame",this,"OnButtonAutoUpdate()");
    bbar->AddFrame(b_autoupdate, new TGLayoutHints(kLHintsLeft|kLHintsCenterY,2,2,2,2));

    AddFrame(bbar,new TGLayoutHints(kLHintsLeft|kLHintsTop,2,2,2,2));

    // combobox needs resizing...
    cmb_spectra->Resize(100, n_from->GetHeight());

    // ============================================================
    // -------------------- print button bar
    TGHorizontalFrame *pbar = new TGHorizontalFrame(this);

    TGTextButton *b_elog = new TGTextButton(pbar, new TGHotString("&Elog"));
    b_elog->Connect("Clicked()", "MyMainFrame",this,"OnButtonElog()");
    pbar->AddFrame(b_elog, new TGLayoutHints(kLHintsLeft|kLHintsCenterY,2,2,2,2));

    label = new TGLabel(pbar, "Print prefix:");
    pbar->AddFrame(label, new TGLayoutHints(kLHintsLeft|kLHintsCenterY));

    t_printprefix = new TGTextEntry(pbar, new TGTextBuffer(20), 1);
    t_printprefix->SetText("rupdate");
    pbar->AddFrame(t_printprefix, new TGLayoutHints(kLHintsLeft|kLHintsCenterY,2,2,2,2));

    TGTextButton *b_print = new TGTextButton(pbar, new TGHotString("&Print"));
    b_print->Connect("Clicked()", "MyMainFrame",this,"OnButtonPrint()");
    pbar->AddFrame(b_print, new TGLayoutHints(kLHintsLeft|kLHintsCenterY,2,2,2,2));

    // -------------------- separator
    separator = new TGVertical3DLine(pbar);
    pbar->AddFrame(separator, new TGLayoutHints(kLHintsLeft|kLHintsExpandY));

    // -------------------- lin/log buttons
    TGTextButton *b_linear = new TGTextButton(pbar, new TGHotString("L&inear"));
    b_linear->Connect("Clicked()", "MyMainFrame",this,"OnButtonLinear()");
    pbar->AddFrame(b_linear, new TGLayoutHints(kLHintsLeft|kLHintsCenterY,2,2,2,2));

    TGTextButton *b_log = new TGTextButton(pbar, new TGHotString("L&og"));
    b_log->Connect("Clicked()", "MyMainFrame",this,"OnButtonLog()");
    pbar->AddFrame(b_log, new TGLayoutHints(kLHintsLeft|kLHintsCenterY,2,2,2,2));

    AddFrame(pbar,new TGLayoutHints(kLHintsLeft|kLHintsTop,2,2,2,2));

    // ============================================================
    // -------------------- embedded canvas
    ecanvas = new TRootEmbeddedCanvas ("Ecanvas", this, 600, 400);
    ecanvas->GetCanvas()->Connect("ProcessedEvent(Int_t,Int_t,Int_t,TObject*)",
                                  "MyMainFrame", this, "OnCanvasEvent(Int_t,Int_t,Int_t,TObject*)");
    AddFrame(ecanvas, new TGLayoutHints(kLHintsExpandX | kLHintsExpandY, 2,2,2,2));

    // ============================================================
    // -------------------- status bar
    int parts[] = { 20, 10, 10, 60 };
    statusbar = new TGStatusBar(this);
    statusbar->SetParts(parts, 4);
    AddFrame(statusbar, new TGLayoutHints(kLHintsBottom|kLHintsLeft|kLHintsExpandX, 2,2,1,1));

    // ============================================================
    // set startup values
    for( int i=1; sort_spectra[i].name; i++)
        cmb_spectra->AddEntry(sort_spectra[i].name, i);

    // start without projection
    b_project->SetState(kButtonUp);
    r_sum->SetState(kButtonDown);
    OnButtonProject();

    // ============================================================
    // set window name and show the main frame
    SetWindowName("ROOT update");
    MapSubwindows();
    Resize(GetDefaultSize());
    MapWindow();

    // ============================================================
    // select first spectrum to display (NASP)
    cmb_spectra->Select(8);

    timer_reconnect = new TTimer();
    timer_reconnect->Connect("Timeout()","MyMainFrame",this,"OnReconnectTimeout()");
    
    // connect to sort (pretend button press)
    b_autoupdate->SetState(kButtonDown);
    OnButtonAutoUpdate();
    // OnReconnectTimeout();

    ReadCommands();
}

// ########################################################################

void MyMainFrame::ReadCommands()
{
    if( commands.read("acq_master_commands.txt") ) {
        statusbar->SetText("Using commands from acq_master_commands.txt.",3);
    } else {
        statusbar->SetText("Could not read commands from acq_master_commands.txt.",3);
    }
}

// ########################################################################

MyMainFrame::~MyMainFrame()
{
    delete lc_sort;
    delete ior;

    DeleteHistograms();
}

// ########################################################################

/**
 * Called if the window is closed.
 *
 * We terminate the application.
 */
void MyMainFrame::CloseWindow()
{
    gApplication->Terminate(0);
}

// ########################################################################

/**
 * Called when the connect/disconnect button has been clicked.
 */
void MyMainFrame::OnButtonAutoUpdate()
{
    if( b_autoupdate->GetState() == kButtonDown ) {
        // auto-update is now on
        OnReconnectTimeout();
    } else if( b_autoupdate->GetState() == kButtonUp ) {
        // auto-update is now off
        timer_reconnect->Stop();
        
        delete lc_sort;
        lc_sort = 0;
    }

    // update the histogram display
    UpdateHistogram();
}

// ########################################################################

/**
 * Called when the re-connect timer runs out.
 */
void MyMainFrame::OnReconnectTimeout()
{
    if( !lc_sort && b_autoupdate->GetState() == kButtonDown ) {
        // if not connected, try to connect to sort process;
        // (line_connect returns 0 if connection fails)
        lc_sort = line_connect(*ior, "127.0.0.1", 32010,
                               new FrameCB(this, &MyMainFrame::SortDisconnected),
                               new FrameCB(this, &MyMainFrame::SortHaveLine));
        if( !lc_sort ) {
            timer_reconnect->Start(1000, kFALSE);
        } else {
            timer_reconnect->Stop();
        }
    }
}

// ########################################################################

std::string MyMainFrame::SaveAsROOTAndPNG(std::string const& prefix)
{
    const char* DIR = "matrixshot";
    std::ostringstream cmd_mkdir;
    cmd_mkdir <<"test -d " << DIR << " || mkdir " << DIR;
    gSystem->Exec(cmd_mkdir.str().c_str());

    char tmp[1024];
    TDatime now;
    sprintf( tmp, "%s_%04d_%02d_%02d_%02d_%02d_%02d", prefix.c_str(), now.GetYear(),
             now.GetMonth(), now.GetDay(), now.GetHour(), now.GetMinute(), now.GetSecond() );

    std::string base = DIR;
    base += "/";
    base += tmp;

    TCanvas* c = ecanvas->GetCanvas();

    // print prefix + date on canvas
    TVirtualPad* p = c->cd();
    TText* n = new TText(0.1, 0.005, tmp);
    n->SetTextSize(0.025);
    n->SetNDC();
    n->Draw();
    c->Update();

    // save as .png and as .root
    char filename[1024];
    snprintf(filename, sizeof(filename), "%s.png", base.c_str());
    c->SaveAs(filename);

#if 0
    snprintf(filename, sizeof(filename), "%s.root", base.c_str());
    c->SaveAs(filename);
#else
    {
        snprintf(filename, sizeof(filename), "%s.root", base.c_str());
        TFile ofile(filename, "recreate");
        if( !IsProjecting() ) {
            TH2* hm = GetHistogramMatrix();
            hm->Write();
        } else if( IsProjectingSum() ) {
            TH1* hs = GetHistogramSum();
            hs->Write();
        } else {
            const int fr = n_from->GetIntNumber();
            const int to = n_to  ->GetIntNumber();
            for(int j=fr; j<=to; j++) {
                TH1* h = GetHistogramRow(j);
                h->Write();
            }
        }
        ofile.Close();
    }
#endif

    delete n;
    c->Update();
    p->cd();

    return base;
}

// ########################################################################

/**
 * Called when the elog button has been clicked.
 */
void MyMainFrame::OnButtonElog()
{
    std::string basename = SaveAsROOTAndPNG(nasp->name);

    // attach to elog
    std::vector<std::string> args;
    args.push_back("-f");
    args.push_back(basename+".png");
    args.push_back("-f");
    args.push_back(basename+".root");
    args.push_back("-a");
    std::ostringstream name;
    name << nasp->name;
    if( IsProjecting() ) {
        int from = n_from->GetIntNumber();
        int to   = n_to  ->GetIntNumber();
        if( from == to )
            name << " row " << from;
        else
            name << " rows " << from << " to " << to;
    }
    args.push_back(std::string("Subject=screenshot ")+name.str());
    args.push_back(std::string("rupdate screenshot of ")+name.str());
    commands.run("elog", args);

    char status[1024];
    snprintf(status, sizeof(status), "sent '%s' to elog", basename.c_str());
    statusbar->SetText(status,3);
}

// ########################################################################

/**
 * Called when the print button has been clicked.
 */
void MyMainFrame::OnButtonPrint()
{
    std::string prefix = t_printprefix->GetText();
    if( prefix.empty() ) {
        statusbar->SetText("bad print prefix, cannot print",3);
        return;
    }
    if( prefix.find_first_of("\"'*?&$@{[]}=+`\\´~|!;:<> /.") != std::string::npos ) {
        statusbar->SetText("illegal prefix, will not print",3);
        return;
    }

    std::string basename = SaveAsROOTAndPNG(prefix);

    // convert to pdf and print
    char cmd_pdf_lpr[1024];
    snprintf(cmd_pdf_lpr, sizeof(cmd_pdf_lpr),
             "( convert -border 30x30 -bordercolor white %s.png %s.pdf && xpdf %s.pdf ) &", 
             basename.c_str(), basename.c_str(), basename.c_str());
    gSystem->Exec(cmd_pdf_lpr);

    char status[1024];
    snprintf(status, sizeof(status), "printed '%s'", basename.c_str());
    statusbar->SetText(status,3);
}

// ########################################################################

/**
 * Called when the 'linear' button has been clicked.
 */
void MyMainFrame::OnButtonLinear()
{
    TCanvas *ec = ecanvas->GetCanvas();

    if( IsProjectingMulti() ) {
        TObject *obj=0;
        TIter next(ec->GetListOfPrimitives());
        while ((obj = next())) {
            if (obj->InheritsFrom(TPad::Class())) {
                ((TPad*)obj)->SetLogy(0);
                ((TPad*)obj)->SetLogz(0);
            }
        }
    } else {
        ec->SetLogy(0);
        ec->SetLogz(0);
    }
    ec->Update();
}

// ########################################################################

/**
 * Called when the 'logscale' button has been clicked.
 */
void MyMainFrame::OnButtonLog()
{
    TCanvas *ec = ecanvas->GetCanvas();
    if( IsProjectingMulti() ) {
            TObject *obj=0;
            TIter next(ec->GetListOfPrimitives());
            while ((obj = next())) {
                if (obj->InheritsFrom(TPad::Class()))
                    ((TPad*)obj)->SetLogy(1);
            }
    } else if( IsProjecting() ) {
        ec->SetLogy(1);
        ec->SetLogz(0);
    } else {
        ec->SetLogy(0);
        ec->SetLogz(1);
    }
    ec->Update();
}

// ########################################################################

/**
 * Called when the spectrum selection has changed.
 */
void MyMainFrame::OnComboSpectraSelect(Int_t id)
{
    nasp = &sort_spectra[id];
    const int maxi = nasp->ydim-1;

    n_from->SetLimits( TGNumberFormat::kNELLimitMinMax, 0, maxi );
    n_to  ->SetLimits( TGNumberFormat::kNELLimitMinMax, 0, maxi );

    t_printprefix->SetText(nasp->name);

    std::string windowname = nasp->name;
    windowname += " -- ROOT update";
    SetWindowName(windowname.c_str());

    DeleteHistograms();
    UpdateHistogram();
}

// ########################################################################

void MyMainFrame::OnButtonProject()
{
    const bool proj = IsProjecting();

    n_from->SetState(proj);
    n_to  ->SetState(proj);

    r_sum   ->SetEnabled(proj);
    r_single->SetEnabled(proj);
    r_multi ->SetEnabled(proj);

    if( !proj )
        ecanvas->GetCanvas()->SetLogy(0);
        
    if( IsProjecting() && !(IsProjectingSum() || IsProjectingSingle() || IsProjectingMulti()) )
        r_sum->SetState(kButtonDown);

    //DeleteHistograms();
    UpdateHistogram();
}

// ########################################################################

void MyMainFrame::OnProjectFrom(Long_t)
{
    int from = n_from->GetIntNumber();
    int to   = n_to  ->GetIntNumber();
    if( to<from )
        n_to->SetNumber(from);

    //DeleteHistograms();
    UpdateHistogram();
}

// ########################################################################

void MyMainFrame::OnProjectTo(Long_t)
{
    int to   = n_to  ->GetIntNumber();
    int from = n_from->GetIntNumber();
    if( to<from )
        n_from->SetNumber(to);

    //DeleteHistograms();
    UpdateHistogram();
}

// ########################################################################

void MyMainFrame::OnProjectSum()
{
    r_single->SetState(kButtonUp);
    r_multi ->SetState(kButtonUp);

    //DeleteHistograms();
    UpdateHistogram();
}

// ########################################################################

void MyMainFrame::OnProjectSingle()
{
    r_sum  ->SetState(kButtonUp);
    r_multi->SetState(kButtonUp);

    //DeleteHistograms();
    UpdateHistogram();
}

// ########################################################################

void MyMainFrame::OnProjectMulti()
{
    r_sum   ->SetState(kButtonUp);
    r_single->SetState(kButtonUp);

    //DeleteHistograms();
    UpdateHistogram();
}

// ########################################################################

/**
 * Called for all events on the canvas, we update the status bar.
 */
void MyMainFrame::OnCanvasEvent(Int_t event, Int_t px, Int_t py, TObject* selected)
{
    if( !selected )
        return;

    TVirtualPad* savepad = gPad;
    gPad = ecanvas->GetCanvas()->GetSelectedPad();

    statusbar->SetText(selected->GetTitle(),0);
    statusbar->SetText(selected->GetName(), 1);
    char tmp[256];
    if (event == kKeyPress)
        snprintf(tmp, sizeof(tmp), "%c", (char) px);
    else
        snprintf(tmp, sizeof(tmp), "%d,%d", px, py);
    statusbar->SetText(tmp, 2);
    statusbar->SetText(selected->GetObjectInfo(px,py),3);
    gPad = savepad;
}

// ########################################################################

static int divy(int n)
{
    return (int)std::ceil(std::sqrt(n));
}

static int divx(int n)
{
    if( n==0 )
        return 0;
    int ndivy = divy(n);
    int ndivx = (int)ceil(n/ndivy);
    while(ndivx*ndivy < n)
        ndivx += 1;
    return ndivx;
}

// ########################################################################

void MyMainFrame::UpdateHistogram()
{
    TCanvas *ec = ecanvas->GetCanvas();

    if( !nasp ) {
        ec->Clear();
        return;
    }

    // take a copy of the shared memory spectrum to avoid messing it
    // up; XXX does not prevent, only makes less likely
    int* copy = new int[nasp->xdim * nasp->ydim];
    memcpy(copy, nasp->ptr, nasp->xdim * nasp->ydim * sizeof(int));

    const int fr = n_from->GetIntNumber();
    const int to = n_to  ->GetIntNumber();

    if( !IsProjecting() ) {
        // update matrix
        TH2* hm = GetHistogramMatrix();
        enum { sW, sW2, sWX, sWX2, sWY, sWY2, sWXY, sEND };
        double stats_matrix[sEND] = {0}, entries_matrix=0;
        for(int j=0; j<nasp->ydim; j++) {
            for(int i=0; i<nasp->xdim; i++) {
                int c = copy[i + j*nasp->xdim];
                hm->SetBinContent(i+1, j+1, c);
                if( c<0 )
                    c = -c;
                
                entries_matrix     += c;
                stats_matrix[sW]   += c;
                stats_matrix[sW2]  += c*c;
                stats_matrix[sWX]  += i;
                stats_matrix[sWX2] += i*i;
                stats_matrix[sWY]  += j;
                stats_matrix[sWY2] += j*j;
                stats_matrix[sWXY] += i*j;
            }
        }
        
        hm->SetEntries(entries_matrix);
        hm->PutStats(stats_matrix);

        ec->cd();
        hm->Draw("colz");
    } else if( IsProjectingSum() ) {
        // update sum histogram
        TH1* hs = GetHistogramSum();

        enum { sW, sW2, sWX, sWX2, sEND };
        double stats_sum[sEND] = {0}, entries_sum=0;
        
        for(int i=0; i<nasp->xdim; i++) {
            int c = 0;
            for(int j=fr; j<=to; j++)
                c += copy[i + j*nasp->xdim];
            hs->SetBinContent(i+1, c);
            if( c<0 )
                c = -c;
            
            entries_sum += c;
            stats_sum[sW]   += c;
            stats_sum[sW2]  += c*c;
            stats_sum[sWX]  += i;
            stats_sum[sWX2] += i*i;
        }
        
        hs->SetEntries(entries_sum);
        hs->PutStats(stats_sum);

        ec->cd();
        hs->Draw();
    } else {
        int padcount = 1;
        if( IsProjectingMulti() ) {
            const int nrows = to-fr+1;
            const int ndivy = divy(nrows), ndivx = divx(nrows);
            
            // count pads on canvas
            int npads = 0, pdivy = 0, pdivx = 0;
            TObject *obj=0;
            TIter next(ec->GetListOfPrimitives());
            while ((obj = next())) {
                if (obj->InheritsFrom(TPad::Class()))
                    npads += 1;
            }
            pdivy = divy(npads);
            pdivx = divx(npads);

            // only re-create pads if the pad number changed
            if( pdivy != ndivy || pdivx != ndivx ) {
                ec->Clear();
                ec->Divide(ndivx, ndivy);
            }
            padcount = ndivx*ndivy;
        }

        // update individual histograms
        int maxrow = fr;
        float maxy = -1;
        for(int j=fr; j<=to; j++) {
            TH1* h = GetHistogramRow(j);
        
            enum { sW, sW2, sWX, sWX2, sEND };
            double stats[sEND] = {0}, entries=0;
        
            for(int i=0; i<nasp->xdim; i++) {
                int c = 0;
                c += copy[i + j*nasp->xdim];
                h->SetBinContent(i+1, c);
                if( c>maxy ) {
                    maxrow = j;
                    maxy = c;
                }
                if( c<0 )
                    c = -c;
                
                entries += c;
                stats[sW]   += c;
                stats[sW2]  += c*c;
                stats[sWX]  += i;
                stats[sWX2] += i*i;
            }
            
            h->SetEntries(entries);
            h->PutStats(stats);
        }
        if( IsProjectingMulti() ) {
            for(int j=fr; j<=to; j++) {
                ec->cd(1+j-fr);
                GetHistogramRow(j)->Draw();
            }
        } else {
            GetHistogramRow(maxrow)->Draw();
            for(int j=fr; j<=to; j++) {
                if( j != maxrow )
                    GetHistogramRow(j)->Draw("same");
            }
        }
        if( IsProjectingMulti() ) {
            const int nrows = to-fr+1;
            for( int p=nrows+1; p<=padcount; ++p ) {
                if( ec->cd(p) )
                    gPad->Clear();
            }
        }
        ec->cd();
    }

    delete[] copy;
    ec->Update();
}

// ########################################################################

void MyMainFrame::SortDisconnected()
{
    delete lc_sort;
    lc_sort = 0;

    // if in auto-update mode, try reconnecting
    OnReconnectTimeout();
}

// ########################################################################

void MyMainFrame::SortHaveLine()
{
    const std::string l = lc_sort->get_line();
    bool do_update = false;
    if( l.substr(0, 8) == "101 bufs" ) {
        // limit the update rate, not more than once every 2s
        timeval t;
        gettimeofday(&t, 0);
        double time_now = t.tv_sec + 1e-6 * t.tv_usec;
        if( time_now > time_last+real_update_rate ) {
            time_last = time_now;
            do_update = true;
        }
    } else if( l.substr(0, 18) == "204 status_cleared" ) {
        do_update = true;
    } else if( l.substr(0, 14) == "207 status_cwd" ) {
        const std::string dirname = l.substr(15);
        if( chdir(dirname.c_str()) != 0 ) {
            std::cout << "Could not change dir to\n" << dirname;
        } else {
            ReadCommands();
        }
    }
    
    if( do_update ) {
        timeval t;
        gettimeofday(&t, 0);
        const double time_before = t.tv_sec + 1e-6 * t.tv_usec;

        UpdateHistogram();

        gettimeofday(&t, 0);
        const double time_after = t.tv_sec + 1e-6 * t.tv_usec;

        // if updates are very slow (e.g. at home), limit the rate
        if( time_after > time_before ) {
            double max_update_rate = std::min(120.0, 10*(time_after - time_before) - 0.05);
            real_update_rate = std::max( max_update_rate, update_rate-0.05 );
        } else {
            real_update_rate = update_rate-0.05;
        }
    }
}

// ########################################################################

void MyMainFrame::DeleteHistograms()
{
    // clear histogram drawing area
    ecanvas->GetCanvas()->Clear();

    delete h_matrix;
    h_matrix = 0;
    delete h_sum;
    h_sum = 0;

    // delete all histograms from memory
    for(unsigned int i=0; i<h_row.size(); ++i)
        delete h_row[i];

    h_row.clear();
    h_row.resize(nasp->ydim, 0);
}

// ########################################################################

bool MyMainFrame::IsProjecting()
{
    return (b_project->GetState()==kButtonDown);
}

// ########################################################################

bool MyMainFrame::IsProjectingSum()
{
    return (r_sum->GetState()==kButtonDown);
}

// ########################################################################

bool MyMainFrame::IsProjectingSingle()
{
    return (r_single->GetState()==kButtonDown);
}

// ########################################################################

bool MyMainFrame::IsProjectingMulti()
{
    return (r_multi->GetState()==kButtonDown);
}

// ########################################################################

TH2* MyMainFrame::GetHistogramMatrix()
{
    if( !h_matrix ) {
        h_matrix = new TH2I(nasp->name, nasp->description,
                            nasp->xdim, 0, nasp->xdim,
                            nasp->ydim, 0, nasp->ydim);
        h_matrix->SetContour(64);
    }
    return h_matrix;
}

// ########################################################################

TH1* MyMainFrame::GetHistogramSum()
{
    if( !h_sum ) {
        char tmp_name[128];
        snprintf(tmp_name, sizeof(tmp_name), "%s_sum", nasp->name);
        h_sum = new TH1I(tmp_name, nasp->description, nasp->xdim, 0, nasp->xdim);
    }
    return h_sum;
}

// ########################################################################

TH1* MyMainFrame::GetHistogramRow(int row)
{
    if( !h_row[row] ) {
        char tmp_name[128];
        snprintf(tmp_name, sizeof(tmp_name), "%s_%02d", nasp->name, row);
        TH1* h = new TH1I(tmp_name, nasp->description, nasp->xdim, 0, nasp->xdim);

        h->SetLineColor( 1 + (row%9) );
        if( (row/9) % 2 )
            h->SetLineWidth(2*h->GetLineWidth());
        h->SetLineStyle( (row/18)%10 );

        h_row[row] = h;
    }
    return h_row[row];
}


// ########################################################################
// ########################################################################
// ########################################################################

int main(int argc, char* argv[])
{
    TApplication theApp("ROOTupdate", &argc, argv);

    gStyle->SetPadGridX(kTRUE);
    gStyle->SetPadGridY(kTRUE);
    gStyle->SetPalette(1);
    gStyle->SetCanvasColor(10);
    gStyle->SetOptStat(1111111);
    gStyle->SetStatFormat("10.8g");
    gStyle->SetNdivisions(520,"y");

    // ------------------------------

    int maxupdate = 2;

    for( int opt=0; (opt = getopt(argc, argv, "u:")) != -1; ) {
        switch (opt) {
        case 'u':
            maxupdate = atoi(optarg);
            break;
        default: /* '?' */
            std::cerr << "Usage: " << argv[0] << "[-u update_secs]" << std::endl;
            exit(EXIT_FAILURE);
        }
    }

    // ------------------------------

    spectra_attach_all(true);
    new MyMainFrame(maxupdate);
    theApp.Run(true);
    spectra_detach_all();

    return 0;
}
