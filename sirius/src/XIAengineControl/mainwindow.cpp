#include "mainwindow.h"
#include "ui_mainwindow.h"

#include "pixie16app_defs.h"
#include "pixie16app_export.h"

#include <iostream>

const char *ch_par_names[] =
{
    "TRIGGER_RISETIME",
    "TRIGGER_FLATTOP",
    "TRIGGER_THRESHOLD",
    "ENERGY_RISETIME",
    "ENERGY_FLATTOP",
    "TAU",
    "BASELINE_PERCENT",
    "BLCUT",
    "CFDDelay",
    "CFDScale",
    "CFDThresh",
    "TRACE_LENGTH",
    "TRACE_DELAY",
    "FASTTRIGBACKLEN",
    "ExtTrigStretch",
    "ExternDelayLen",
    "FtrigoutDelay",
    "VetoStrech",
    "ChanTrigStrech",
    "MultiplicityMaskL",
    "MultiplicityMaskH",
    "QDCLen0",
    "QDCLen1",
    "QDCLen2",
    "QDCLen3",
    "QDCLen4",
    "QDCLen5",
    "QDCLen6",
    "QDCLen7"
};

MainWindow::MainWindow(int num_mod, QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow),
    n_modules( num_mod )
{
    ui->setupUi(this);

    // Set limits as given in Pixie16 SDK

    // Set module limits
    ui->current_mod->setMinimum(0);
    ui->current_mod->setValue(0);
    ui->current_mod->setMaximum(num_mod-1 /* call function to determine current max */);

    // Set channel limits
    ui->current_channel->setMinimum(0);
    ui->current_channel->setValue(0);
    ui->current_channel->setMaximum(NUMBER_OF_CHANNELS-1);

    // Set format of the line inputs.
    ui->multMaskL->setInputMask("HHHHHHHH");
    ui->multMaskH->setInputMask("HHHHHHHH");
    ui->trigConfig0->setInputMask("HHHHHHHH");
    ui->trigConfig1->setInputMask("HHHHHHHH");
    ui->trigConfig2->setInputMask("HHHHHHHH");
    ui->trigConfig3->setInputMask("HHHHHHHH");


    // We set the current view to reflect the values set in the XIA module.
    UpdateView();

}


