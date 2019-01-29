#include "mainwindow.h"
#include "ui_mainwindow.h"

#include "pixie16app_defs.h"
#include "pixie16app_export.h"

#include "Functions.h"
#include "run_command.h"

#include <iostream>
#include <cmath>
#include <stdio.h>
#include <unistd.h>

#include <QFileDialog>

extern command_list* commands;

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

    // Populate the 'copy' table.
    MakeCopyTable();

    // We set the current view to reflect the values set in the XIA module.
    UpdateView();

}

void MainWindow::MakeCopyTable()
{
    // We have rows = n_modules
    // We have columns = 16 (NUM_CHANNELS)
    char tmp[1024];
    ui->tableWidget->setColumnCount(NUMBER_OF_CHANNELS);
    ui->tableWidget->setRowCount(n_modules);

    for (int i = 0 ; i < NUMBER_OF_CHANNELS ; ++i){
        sprintf(tmp, "%d", i);
        ui->tableWidget->setHorizontalHeaderItem(i, new QTableWidgetItem(tr(tmp)));
    }

    for (int i = 0 ; i < n_modules ; ++i){
        sprintf(tmp, "%d", i);
        ui->tableWidget->setVerticalHeaderItem(i, new QTableWidgetItem(tr(tmp)));
    }

    for (int i = 0 ; i < n_modules ; ++i){
        for (int j = 0 ; j < NUMBER_OF_CHANNELS ; ++j){
            ui->tableWidget->setCellWidget(i, j, new QCheckBox());
        }
    }

    ui->tableWidget->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    ui->tableWidget->verticalHeader()->setSectionResizeMode(QHeaderView::Stretch);

    connect(ui->tableWidget->horizontalHeader(),
            &QHeaderView::sectionDoubleClicked,
            this,
            &MainWindow::horizontalHeaderSectionDoubleClicked);
    connect(ui->tableWidget->verticalHeader(),
            &QHeaderView::sectionDoubleClicked,
            this,
            &MainWindow::verticalHeaderSectionDoubleClicked);
}

void MainWindow::horizontalHeaderSectionDoubleClicked(int sec)
{
    // If all is checked, then we uncheck.
    int n_checked = 0;
    for (int i = 0 ; i < n_modules ; ++i){
        QCheckBox *box = (QCheckBox *)ui->tableWidget->cellWidget(i, sec);
        if (box->isChecked())
            ++n_checked;
    }
    for (int i = 0 ; i < n_modules ; ++i){
        QCheckBox *box = (QCheckBox *)ui->tableWidget->cellWidget(i, sec);
        if (n_checked == n_modules)
            box->setChecked(false);
        else
            box->setChecked(true);
    }
}

void MainWindow::verticalHeaderSectionDoubleClicked(int sec)
{
    // If all is checked, then we uncheck.
    int n_checked = 0;
    for (int i = 0 ; i < NUMBER_OF_CHANNELS ; ++i){
        QCheckBox *box = (QCheckBox *)ui->tableWidget->cellWidget(sec, i);
        if (box->isChecked())
            ++n_checked;
    }
    for (int i = 0 ; i < NUMBER_OF_CHANNELS ; ++i){
        QCheckBox *box = (QCheckBox *)ui->tableWidget->cellWidget(sec, i);
        if (n_checked == NUMBER_OF_CHANNELS)
            box->setChecked(false);
        else
            box->setChecked(true);
    }
}


