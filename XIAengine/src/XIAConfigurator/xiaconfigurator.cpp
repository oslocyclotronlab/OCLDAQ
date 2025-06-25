#include "xiaconfigurator.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QSpinBox>
#include <QDoubleSpinBox>
#include <QGroupBox>
#include <QTabWidget>
#include <QLabel>
#include <QCheckBox>
#include <QLineEdit>
#include <QComboBox>
#include <QTableWidget>
#include <QHeaderView>
#include <QPushButton>
#include <QLoggingCategory>

#ifndef NUMBER_OF_CHANNELS
#define NUMBER_OF_CHANNELS 16
#endif // NUMBER_OF_CHANNELS

#include <iostream>
#include <bitset>

#include "TimingFilterTab.h"
#include "ChannelRegisterTab.h"
#include "ModuleRegisterTab.h"
#include "CopySettingsTab.h"

#include "enumerate.h"
#include "helpers.h"

QLoggingCategory logger("GUI");


Buttons::Buttons(QWidget *parent)
    : QWidget{parent}
    , all_chan( new QCheckBox("All chan/mod.", this) )
    , blcutAdjustBtn( new QPushButton("Measure Baseline cut", this) )
    , blAdjustBtn( new QPushButton("Measure baseline offset", this) )
    , writeBtn( new QPushButton("Write to modules", this) )
    , saveBtn( new QPushButton("Save current settings", this) )
    , saveAsBtn( new QPushButton("Save As current settings", this) )
{
    auto *layout = new QVBoxLayout;

    auto *measureLayout = new QHBoxLayout;
    measureLayout->addStretch();
    measureLayout->addWidget(all_chan);
    measureLayout->addWidget(blcutAdjustBtn);
    measureLayout->addWidget(blAdjustBtn);
    measureLayout->addStretch();
    layout->addLayout(measureLayout);

    auto *controlLayout = new QHBoxLayout;
    controlLayout->addStretch();
    controlLayout->addWidget(writeBtn);
    controlLayout->addWidget(saveBtn);
    controlLayout->addWidget(saveAsBtn);
    controlLayout->addStretch();
    layout->addLayout(controlLayout);

    setLayout(layout);

}


XIAConfigurator::XIAConfigurator(XIAInterface *_interface, QWidget *parent)
    : QDialog(parent)
    , interface( _interface )
    , module( new QSpinBox(this) )
    , channel( new QSpinBox((this)) )
    , module_revision( new QLineEdit(this) )
    , module_ADCbits( new QLineEdit(this) )
    , module_ADCmsps( new QLineEdit(this) )
    , module_serial( new QLineEdit(this) )
    , tabWidget( new QTabWidget((this)) )
    , timingFilterTab( new TimingFilterTab(interface, this) )
    , channelRegisterTab( new ChannelRegisterTab(interface, this) )
    , moduleRegisterTab( new ModuleRegisterTab(interface, this) )
    , copySettingsTab( new CopySettingsTab(interface, this) )
    , buttons( new Buttons(this) )
{

    if ( interface->GetNumModules() < 1 )
        throw std::runtime_error("There has to be at least one module.");

    connect(buttons->writeBtn, SIGNAL(clicked(bool)), this, SLOT(WriteButtonClick(bool)));
    connect(buttons->blcutAdjustBtn, SIGNAL(clicked(bool)), this, SLOT(MeasureBaselineCut(bool)));
    connect(buttons->blAdjustBtn, SIGNAL(clicked(bool)), this, SLOT(MeasureBaselineOffset(bool)));


    QVBoxLayout *mainLayout = new QVBoxLayout;
    UpdateView(0, 0);
    module->setMinimum(0);
    module->setMaximum(int(interface->GetNumModules())-1);
    connect(module, SIGNAL(valueChanged(int)), this, SLOT(module_change(int)));

    channel->setMinimum(0);
    channel->setMaximum(NUMBER_OF_CHANNELS-1);
    connect(channel, SIGNAL(valueChanged(int)), this, SLOT(channel_change(int)));

    module_revision->setMaxLength(1);
    module_revision->setMaximumWidth(20);
    module_revision->setDisabled(true);

    module_ADCbits->setMaxLength(3);
    module_ADCbits->setMaximumWidth(30);
    module_ADCbits->setDisabled(true);

    module_ADCmsps->setMaxLength(3);
    module_ADCmsps->setMaximumWidth(30);
    module_ADCmsps->setDisabled(true);

    module_serial->setMaxLength(10);
    module_serial->setMaximumWidth(70);
    module_serial->setDisabled(true);


    QHBoxLayout *topBar = new QHBoxLayout;
    topBar->addWidget(new QLabel(tr("Module:"), this));
    topBar->addWidget(module);
    topBar->addWidget(new QLabel(tr("Channel:"), this));
    topBar->addWidget(channel);
    topBar->addWidget(new QLabel(tr("Revision:"), this));
    topBar->addWidget(module_revision);
    topBar->addWidget(new QLabel(tr("ADC:"), this));
    topBar->addWidget(module_ADCbits);
    topBar->addWidget(new QLabel(tr("bits"), this));
    topBar->addWidget(module_ADCmsps);
    topBar->addWidget(new QLabel(tr("MSPS"), this));
    topBar->addWidget(new QLabel(tr("Serial:"), this));
    topBar->addWidget(module_serial);
    topBar->addStretch();

    tabWidget->addTab(timingFilterTab, tr("Filter/Timing"));
    tabWidget->addTab(channelRegisterTab, tr("Channel register"));
    tabWidget->addTab(moduleRegisterTab, tr("Module register"));
    tabWidget->addTab(copySettingsTab, tr("Copy settings"));

    mainLayout->addLayout(topBar);
    mainLayout->addWidget(tabWidget);
    mainLayout->addStretch();
    mainLayout->addWidget(buttons);
    setLayout(mainLayout);

    UpdateView(module->value(), channel->value());
}