// To be called once each time either slow range, fast range or module is changed.
void MainWindow::UpdateLimits()
{
    // First we need to get the module and its MHz
    unsigned short modRev=0xF, modADCBits=16, modADCMPS=500;
    unsigned int modSerNum=12;
    //Pixie16ReadModuleInfo(ui->current_mod->value(), &modRev, &modSerNum, &modADCBits, &modADCMPS);

    int fastfilterrange = 0;
    int rev = 0xF;//modRev;
    int msps = 500;//modADCMPS;
    double adcFactor;
    if ( msps == 100 )
        adcFactor = msps;
    else if ( msps == 250 )
        adcFactor = msps / 2;
    else if ( msps == 500 )
        adcFactor = msps / 5;

    // Update limits
    ui->slowRange->setMinimum(SLOWFILTERRANGE_MIN);
    ui->slowRange->setMaximum(SLOWFILTERRANGE_MAX);

    // Fast filter limits
    ui->trigRiseTime->setMinimum(0);
    ui->trigRiseTime->setMaximum( FASTFILTER_MAX_LEN * pow(2.0, fastfilterrange) / adcFactor );
    ui->trigFlatTop->setMinimum(0);
    ui->trigFlatTop->setMaximum( FASTFILTER_MAX_LEN * pow(2.0, fastfilterrange) / adcFactor );
    ui->trigThreshold->setMinimum(0);
    ui->trigThreshold->setMaximum(FAST_THRESHOLD_MAX);

    // Slow filter limits
    ui->eRiseTime->setMinimum(0);
    ui->eRiseTime->setMaximum( SLOWFILTER_MAX_LEN * pow(2.0, current_slow_filter) / adcFactor );
    ui->eFlatTop->setMinimum( MIN_SLOWGAP_LEN * pow(2.0, current_slow_filter) / adcFactor );
    ui->eFlatTop->setMaximum( SLOWFILTER_MAX_LEN * pow(2.0, current_slow_filter) / adcFactor );

    // Tau limits
    ui->tau->setMinimum(0);
    ui->tau->setMaximum(1e6);

    // CFD settings
    ui->cfdDelay->setMinimum( CFDDELAY_MIN / adcFactor );
    ui->cfdDelay->setMaximum( CFDDELAY_MAX / adcFactor );
    ui->cfdScale->setMinimum(0);
    ui->cfdScale->setMaximum( CFDSCALE_MAX );
    ui->cfdThreshold->setMinimum( CFDTHRESH_MIN );
    ui->cfdThreshold->setMaximum( CFDTHRESH_MAX );

    // Baseline settings
    ui->baselinePercent->setMinimum(1);
    ui->baselinePercent->setMaximum(99);
    ui->baselineCut->setMinimum(0);
    ui->baselineCut->setMaximum(999999999);

    // Trace length
    if ( msps == 500 ){
        ui->traceLength->setMinimum( TRACELEN_MIN_500MHZADC / ( msps * pow(2., current_slow_filter) ) );
        ui->traceLength->setMaximum( 999999999 );
        ui->traceDelay->setMinimum( 0 );
        ui->traceDelay->setMaximum( TRACEDELAY_MAX / ( (msps / 5 ) * pow(2.0, fastfilterrange) ) );
    } else if ( msps == 250 ) {
        ui->traceLength->setMinimum( TRACELEN_MIN_250OR100MHZADC / ( msps * pow(2., current_slow_filter) ) );
        ui->traceLength->setMaximum( 999999999 );
        ui->traceDelay->setMinimum( 0 );
        ui->traceDelay->setMaximum( TRACEDELAY_MAX / ( (msps / 2 ) * pow(2.0, fastfilterrange) ) );
    } else if ( msps == 100 ) {
        ui->traceLength->setMinimum( TRACELEN_MIN_250OR100MHZADC / ( msps * pow(2., current_slow_filter) ) );
        ui->traceLength->setMaximum( 999999999 );
        ui->traceDelay->setMinimum( 0 );
        ui->traceDelay->setMaximum( TRACEDELAY_MAX / ( msps  * pow(2.0, fastfilterrange) ) );
    }

    // Timing limits
    ui->fastTrigBackLen->setMinimum( FASTTRIGBACKLEN_MIN_100MHZFIPCLK / adcFactor );
    ui->fastTrigBackLen->setMaximum( FASTTRIGBACKLEN_MAX / adcFactor );
    ui->extTrigStrech->setMinimum( EXTTRIGSTRETCH_MIN / adcFactor );
    ui->extTrigStrech->setMaximum( EXTTRIGSTRETCH_MAX / adcFactor );

    ui->ExternDelayLen->setMinimum( EXTDELAYLEN_MIN / adcFactor );
    ui->FtrigoutDelay->setMinimum( FASTTRIGBACKDELAY_MIN / adcFactor );
    if ( rev == 0xB || rev == 0xC || rev == 0xD ) {
        ui->ExternDelayLen->setMaximum( EXTDELAYLEN_MAX_REVBCD / adcFactor );
        ui->FtrigoutDelay->setMaximum( FASTTRIGBACKDELAY_MAX_REVBCD / adcFactor );
    } else if ( rev == 0xF ) {
        ui->ExternDelayLen->setMaximum( EXTDELAYLEN_MAX_REVF / adcFactor );
        ui->FtrigoutDelay->setMaximum( FASTTRIGBACKDELAY_MAX_REVF / adcFactor );
    }

    ui->VetoStrech->setMinimum( VETOSTRETCH_MIN / adcFactor );
    ui->VetoStrech->setMaximum( VETOSTRETCH_MAX / adcFactor );
    ui->ChanTrigStrech->setMinimum( CHANTRIGSTRETCH_MIN / adcFactor );
    ui->ChanTrigStrech->setMaximum( CHANTRIGSTRETCH_MAX / adcFactor );

    // We do the QDCLen
    double QDCfactor = msps;
    if ( msps == 500 )
        QDCfactor = msps / 5;

    ui->QDCLen0->setMinimum( QDCLEN_MIN / QDCfactor );
    ui->QDCLen0->setMaximum( QDCLEN_MAX / QDCfactor );
    ui->QDCLen1->setMinimum( QDCLEN_MIN / QDCfactor );
    ui->QDCLen1->setMaximum( QDCLEN_MAX / QDCfactor );
    ui->QDCLen2->setMinimum( QDCLEN_MIN / QDCfactor );
    ui->QDCLen2->setMaximum( QDCLEN_MAX / QDCfactor );
    ui->QDCLen3->setMinimum( QDCLEN_MIN / QDCfactor );
    ui->QDCLen3->setMaximum( QDCLEN_MAX / QDCfactor );
    ui->QDCLen4->setMinimum( QDCLEN_MIN / QDCfactor );
    ui->QDCLen4->setMaximum( QDCLEN_MAX / QDCfactor );
    ui->QDCLen5->setMinimum( QDCLEN_MIN / QDCfactor );
    ui->QDCLen5->setMaximum( QDCLEN_MAX / QDCfactor );
    ui->QDCLen6->setMinimum( QDCLEN_MIN / QDCfactor );
    ui->QDCLen6->setMaximum( QDCLEN_MAX / QDCfactor );
    ui->QDCLen7->setMinimum( QDCLEN_MIN / QDCfactor );
    ui->QDCLen7->setMaximum( QDCLEN_MAX / QDCfactor );
}