// To be called once each time either slow range, fast range or module is changed.
void MainWindow::UpdateLimits()
{
    // First we need to get the module and its MHz
    unsigned short modRev=0xF, modADCBits=16, modADCMPS=500;
    unsigned int modSerNum=12;
#ifndef OFFLINE
    Pixie16ReadModuleInfo(current_module, &modRev, &modSerNum, &modADCBits, &modADCMPS);
#endif // OFFLINE

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
    else {
        adcFactor = msps;
    }

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

    // Trigger sample limits
    ui->peak_sample->setMinimum( -FASTFILTER_MAX_LEN * pow(2.0, current_slow_filter) / adcFactor );
    ui->peak_sample->setMaximum( SLOWFILTER_MAX_LEN * pow(2.0, current_slow_filter) / adcFactor );

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
    ui->baselineAverage->setMinimum(0);
    ui->baselineAverage->setMaximum(16);

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
    unsigned short module = current_module;
    unsigned short channel = current_channel;

    double trigRiseTime, trigFlatTop, peakSample;
    unsigned int trigThreshold;
    double energyRiseTime, energyFlatTop, tau;
    unsigned int baselinePercent, baselineCut, baselineAverage;
    double cfdDelay;
    unsigned int cfdScale, cfdThreshold;
    double trace_length, trace_delay;
    double fastTrigBackLen, extTrigStrech, ExternDelayLen, FtrigDelay, VetoStrech, ChanTrigStrech;
    unsigned int chanMultMaskL, chanMultMaskH;
    double QDCLen0, QDCLen1, QDCLen2, QDCLen3, QDCLen4, QDCLen5, QDCLen6, QDCLen7;
    unsigned int csra;

    double tmp;

    Pixie16ReadSglChanPar(const_cast<char *>("TRIGGER_RISETIME"), &trigRiseTime, module, channel);
    Pixie16ReadSglChanPar(const_cast<char *>("TRIGGER_FLATTOP"), &trigFlatTop, module, channel);
    Pixie16ReadSglChanPar(const_cast<char *>("PEAKSAMPLE"), &peakSample, module, channel);
    Pixie16ReadSglChanPar(const_cast<char *>("TRIGGER_THRESHOLD"), &tmp, module, channel);
    trigThreshold = tmp;
    Pixie16ReadSglChanPar(const_cast<char *>("ENERGY_RISETIME"), &energyRiseTime, module, channel);
    Pixie16ReadSglChanPar(const_cast<char *>("ENERGY_FLATTOP"), &energyFlatTop, module, channel);
    Pixie16ReadSglChanPar(const_cast<char *>("TAU"), &tau, module, channel);
    Pixie16ReadSglChanPar(const_cast<char *>("BASELINE_PERCENT"), &tmp, module, channel);
    baselinePercent = tmp;
    Pixie16ReadSglChanPar(const_cast<char *>("BLCUT"), &tmp, module, channel);
    baselineCut = tmp;
    Pixie16ReadSglChanPar(const_cast<char *>("BASELINE_AVERAGE"), &tmp, module, channel);
    baselineAverage = tmp;
    Pixie16ReadSglChanPar(const_cast<char *>("CFDDelay"), &cfdDelay, module, channel);
    Pixie16ReadSglChanPar(const_cast<char *>("CFDScale"), &tmp, module, channel);
    cfdScale = tmp;
    Pixie16ReadSglChanPar(const_cast<char *>("CFDThresh"), &tmp, module, channel);
    cfdThreshold = tmp;
    Pixie16ReadSglChanPar(const_cast<char *>("TRACE_LENGTH"), &trace_length, module, channel);
    Pixie16ReadSglChanPar(const_cast<char *>("TRACE_DELAY"), &trace_delay, module, channel);
    Pixie16ReadSglChanPar(const_cast<char *>("FASTTRIGBACKLEN"), &fastTrigBackLen, module, channel);
    Pixie16ReadSglChanPar(const_cast<char *>("ExtTrigStretch"), &extTrigStrech, module, channel);
    Pixie16ReadSglChanPar(const_cast<char *>("ExternDelayLen"), &ExternDelayLen, module, channel);
    Pixie16ReadSglChanPar(const_cast<char *>("FtrigoutDelay"), &FtrigDelay, module, channel);
    Pixie16ReadSglChanPar(const_cast<char *>("VetoStretch"), &VetoStrech, module, channel);
    Pixie16ReadSglChanPar(const_cast<char *>("ChanTrigStretch"), &ChanTrigStrech, module, channel);
    Pixie16ReadSglChanPar(const_cast<char *>("MultiplicityMaskL"), &tmp, module, channel);
    chanMultMaskL = tmp;
    Pixie16ReadSglChanPar(const_cast<char *>("MultiplicityMaskH"), &tmp, module, channel);
    chanMultMaskH = tmp;
    Pixie16ReadSglChanPar(const_cast<char *>("QDCLen0"), &QDCLen0, module, channel);
    Pixie16ReadSglChanPar(const_cast<char *>("QDCLen1"), &QDCLen1, module, channel);
    Pixie16ReadSglChanPar(const_cast<char *>("QDCLen2"), &QDCLen2, module, channel);
    Pixie16ReadSglChanPar(const_cast<char *>("QDCLen3"), &QDCLen3, module, channel);
    Pixie16ReadSglChanPar(const_cast<char *>("QDCLen4"), &QDCLen4, module, channel);
    Pixie16ReadSglChanPar(const_cast<char *>("QDCLen5"), &QDCLen5, module, channel);
    Pixie16ReadSglChanPar(const_cast<char *>("QDCLen6"), &QDCLen6, module, channel);
    Pixie16ReadSglChanPar(const_cast<char *>("QDCLen7"), &QDCLen7, module, channel);
    Pixie16ReadSglChanPar(const_cast<char *>("CHANNEL_CSRA"), &tmp, module, channel);
    csra = tmp;

    ui->trigRiseTime->setValue(trigRiseTime);
    ui->trigFlatTop->setValue(trigFlatTop);
    ui->peak_sample->setValue(peakSample);
    ui->trigThreshold->setValue(trigThreshold);
    ui->eRiseTime->setValue(energyRiseTime);
    ui->eFlatTop->setValue(energyFlatTop);
    ui->tau->setValue(tau);
    ui->baselinePercent->setValue(baselinePercent);
    ui->baselineCut->setValue(baselineCut);
    ui->baselineAverage->setValue(baselineAverage);
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

    Pixie16ReadSglModPar(const_cast<char *>("SLOW_FILTER_RANGE"), &current_slow_filter, current_module);
    Pixie16ReadSglModPar(const_cast<char *>("MODULE_CSRB"), &modcsrb, current_module);
    Pixie16ReadSglModPar(const_cast<char *>("TrigConfig0"), &trigConfig0, current_module);
    Pixie16ReadSglModPar(const_cast<char *>("TrigConfig1"), &trigConfig1, current_module);
    Pixie16ReadSglModPar(const_cast<char *>("TrigConfig2"), &trigConfig2, current_module);
    Pixie16ReadSglModPar(const_cast<char *>("TrigConfig3"), &trigConfig3, current_module);


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
    unsigned long tmp;
    unsigned int tmpI;
    std::cout << "Writing parameters to module "<< module << ", channel " << channel;

    // First we will read the CSRA value
    unsigned long csra, csra_old;
    Pixie16ReadSglChanPar(const_cast<char *>("CHANNEL_CSRA"), (double *)&csra, module, channel);
    csra_old = csra;
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
    if (csra != csra_old)
        Pixie16WriteSglChanPar(const_cast<char *>("CHANNEL_CSRA"), csra, module, channel);

    // Next we do the module CSRB values.
    unsigned int csrb, csrb_old;
    Pixie16ReadSglModPar(const_cast<char *>("MODULE_CSRB"), &csrb, current_module);
    csrb_old = csrb;
    csrb = ( ui->MODCSRB_0->isChecked() ) ? APP32_SetBit(MODCSRB_CPLDPULLUP, csrb) : APP32_ClrBit(MODCSRB_CPLDPULLUP, csrb);
    csrb = ( ui->MODCSRB_4->isChecked() ) ? APP32_SetBit(MODCSRB_DIRMOD, csrb) : APP32_ClrBit(MODCSRB_DIRMOD, csrb);
    csrb = ( ui->MODCSRB_6->isChecked() ) ? APP32_SetBit(MODCSRB_CHASSISMASTER, csrb) : APP32_ClrBit(MODCSRB_CHASSISMASTER, csrb);
    csrb = ( ui->MODCSRB_7->isChecked() ) ? APP32_SetBit(MODCSRB_GFTSEL, csrb) : APP32_ClrBit(MODCSRB_GFTSEL, csrb);
    csrb = ( ui->MODCSRB_8->isChecked() ) ? APP32_SetBit(MODCSRB_ETSEL, csrb) : APP32_ClrBit(MODCSRB_ETSEL, csrb);
    csrb = ( ui->MODCSRB_10->isChecked() ) ? APP32_SetBit(MODCSRB_INHIBITENA, csrb) : APP32_ClrBit(MODCSRB_INHIBITENA, csrb);
    csrb = ( ui->MODCSRB_11->isChecked() ) ? APP32_SetBit(MODCSRB_MULTCRATES, csrb) : APP32_ClrBit(MODCSRB_MULTCRATES, csrb);
    csrb = ( ui->MODCSRB_12->isChecked() ) ? APP32_SetBit(MODCSRB_SORTEVENTS, csrb) : APP32_ClrBit(MODCSRB_SORTEVENTS, csrb);
    csrb = ( ui->MODCSRB_13->isChecked() ) ? APP32_SetBit(MODCSRB_BKPLFASTTRIG, csrb) : APP32_ClrBit(MODCSRB_BKPLFASTTRIG, csrb);
    if (csrb != csrb_old)
    Pixie16WriteSglModPar(const_cast<char *>("MODULE_CSRB"), csrb, current_module);
    
    Pixie16ReadSglModPar(const_cast<char *>("SLOW_FILTER_RANGE"), &tmpI, current_module);
    if ( tmpI != ui->slowRange->value())
        Pixie16WriteSglModPar(const_cast<char *>("SLOW_FILTER_RANGE"), ui->slowRange->value(), current_module);


    unsigned int trigConfig0 = std::strtoul(ui->trigConfig0->text().toStdString().c_str(), 0, 16);
    unsigned int trigConfig1 = std::strtoul(ui->trigConfig1->text().toStdString().c_str(), 0, 16);
    unsigned int trigConfig2 = std::strtoul(ui->trigConfig2->text().toStdString().c_str(), 0, 16);
    unsigned int trigConfig3 = std::strtoul(ui->trigConfig3->text().toStdString().c_str(), 0, 16);

    Pixie16ReadSglModPar(const_cast<char *>("TrigConfig0"), &tmpI, current_module);
    if (trigConfig0 != tmpI)
        Pixie16WriteSglModPar(const_cast<char *>("TrigConfig0"), trigConfig0, current_module);
    
    Pixie16ReadSglModPar(const_cast<char *>("TrigConfig1"), &tmpI, current_module);
    if (trigConfig1 != tmpI)
        Pixie16WriteSglModPar(const_cast<char *>("TrigConfig1"), trigConfig1, current_module);
    
    Pixie16ReadSglModPar(const_cast<char *>("TrigConfig2"), &tmpI, current_module);
    if (trigConfig2 != tmpI)
        Pixie16WriteSglModPar(const_cast<char *>("TrigConfig2"), trigConfig2, current_module);
    
    Pixie16ReadSglModPar(const_cast<char *>("TrigConfig3"), &tmpI, current_module);
    if (trigConfig3 != tmpI)
        Pixie16WriteSglModPar(const_cast<char *>("TrigConfig3"), trigConfig3, current_module);

    double tmpD;
    Pixie16ReadSglChanPar(const_cast<char *>("TRIGGER_RISETIME"), &tmpD, module, channel);
    if (tmpD != ui->trigRiseTime->value())
        Pixie16WriteSglChanPar(const_cast<char *>("TRIGGER_RISETIME"), ui->trigRiseTime->value(), module, channel);
    
    Pixie16ReadSglChanPar(const_cast<char *>("TRIGGER_FLATTOP"), &tmpD, module, channel);
    if (tmpD != ui->trigFlatTop->value())
        Pixie16WriteSglChanPar(const_cast<char *>("TRIGGER_FLATTOP"), ui->trigFlatTop->value(), module, channel);

    Pixie16ReadSglChanPar(const_cast<char *>("TRIGGER_THRESHOLD"), (double *)&tmp, module, channel);
    if (tmp != ui->trigThreshold->value())
        Pixie16WriteSglChanPar(const_cast<char *>("TRIGGER_THRESHOLD"), ui->trigThreshold->value(), module, channel);

    Pixie16ReadSglChanPar(const_cast<char *>("ENERGY_RISETIME"), &tmpD, module, channel);
    if (tmpD != ui->eRiseTime->value()){
        Pixie16WriteSglChanPar(const_cast<char *>("ENERGY_RISETIME"), ui->eRiseTime->value(), module, channel);
        Pixie16WriteSglChanPar(const_cast<char *>("PEAKSAMPLE"), ui->peak_sample->value(), module, channel);
    }
    
    Pixie16ReadSglChanPar(const_cast<char *>("ENERGY_FLATTOP"), &tmpD, module, channel);
    if (tmpD != ui->eFlatTop->value()){
        Pixie16WriteSglChanPar(const_cast<char *>("ENERGY_FLATTOP"), ui->eFlatTop->value(), module, channel);
        Pixie16WriteSglChanPar(const_cast<char *>("PEAKSAMPLE"), ui->peak_sample->value(), module, channel);
    }

    Pixie16ReadSglChanPar(const_cast<char *>("PEAKSAMPLE"), &tmpD, module, channel);
    if (tmpD != ui->peak_sample->value()){
        Pixie16WriteSglChanPar(const_cast<char *>("PEAKSAMPLE"), ui->peak_sample->value(), module, channel);
    }

    Pixie16ReadSglChanPar(const_cast<char *>("TAU"), &tmpD, module, channel);
    if (tmpD != ui->tau->value())
        Pixie16WriteSglChanPar(const_cast<char *>("TAU"), ui->tau->value(), module, channel);

    Pixie16ReadSglChanPar(const_cast<char *>("CFDDelay"), &tmpD, module, channel);
    if (tmpD != ui->cfdDelay->value())
        Pixie16WriteSglChanPar(const_cast<char *>("CFDDelay"), ui->cfdDelay->value(), module, channel);
    
    Pixie16ReadSglChanPar(const_cast<char *>("CFDScale"), (double *)&tmp, module, channel);
    if (tmp != ui->cfdScale->value())
        Pixie16WriteSglChanPar(const_cast<char *>("CFDScale"), ui->cfdScale->value(), module, channel);

    Pixie16ReadSglChanPar(const_cast<char *>("CFDThresh"), (double *)&tmp, module, channel);
    if (tmp != ui->cfdThreshold->value())
        Pixie16WriteSglChanPar(const_cast<char *>("CFDThresh"), ui->cfdThreshold->value(), module, channel);

    Pixie16ReadSglChanPar(const_cast<char *>("TRACE_LENGTH"), &tmpD, module, channel);
    if (tmpD != ui->traceLength->value())
        Pixie16WriteSglChanPar(const_cast<char *>("TRACE_LENGTH"), ui->traceLength->value(), module, channel);
    
    Pixie16ReadSglChanPar(const_cast<char *>("TRACE_DELAY"), &tmpD, module, channel);
    if (tmpD != ui->traceDelay->value())
        Pixie16WriteSglChanPar(const_cast<char *>("TRACE_DELAY"), ui->traceDelay->value(), module, channel);

    Pixie16ReadSglChanPar(const_cast<char *>("FASTTRIGBACKLEN"), &tmpD, module, channel);
    if (tmpD != ui->fastTrigBackLen->value())
        Pixie16WriteSglChanPar(const_cast<char *>("FASTTRIGBACKLEN"), ui->fastTrigBackLen->value(), module, channel);

    Pixie16ReadSglChanPar(const_cast<char *>("ExtTrigStretch"), &tmpD, module, channel);
    if (tmpD != ui->extTrigStrech->value())
        Pixie16WriteSglChanPar(const_cast<char *>("ExtTrigStretch"), ui->extTrigStrech->value(), module, channel);

    Pixie16ReadSglChanPar(const_cast<char *>("ExternDelayLen"), &tmpD, module, channel);
    if (tmpD != ui->ExternDelayLen->value())
        Pixie16WriteSglChanPar(const_cast<char *>("ExternDelayLen"), ui->ExternDelayLen->value(), module, channel);
    
    Pixie16ReadSglChanPar(const_cast<char *>("FtrigoutDelay"), &tmpD, module, channel);
    if (tmpD != ui->FtrigoutDelay->value())
        Pixie16WriteSglChanPar(const_cast<char *>("FtrigoutDelay"), ui->FtrigoutDelay->value(), module, channel);

    Pixie16ReadSglChanPar(const_cast<char *>("VetoStretch"), &tmpD, module, channel);
    if (tmpD != ui->VetoStrech->value())
        Pixie16WriteSglChanPar(const_cast<char *>("VetoStretch"), ui->VetoStrech->value(), module, channel);

    Pixie16ReadSglChanPar(const_cast<char *>("ChanTrigStretch"), &tmpD, module, channel);
    if (tmpD != ui->ChanTrigStrech->value())
        Pixie16WriteSglChanPar(const_cast<char *>("ChanTrigStretch"), ui->ChanTrigStrech->value(), module, channel);

    Pixie16ReadSglChanPar(const_cast<char *>("QDCLen0"), &tmpD, module, channel);
    if (tmpD != ui->QDCLen0->value())
        Pixie16WriteSglChanPar(const_cast<char *>("QDCLen0"), ui->QDCLen0->value(), module, channel);

    Pixie16ReadSglChanPar(const_cast<char *>("QDCLen1"), &tmpD, module, channel);
    if (tmpD != ui->QDCLen1->value())
        Pixie16WriteSglChanPar(const_cast<char *>("QDCLen1"), ui->QDCLen1->value(), module, channel);
    
    Pixie16ReadSglChanPar(const_cast<char *>("QDCLen2"), &tmpD, module, channel);
    if (tmpD != ui->QDCLen2->value())
        Pixie16WriteSglChanPar(const_cast<char *>("QDCLen2"), ui->QDCLen2->value(), module, channel);
    
    Pixie16ReadSglChanPar(const_cast<char *>("QDCLen3"), &tmpD, module, channel);
    if (tmpD != ui->QDCLen3->value())
        Pixie16WriteSglChanPar(const_cast<char *>("QDCLen3"), ui->QDCLen3->value(), module, channel);

    Pixie16ReadSglChanPar(const_cast<char *>("QDCLen4"), &tmpD, module, channel);
    if (tmpD != ui->QDCLen4->value())
        Pixie16WriteSglChanPar(const_cast<char *>("QDCLen4"), ui->QDCLen4->value(), module, channel);

    Pixie16ReadSglChanPar(const_cast<char *>("QDCLen5"), &tmpD, module, channel);
    if (tmpD != ui->QDCLen5->value())
        Pixie16WriteSglChanPar(const_cast<char *>("QDCLen5"), ui->QDCLen5->value(), module, channel);

    Pixie16ReadSglChanPar(const_cast<char *>("QDCLen6"), &tmpD, module, channel);
    if (tmpD != ui->QDCLen6->value())
        Pixie16WriteSglChanPar(const_cast<char *>("QDCLen6"), ui->QDCLen6->value(), module, channel);

    Pixie16ReadSglChanPar(const_cast<char *>("QDCLen7"), &tmpD, module, channel);
    if (tmpD != ui->QDCLen7->value())
        Pixie16WriteSglChanPar(const_cast<char *>("QDCLen7"), ui->QDCLen7->value(), module, channel);

    Pixie16ReadSglChanPar(const_cast<char *>("BASELINE_PERCENT"), &tmpD, module, channel);
    if (tmpD != ui->baselinePercent->value())
        Pixie16WriteSglChanPar(const_cast<char *>("BASELINE_PERCENT"), ui->baselinePercent->value(), module, channel);

    Pixie16ReadSglChanPar(const_cast<char *>("BLCUT"), &tmpD, module, channel);
    if (tmpD != ui->baselineCut->value())
        Pixie16WriteSglChanPar(const_cast<char *>("BLCUT"), ui->baselineCut->value(), module, channel);

    Pixie16ReadSglChanPar(const_cast<char *>("BASELINE_AVERAGE"), &tmpD, module, channel);
    if (tmpD != ui->baselineAverage->value())
        Pixie16WriteSglChanPar(const_cast<char *>("BASELINE_AVERAGE"), ui->baselineAverage->value(), module, channel);

    unsigned long chanMultMaskL = std::strtoul(ui->multMaskL->text().toStdString().c_str(), 0, 16);
    unsigned long chanMultMaskH = std::strtoul(ui->multMaskH->text().toStdString().c_str(), 0, 16);

    Pixie16ReadSglChanPar(const_cast<char *>("MultiplicityMaskL"), (double *)&tmp, module, channel);
    if (tmp != chanMultMaskL)
        Pixie16WriteSglChanPar(const_cast<char *>("MultiplicityMaskL"), chanMultMaskL, module, channel);

    Pixie16ReadSglChanPar(const_cast<char *>("MultiplicityMaskH"), (double *)&tmp, module, channel);
    if (tmp != chanMultMaskH)
        Pixie16WriteSglChanPar(const_cast<char *>("MultiplicityMaskH"), chanMultMaskH, module, channel);

    std::cout << " ... Done! " << std::endl;
    SaveSettings(const_cast<char *>("settings.set"));
}