void XIAConfigurator::UpdateView(const int &moduleID, const int &channelID)
{
    auto moduleInfo = interface->GetModuleInfo(moduleID);
    module_revision->setText(QString::number(moduleInfo.revision, 16).toUpper());
    module_ADCbits->setText(QString::number(moduleInfo.adc_bits, 10));
    module_ADCmsps->setText(QString::number(moduleInfo.adc_msps, 10));
    module_serial->setText(QString::number(moduleInfo.serial_number, 10));
    timingFilterTab->UpdateView(moduleID, channelID);
    channelRegisterTab->UpdateView(moduleID, channelID);
    moduleRegisterTab->UpdateView(moduleID, channelID);
}

void XIAConfigurator::WriteView(const int &moduleID, const int &channelID)
{
    timingFilterTab->UpdateSettings(moduleID, channelID);
    channelRegisterTab->UpdateSettings(moduleID, channelID);
    moduleRegisterTab->UpdateSettings(moduleID, channelID);
}

void XIAConfigurator::module_change(int)
{
    int moduleNumber = module->value();
    int channelNumber = channel->value();
    UpdateView(moduleNumber, channelNumber);
}

void XIAConfigurator::channel_change(int)
{
    int moduleNumber = module->value();
    int channelNumber = channel->value();
    UpdateView(moduleNumber, channelNumber);
}

void XIAConfigurator::WriteButtonClick(bool)
{
    int moduleNumber = module->value();
    int channelNumber = channel->value();
    WriteView(moduleNumber, channelNumber);
    UpdateView(moduleNumber, channelNumber);
}

void XIAConfigurator::SaveSettingsButtonClick(bool)
{
    interface->WriteSettings("settings.set");
}

void XIAConfigurator::MeasureBaselineCut(bool)
{
    int moduleNumber = module->value();
    int channelNumber = channel->value();
    auto cut = interface->MeasureBLCut(moduleNumber, channelNumber);
    qCDebug(logger) << "Measured baseline cut in module " << moduleNumber << ", channel " << channelNumber << ": " << cut;
    UpdateView(moduleNumber, channelNumber);
}

void XIAConfigurator::MeasureBaselineOffset(bool)
{
    int moduleNumber = module->value();
    int channelNumber = channel->value();
    interface->MeasureBaseline(moduleNumber);
    UpdateView(moduleNumber, channelNumber);
}