void MainWindow::UpdateViewChannel()
{
    // We get the module & channel number from the fields.
    int module = current_module;
    int channel = current_channel;

    double trigRiseTime, trigFlatTop;
    unsigned long trigThreshold;
    double energyRiseTime, energyFlatTop, tau;
    unsigned long baselinePercent, baselineCut;
    double cfdDelay;
    unsigned long cfdScale, cfdThreshold;
    double trace_length, trace_delay;
    double fastTrigBackLen, extTrigStrech, ExternDelayLen, FtrigDelay, VetoStrech, ChanTrigStrech;
    unsigned long chanMultMaskL, chanMultMaskH;
    double QDCLen0, QDCLen1, QDCLen2, QDCLen3, QDCLen4, QDCLen5, QDCLen6, QDCLen7;
    unsigned long csra;

    double tmp;

    Pixie16ReadSglChanPar("TRIGGER_RISETIME", &trigRiseTime, module, channel);
    Pixie16ReadSglChanPar("TRIGGER_FLATTOP", &trigFlatTop, module, channel);
    Pixie16ReadSglChanPar("TRIGGER_THRESHOLD", &tmp, module, channel);
    trigThreshold = tmp;
    Pixie16ReadSglChanPar("ENERGY_RISETIME", &energyRiseTime, module, channel);
    Pixie16ReadSglChanPar("ENERGY_FLATTOP", &energyFlatTop, module, channel);
    Pixie16ReadSglChanPar("TAU", &tau, module, channel);
    Pixie16ReadSglChanPar("BASELINE_PERCENT", &tmp, module, channel);
    baselinePercent = tmp;
    Pixie16ReadSglChanPar("BLCUT", &tmp, module, channel);
    baselineCut = tmp;
    Pixie16ReadSglChanPar("CFDDelay", &cfdDelay, module, channel);
    Pixie16ReadSglChanPar("CFDScale", &tmp, module, channel);
    cfdScale = tmp;
    Pixie16ReadSglChanPar("CFDThresh", &tmp, module, channel);
    cfdThreshold = tmp;
    Pixie16ReadSglChanPar("TRACE_LENGTH", &trace_length, module, channel);
    Pixie16ReadSglChanPar("TRACE_DELAY", &trace_delay, module, channel);
    Pixie16ReadSglChanPar("FASTTRIGBACKLEN", &fastTrigBackLen, module, channel);
    Pixie16ReadSglChanPar("ExtTrigStretch", &extTrigStrech, module, channel);
    Pixie16ReadSglChanPar("ExternDelayLen", &ExternDelayLen, module, channel);
    Pixie16ReadSglChanPar("FtrigoutDelay", &FtrigDelay, module, channel);
    Pixie16ReadSglChanPar("VetoStretch", &VetoStrech, module, channel);
    Pixie16ReadSglChanPar("ChanTrigStretch", &ChanTrigStrech, module, channel);
    Pixie16ReadSglChanPar("MultiplicityMaskL", &tmp, module, channel);
    chanMultMaskL = tmp;
    Pixie16ReadSglChanPar("MultiplicityMaskH", &tmp, module, channel);
    chanMultMaskH = tmp;
    Pixie16ReadSglChanPar("QDCLen0", &QDCLen0, module, channel);
    Pixie16ReadSglChanPar("QDCLen1", &QDCLen1, module, channel);
    Pixie16ReadSglChanPar("QDCLen2", &QDCLen2, module, channel);
    Pixie16ReadSglChanPar("QDCLen3", &QDCLen3, module, channel);
    Pixie16ReadSglChanPar("QDCLen4", &QDCLen4, module, channel);
    Pixie16ReadSglChanPar("QDCLen5", &QDCLen5, module, channel);
    Pixie16ReadSglChanPar("QDCLen6", &QDCLen6, module, channel);
    Pixie16ReadSglChanPar("QDCLen7", &QDCLen7, module, channel);
    Pixie16ReadSglChanPar("CHANNEL_CSRA", &tmp, module, channel);
    csra = tmp;

    ui->trigRiseTime->setValue(trigRiseTime);
    ui->trigFlatTop->setValue(trigFlatTop);
    ui->trigThreshold->setValue(trigThreshold);
    ui->eRiseTime->setValue(energyRiseTime);
    ui->eFlatTop->setValue(energyFlatTop);
    ui->tau->setValue(tau);
    ui->baselinePercent->setValue(baselinePercent);
    ui->baselineCut->setValue(baselineCut);
    ui->cfdDelay->setValue(cfdDelay);
    ui->cfdScale->setValue(cfdScale);
    ui->cfdThreshold->setValue(cfdThreshold);
    ui->traceLength->setValue(trace_length);
    ui->traceDelay->setValue(trace_delay);
    ui->fastTrigBackLen->setValue(fastTrigBackLen);
    ui->extTrigStrech->setValue(extTrigStrech);
    ui->ExternDelayLen->setValue(ExternDelayLen);
    ui->FtrigoutDelay->setValue(FtrigDelay);
    ui->VetoStrech->setValue(VetoStrech);
    ui->ChanTrigStrech->setValue(ChanTrigStrech);
    ui->QDCLen0->setValue(QDCLen0);
    ui->QDCLen1->setValue(QDCLen1);
    ui->QDCLen2->setValue(QDCLen2);
    ui->QDCLen3->setValue(QDCLen3);
    ui->QDCLen4->setValue(QDCLen4);
    ui->QDCLen5->setValue(QDCLen5);
    ui->QDCLen6->setValue(QDCLen6);
    ui->QDCLen7->setValue(QDCLen7);

    // Register settings
    ui->CSRA_0->setChecked(APP32_TstBit(CCSRA_FTRIGSEL, csra));
    ui->CSRA_1->setChecked(APP32_TstBit(CCSRA_EXTTRIGSEL, csra));
    ui->CSRA_2->setChecked(APP32_TstBit(CCSRA_GOOD, csra));
    ui->CSRA_3->setChecked(APP32_TstBit(CCSRA_CHANTRIGSEL, csra));
    ui->CSRA_4->setChecked(APP32_TstBit(CCSRA_SYNCDATAACQ, csra));
    ui->CSRA_5->setChecked(APP32_TstBit(CCSRA_POLARITY, csra));
    ui->CSRA_6->setChecked(APP32_TstBit(CCSRA_VETOENA, csra));
    ui->CSRA_7->setChecked(APP32_TstBit(CCSRA_HISTOE, csra));
    ui->CSRA_8->setChecked(APP32_TstBit(CCSRA_TRACEENA, csra));
    ui->CSRA_9->setChecked(APP32_TstBit(CCSRA_QDCENA, csra));
    ui->CSRA_10->setChecked(APP32_TstBit(CCSRA_CFDMODE, csra));
    ui->CSRA_11->setChecked(APP32_TstBit(CCSRA_GLOBTRIG, csra));
    ui->CSRA_12->setChecked(APP32_TstBit(CCSRA_ESUMSENA, csra));
    ui->CSRA_13->setChecked(APP32_TstBit(CCSRA_CHANTRIG, csra));
    ui->CSRA_14->setChecked(APP32_TstBit(CCSRA_ENARELAY, csra));
    ui->CSRA_17->setChecked(APP32_TstBit(CCSRA_ENAENERGYCUT, csra));
    ui->CSRA_18->setChecked(APP32_TstBit(CCSRA_GROUPTRIGSEL, csra));
    ui->CSRA_19->setChecked(APP32_TstBit(CCSRA_CHANVETOSEL, csra));
    ui->CSRA_20->setChecked(APP32_TstBit(CCSRA_MODVETOSEL, csra));
    ui->CSRA_21->setChecked(APP32_TstBit(CCSRA_EXTTSENA, csra));

    // Done reading and updating the view!
    if ( !APP32_TstBit(CCSRA_PILEUPCTRL, csra) && !APP32_TstBit(CCSRA_INVERSEPILEUP, csra) )
        ui->pile_up_sel->setCurrentIndex(0);
    else if ( APP32_TstBit(CCSRA_PILEUPCTRL, csra) && !APP32_TstBit(CCSRA_INVERSEPILEUP, csra) )
        ui->pile_up_sel->setCurrentIndex(1);
    else if ( !APP32_TstBit(CCSRA_PILEUPCTRL, csra) && APP32_TstBit(CCSRA_INVERSEPILEUP, csra) )
        ui->pile_up_sel->setCurrentIndex(2);
    else
        ui->pile_up_sel->setCurrentIndex(3);

    char tmp_txt[256];
    sprintf(tmp_txt, "%08X", chanMultMaskL);
    ui->multMaskL->setText(tmp_txt);
    sprintf(tmp_txt, "%08X", chanMultMaskH);
    ui->multMaskH->setText(tmp_txt);

}