void MainWindow::SaveSettings(char *filename)
{
    // First we need to read the entire settings file as it currently is on disk (as a backup copy)
    FILE *file = fopen(filename, "rb");
    bool have_bck = true;
    unsigned int config_raw[N_DSP_PAR * PRESET_MAX_MODULES];
    if (fread(config_raw, sizeof(unsigned int), N_DSP_PAR*PRESET_MAX_MODULES, file) != N_DSP_PAR*PRESET_MAX_MODULES){
        std::cout << "Warning: settings.set is not valid. All current settings may be lost if saving .set file fails." << std::endl;
        have_bck = false;
    }

    fclose(file); // We are done with the file.

    std::cout << "Trying to save settings to file '" << filename << "'" << std::endl;

    int retval = Pixie16SaveDSPParametersToFile(filename);
    if (retval == 0){
        std::cout << "... Done" << std::endl;
        return;
    } else if (retval == -1) {
        std::cout << "... Failed, unable to read DSP parameter value from modules. Please restart engine and check the 'Pixie16msg.txt' file." << std::endl;
        if (!have_bck){
            std::cout << "Warning: Unable to restore old '" << filename << "' file" << std::endl;
            return;
        }
    } else {
        std::cout << "... Failed, unable to write to disk." << std::endl;
        if (!have_bck){
            std::cout << "Warning: Unable to restore old '" << filename << "' file" << std::endl;
            return;
        }
    }

    file = fopen(filename, "wb");

    // Writing to file...
    if (fwrite(config_raw, sizeof(unsigned int), N_DSP_PAR*PRESET_MAX_MODULES, file) != N_DSP_PAR*PRESET_MAX_MODULES){
        std::cout << "Warning: Unable to restore old '" << filename << "' file. This may cause an error when restarting engine." << std::endl;
    }

    fclose(file); // Done!
}

