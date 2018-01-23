// -*- c++ -*-

#ifndef RUPDATE_H
#define RUPDATE_H 1

#include <TGFrame.h>
#include <vector>
#ifndef __CINT__
#include "run_command.h"
#endif

class TGCheckButton;
class TGComboBox;
class TGNumberEntry;
class TGRadioButton;
class TGStatusBar;
class TGTextButton;
class TGTextEntry;
class TH1;
class TH2;
class TRootEmbeddedCanvas;
class TTimer;

class io_root;
class line_channel;
struct sort_spectrum_t;

class MyMainFrame : public TGMainFrame {
public:
    MyMainFrame(int maxupdate);
    virtual ~MyMainFrame();

    void CloseWindow();

    void OnButtonAutoUpdate();
    void OnButtonElog();
    void OnButtonPrint();
    void OnButtonLinear();
    void OnButtonLog();
    void OnComboSpectraSelect(Int_t id);
    void OnButtonProject();
    void OnProjectFrom(Long_t);
    void OnProjectTo(Long_t);
    void OnProjectSum();
    void OnProjectSingle();
    void OnProjectMulti();
    void OnCanvasEvent(Int_t event, Int_t x, Int_t y, TObject* obj);
    void OnReconnectTimeout();
    
    void UpdateHistogram();

    void SortDisconnected();
    void SortHaveLine();

private:
    void DeleteHistograms();
    bool IsProjecting();
    bool IsProjectingSum();
    bool IsProjectingSingle();
    bool IsProjectingMulti();
    void ReadCommands();

    TH2* GetHistogramMatrix();
    TH1* GetHistogramSum();
    TH1* GetHistogramRow(int row);

    std::string SaveAsROOTAndPNG(std::string const& prefix);

    ClassDef(MyMainFrame,0)

private:
    TGTextButton  *b_autoupdate;
    TGComboBox    *cmb_spectra;
    TGCheckButton *b_project;
    TGNumberEntry *n_from, *n_to;
    TGRadioButton *r_sum, *r_single, *r_multi;
    TRootEmbeddedCanvas *ecanvas;
    TGStatusBar   *statusbar;
    TGTextEntry   *t_printprefix;
    TTimer *timer_reconnect;

    io_root       *ior;
    line_channel  *lc_sort;

    sort_spectrum_t* nasp;
    TH2 *h_matrix;
    TH1 *h_sum;
    std::vector<TH1*> h_row;

    double update_rate, real_update_rate;
    double time_last;

#ifndef __CINT__
    command_list commands;
#endif
};

#endif /* RUPDATE_H */