void MainWindow::UpdateViewModule()
{
    unsigned int modcsrb;
    unsigned int trigConfig0, trigConfig1, trigConfig2, trigConfig3;

    Pixie16ReadSglModPar("SLOW_FILTER_RANGE", &current_slow_filter, current_module);
    Pixie16ReadSglModPar("MODULE_CSRB", &modcsrb, current_module);
    Pixie16ReadSglModPar("TrigConfig0", &trigConfig0, current_module);
    Pixie16ReadSglModPar("TrigConfig1", &trigConfig1, current_module);
    Pixie16ReadSglModPar("TrigConfig2", &trigConfig2, current_module);
    Pixie16ReadSglModPar("TrigConfig3", &trigConfig3, current_module);


    ui->slowRange->setValue(current_slow_filter);

    ui->MODCSRB_0->setChecked(APP32_TstBit(MODCSRB_CPLDPULLUP, modcsrb));
    ui->MODCSRB_4->setChecked(APP32_TstBit(MODCSRB_DIRMOD, modcsrb));
    ui->MODCSRB_6->setChecked(APP32_TstBit(MODCSRB_CHASSISMASTER, modcsrb));
    ui->MODCSRB_7->setChecked(APP32_TstBit(MODCSRB_GFTSEL, modcsrb));
    ui->MODCSRB_8->setChecked(APP32_TstBit(MODCSRB_ETSEL, modcsrb));
    ui->MODCSRB_10->setChecked(APP32_TstBit(MODCSRB_INHIBITENA, modcsrb));
    ui->MODCSRB_11->setChecked(APP32_TstBit(MODCSRB_MULTCRATES, modcsrb));
    ui->MODCSRB_12->setChecked(APP32_TstBit(MODCSRB_SORTEVENTS, modcsrb));
    ui->MODCSRB_13->setChecked(APP32_TstBit(MODCSRB_BKPLFASTTRIG, modcsrb));

    // Update trigger view
    char tmp[256];
    sprintf(tmp, "%08X", trigConfig0);
    ui->trigConfig0->setText(tmp);
    sprintf(tmp, "%08X", trigConfig1);
    ui->trigConfig1->setText(tmp);
    sprintf(tmp, "%08X", trigConfig2);
    ui->trigConfig2->setText(tmp);
    sprintf(tmp, "%08X", trigConfig3);
    ui->trigConfig3->setText(tmp);

}