void MainWindow::on_SaveButton_clicked()
{
    SaveSettings(const_cast<char *>("settings.set"));

    std::vector<std::string> args;
    args.push_back("-f");
    args.push_back("settings.set");
    args.push_back(std::string("-a Subject=Settings"));
    args.push_back(std::string("Settings file for XIA modules\nPlease add comment here!"));



    std::cout << "Writing 'settings.set' to elog" << std::endl;
    UpdateView();
}

void MainWindow::on_SaveAsButton_clicked()
{
    char cwd[1024];
    QString directory;
    if (!getcwd(cwd, sizeof(cwd))){
        directory = "";
    } else {
        directory = cwd;
    }
    QString fname = QFileDialog::getSaveFileName(this,
                                                 tr("Save XIA settings"), directory,
                                                 tr("Settings file (*.set);;All files(*"));

    char tmp[16384];
    sprintf(tmp, "%s", fname.toStdString().c_str());

    SaveSettings(tmp);
}

void MainWindow::on_ClearButton_clicked()
{
    // If this button is clicked, we will reset all to false!

    // First the table:
    for (int i = 0 ; i < n_modules ; ++i){
        for (int j = 0 ; j < NUMBER_OF_CHANNELS ; ++j){
            QCheckBox *box = (QCheckBox *)ui->tableWidget->cellWidget(i, j);
            box->setChecked(false);
        }
    }

    // Then option boxes.
    ui->cpSet_0->setChecked(false);
    ui->cpSet_1->setChecked(false);
    ui->cpSet_2->setChecked(false);
    ui->cpSet_3->setChecked(false);
    ui->cpSet_4->setChecked(false);
    ui->cpSet_5->setChecked(false);
    ui->cpSet_7->setChecked(false);
    ui->cpSet_8->setChecked(false);
    ui->cpSet_9->setChecked(false);
    ui->cpSet_10->setChecked(false);
    ui->cpSet_11->setChecked(false);
    ui->cpSet_12->setChecked(false);
}