void MainWindow::UpdateView()
{
    // First we check that we have the correct limits
    UpdateLimits();


    // We set the current view to reflect the values set in the XIA module.
    UpdateViewChannel();

    // Then we update the module settings tab.
    UpdateViewModule();
}



MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_current_mod_valueChanged(int arg1)
{
    current_module = arg1;

    UpdateView();
}


void MainWindow::on_current_channel_valueChanged(int arg1)
{

    current_channel = arg1;

    UpdateView();
}

void MainWindow::on_WriteButton_clicked()
{

    unsigned short module = current_module, channel = current_channel;
    std::cout << "Writing parameters to module "<< module << ", channel " << channel;

    // First we will read the CSRA value
    unsigned long csra;
    Pixie16ReadSglChanPar("CHANNEL_CSRA", (double *)&csra, module, channel);
    csra = ( ui->CSRA_0->isChecked() ) ? APP32_SetBit(CCSRA_FTRIGSEL, csra) : APP32_ClrBit(CCSRA_FTRIGSEL, csra);
    csra = ( ui->CSRA_1->isChecked() ) ? APP32_SetBit(CCSRA_EXTTRIGSEL, csra) : APP32_ClrBit(CCSRA_EXTTRIGSEL, csra);
    csra = ( ui->CSRA_2->isChecked() ) ? APP32_SetBit(CCSRA_GOOD, csra) : APP32_ClrBit(CCSRA_GOOD, csra);
    csra = ( ui->CSRA_3->isChecked() ) ? APP32_SetBit(CCSRA_CHANTRIGSEL, csra) : APP32_ClrBit(CCSRA_CHANTRIGSEL, csra);
    csra = ( ui->CSRA_4->isChecked() ) ? APP32_SetBit(CCSRA_SYNCDATAACQ, csra) : APP32_ClrBit(CCSRA_SYNCDATAACQ, csra);
    csra = ( ui->CSRA_5->isChecked() ) ? APP32_SetBit(CCSRA_POLARITY, csra) : APP32_ClrBit(CCSRA_POLARITY, csra);
    csra = ( ui->CSRA_6->isChecked() ) ? APP32_SetBit(CCSRA_VETOENA, csra) : APP32_ClrBit(CCSRA_VETOENA, csra);
    csra = ( ui->CSRA_7->isChecked() ) ? APP32_SetBit(CCSRA_HISTOE, csra) : APP32_ClrBit(CCSRA_HISTOE, csra);
    csra = ( ui->CSRA_8->isChecked() ) ? APP32_SetBit(CCSRA_TRACEENA, csra) : APP32_ClrBit(CCSRA_TRACEENA, csra);
    csra = ( ui->CSRA_9->isChecked() ) ? APP32_SetBit(CCSRA_QDCENA, csra) : APP32_ClrBit(CCSRA_QDCENA, csra);
    csra = ( ui->CSRA_10->isChecked() ) ? APP32_SetBit(CCSRA_CFDMODE, csra) : APP32_ClrBit(CCSRA_CFDMODE, csra);
    csra = ( ui->CSRA_11->isChecked() ) ? APP32_SetBit(CCSRA_GLOBTRIG, csra) : APP32_ClrBit(CCSRA_GLOBTRIG, csra);
    csra = ( ui->CSRA_12->isChecked() ) ? APP32_SetBit(CCSRA_ESUMSENA, csra) : APP32_ClrBit(CCSRA_ESUMSENA, csra);
    csra = ( ui->CSRA_13->isChecked() ) ? APP32_SetBit(CCSRA_CHANTRIG, csra) : APP32_ClrBit(CCSRA_CHANTRIG, csra);
    csra = ( ui->CSRA_14->isChecked() ) ? APP32_SetBit(CCSRA_ENARELAY, csra) : APP32_ClrBit(CCSRA_ENARELAY, csra);
    csra = ( ui->CSRA_17->isChecked() ) ? APP32_SetBit(CCSRA_ENAENERGYCUT, csra) : APP32_ClrBit(CCSRA_ENAENERGYCUT, csra);
    csra = ( ui->CSRA_18->isChecked() ) ? APP32_SetBit(CCSRA_GROUPTRIGSEL, csra) : APP32_ClrBit(CCSRA_GROUPTRIGSEL, csra);
    csra = ( ui->CSRA_19->isChecked() ) ? APP32_SetBit(CCSRA_CHANVETOSEL, csra) : APP32_ClrBit(CCSRA_CHANVETOSEL, csra);
    csra = ( ui->CSRA_20->isChecked() ) ? APP32_SetBit(CCSRA_MODVETOSEL, csra) : APP32_ClrBit(CCSRA_MODVETOSEL, csra);
    csra = ( ui->CSRA_21->isChecked() ) ? APP32_SetBit(CCSRA_EXTTSENA, csra) : APP32_ClrBit(CCSRA_EXTTSENA, csra);
    Pixie16WriteSglChanPar("CHANNEL_CSRA", csra, module, channel);

    // Next we do the module CSRB values.
    unsigned int csrb;
    Pixie16ReadSglModPar("MODULE_CSRB", &csrb, current_module);
    csrb = ( ui->MODCSRB_0->isChecked() ) ? APP32_SetBit(MODCSRB_CPLDPULLUP, csrb) : APP32_ClrBit(MODCSRB_CPLDPULLUP, csrb);
    csrb = ( ui->MODCSRB_4->isChecked() ) ? APP32_SetBit(MODCSRB_DIRMOD, csrb) : APP32_ClrBit(MODCSRB_DIRMOD, csrb);
    csrb = ( ui->MODCSRB_6->isChecked() ) ? APP32_SetBit(MODCSRB_CHASSISMASTER, csrb) : APP32_ClrBit(MODCSRB_CHASSISMASTER, csrb);
    csrb = ( ui->MODCSRB_7->isChecked() ) ? APP32_SetBit(MODCSRB_GFTSEL, csrb) : APP32_ClrBit(MODCSRB_GFTSEL, csrb);
    csrb = ( ui->MODCSRB_8->isChecked() ) ? APP32_SetBit(MODCSRB_ETSEL, csrb) : APP32_ClrBit(MODCSRB_ETSEL, csrb);
    csrb = ( ui->MODCSRB_10->isChecked() ) ? APP32_SetBit(MODCSRB_INHIBITENA, csrb) : APP32_ClrBit(MODCSRB_INHIBITENA, csrb);
    csrb = ( ui->MODCSRB_11->isChecked() ) ? APP32_SetBit(MODCSRB_MULTCRATES, csrb) : APP32_ClrBit(MODCSRB_MULTCRATES, csrb);
    csrb = ( ui->MODCSRB_12->isChecked() ) ? APP32_SetBit(MODCSRB_SORTEVENTS, csrb) : APP32_ClrBit(MODCSRB_SORTEVENTS, csrb);
    csrb = ( ui->MODCSRB_13->isChecked() ) ? APP32_SetBit(MODCSRB_BKPLFASTTRIG, csrb) : APP32_ClrBit(MODCSRB_BKPLFASTTRIG, csrb);

    // First write the CSRB register
    Pixie16WriteSglModPar("MODULE_CSRB", csrb, current_module);
    Pixie16WriteSglModPar("SLOW_FILTER_RANGE", ui->slowRange->value(), current_module);
    unsigned int trigConfig0 = std::strtoul(ui->trigConfig0->text().toStdString().c_str(), 0, 16);
    unsigned int trigConfig1 = std::strtoul(ui->trigConfig1->text().toStdString().c_str(), 0, 16);
    unsigned int trigConfig2 = std::strtoul(ui->trigConfig2->text().toStdString().c_str(), 0, 16);
    unsigned int trigConfig3 = std::strtoul(ui->trigConfig3->text().toStdString().c_str(), 0, 16);

    Pixie16WriteSglModPar("TrigConfig0", trigConfig0, current_module);
    Pixie16WriteSglModPar("TrigConfig1", trigConfig1, current_module);
    Pixie16WriteSglModPar("TrigConfig2", trigConfig2, current_module);
    Pixie16WriteSglModPar("TrigConfig3", trigConfig3, current_module);

    Pixie16WriteSglChanPar("TRIGGER_RISETIME", ui->trigRiseTime->value(), module, channel);
    Pixie16WriteSglChanPar("TRIGGER_FLATTOP", ui->trigFlatTop->value(), module, channel);
    Pixie16WriteSglChanPar("TRIGGER_THRESHOLD", ui->trigThreshold->value(), module, channel);
    Pixie16WriteSglChanPar("ENERGY_RISETIME", ui->eRiseTime->value(), module, channel);
    Pixie16WriteSglChanPar("ENERGY_FLATTOP", ui->eFlatTop->value(), module, channel);
    Pixie16WriteSglChanPar("TAU", ui->tau->value(), module, channel);
    Pixie16WriteSglChanPar("BASELINE_PERCENT", ui->baselinePercent->value(), module, channel);
    Pixie16WriteSglChanPar("BLCUT", ui->baselinePercent->value(), module, channel);
    Pixie16WriteSglChanPar("CFDDelay", ui->cfdDelay->value(), module, channel);
    Pixie16WriteSglChanPar("CFDScale", ui->cfdScale->value(), module, channel);
    Pixie16WriteSglChanPar("CFDThresh", ui->cfdThreshold->value(), module, channel);
    Pixie16WriteSglChanPar("TRACE_LENGTH", ui->traceLength->value(), module, channel);
    Pixie16WriteSglChanPar("TRACE_DELAY", ui->traceDelay->value(), module, channel);
    Pixie16WriteSglChanPar("FASTTRIGBACKLEN", ui->fastTrigBackLen->value(), module, channel);
    Pixie16WriteSglChanPar("ExtTrigStretch", ui->extTrigStrech->value(), module, channel);
    Pixie16WriteSglChanPar("ExternDelayLen", ui->ExternDelayLen->value(), module, channel);
    Pixie16WriteSglChanPar("FtrigoutDelay", ui->FtrigoutDelay->value(), module, channel);
    Pixie16WriteSglChanPar("VetoStretch", ui->VetoStrech->value(), module, channel);
    Pixie16WriteSglChanPar("ChanTrigStretch", ui->ChanTrigStrech->value(), module, channel);
    Pixie16WriteSglChanPar("QDCLen0", ui->QDCLen0->value(), module, channel);
    Pixie16WriteSglChanPar("QDCLen1", ui->QDCLen1->value(), module, channel);
    Pixie16WriteSglChanPar("QDCLen2", ui->QDCLen2->value(), module, channel);
    Pixie16WriteSglChanPar("QDCLen3", ui->QDCLen3->value(), module, channel);
    Pixie16WriteSglChanPar("QDCLen4", ui->QDCLen4->value(), module, channel);
    Pixie16WriteSglChanPar("QDCLen5", ui->QDCLen5->value(), module, channel);
    Pixie16WriteSglChanPar("QDCLen6", ui->QDCLen6->value(), module, channel);
    Pixie16WriteSglChanPar("QDCLen7", ui->QDCLen7->value(), module, channel);

    unsigned long chanMultMaskL = std::strtoul(ui->multMaskL->text().toStdString().c_str(), 0, 16);
    unsigned long chanMultMaskH = std::strtoul(ui->multMaskL->text().toStdString().c_str(), 0, 16);
    Pixie16WriteSglChanPar("MultiplicityMaskL", chanMultMaskL, module, channel);
    Pixie16WriteSglChanPar("MultiplicityMaskH", chanMultMaskH, module, channel);

    std::cout << " ... Done! " << std::endl;
}

void MainWindow::on_SaveButton_clicked()
{
    std::cout << "Saved settings to file 'settings.set'" << std::endl;
    Pixie16SaveDSPParametersToFile("settings.set");
}