void MainWindow::on_CopyButton_clicked()
{
    // Destination mask
    unsigned short destinationMask[NUMBER_OF_CHANNELS * PRESET_MAX_MODULES];

    // Read from table if we should copy or not.
    for (int i = 0 ; i < n_modules ; ++i){
        for (int j = 0 ; j < NUMBER_OF_CHANNELS ; ++j){
            QCheckBox *box = (QCheckBox *)ui->tableWidget->cellWidget(i, j);
            destinationMask[i*NUMBER_OF_CHANNELS + j] = ( box->isChecked() ) ? 1 : 0;
        }
    }

    unsigned short cp_mask = 0;

    cp_mask = (ui->cpSet_0->isChecked() ) ? APP16_SetBit(0, cp_mask) : APP16_ClrBit(0, cp_mask);
    cp_mask = (ui->cpSet_1->isChecked() ) ? APP16_SetBit(1, cp_mask) : APP16_ClrBit(1, cp_mask);
    cp_mask = (ui->cpSet_2->isChecked() ) ? APP16_SetBit(2, cp_mask) : APP16_ClrBit(2, cp_mask);
    cp_mask = (ui->cpSet_3->isChecked() ) ? APP16_SetBit(3, cp_mask) : APP16_ClrBit(3, cp_mask);
    cp_mask = (ui->cpSet_4->isChecked() ) ? APP16_SetBit(4, cp_mask) : APP16_ClrBit(4, cp_mask);
    cp_mask = (ui->cpSet_5->isChecked() ) ? APP16_SetBit(5, cp_mask) : APP16_ClrBit(5, cp_mask);
    cp_mask = APP16_ClrBit(6, cp_mask); // We NEVER copy module settings!!
    cp_mask = (ui->cpSet_7->isChecked() ) ? APP16_SetBit(7, cp_mask) : APP16_ClrBit(7, cp_mask);
    cp_mask = (ui->cpSet_8->isChecked() ) ? APP16_SetBit(8, cp_mask) : APP16_ClrBit(8, cp_mask);
    cp_mask = (ui->cpSet_9->isChecked() ) ? APP16_SetBit(9, cp_mask) : APP16_ClrBit(9, cp_mask);
    cp_mask = (ui->cpSet_10->isChecked() ) ? APP16_SetBit(10, cp_mask) : APP16_ClrBit(10, cp_mask);
    cp_mask = (ui->cpSet_11->isChecked() ) ? APP16_SetBit(11, cp_mask) : APP16_ClrBit(11, cp_mask);
    cp_mask = (ui->cpSet_12->isChecked() ) ? APP16_SetBit(12, cp_mask) : APP16_ClrBit(12, cp_mask);

    // Now we call the Pixie16 API with our choise.
    int retval = Pixie16CopyDSPParameters(cp_mask, ui->source_module->value(), ui->source_channel->value(), destinationMask);
    if (retval == 0){
        std::cout << "Values successfully copied." << std::endl;
    } else if (retval == -1){
        std::cout << "Failed to program FiPPi, please restart engine." << std::endl;
    } else {
        std::cout << "Failed to set DACs, please restart engine." << std::endl;
    }

    // Clear...
    on_ClearButton_clicked();
}

void MainWindow::on_AdjBLineC_clicked()
{

    // Check if we should update all values.
    if ( ui->checkAll->isChecked() ){
        unsigned int BLcut[PRESET_MAX_MODULES][16];
        int retval;
        printf("\n\n");
        for (int i = 0 ; i < 16 ; ++i)
            printf("\t%d", i);
        for (int i = 0 ; i < n_modules ; ++i){
            printf("\n%d", i);
            for (int j = 0 ; j < 16 ; ++j){
                retval = Pixie16BLcutFinder(i, j, &BLcut[i][j]);
                if (retval < 0){
                    printf("*ERROR* Pixie16BLcutFinder for mod = %d, ch = %d failed, retval = %d\n", i, j, retval);
                    return;
                }
                printf("\t%d", BLcut[i][j]);
            }
        }
        printf("\n\n");
    } else {
        unsigned short module = current_module, channel = current_channel;
        unsigned int bl;
        int retval = Pixie16BLcutFinder(module, channel, &bl);
        if (retval < 0){
            printf("*ERROR* Pixie16BLcutFinder for mod = %d, ch = %d failed, retval = %d\n", module, channel, retval);
            return;
        }
        printf("Baseline mod=%d, ch=%d: %d\n", module, channel, bl);
    }

    // Update the current view!
    UpdateView();
}

void MainWindow::on_AdjBLine_clicked()
{
    // Adjust baselines!
    int retval = AdjustBaselineOffset(n_modules);
    if (retval < 0){
        printf("*ERROR* Pixie16AdjustOffsets failed, retval = %d\n", retval);
        return;
    }

    UpdateView();
}
