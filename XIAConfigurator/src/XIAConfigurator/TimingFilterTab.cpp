//
// Created by Vetle Wegner Ingeberg on 02/12/2024.
//

#include "TimingFilterTab.h"
#include "xiainterface.h"

#include "TriggerFilterWidget.h"
#include "EnergyFilterWidget.h"
#include "CFDFilterWidget.h"
#include "BaselineFilterWidget.h"
#include "TimingSettingsWidget.h"
#include "QDCFilterWidget.h"

#include <QSpinBox>
#include <QDoubleSpinBox>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QFormLayout>
#include <QLabel>

#include "helpers.h"

TimingFilterTab::TimingFilterTab(XIAInterface *_interface, QWidget *parent)
        : QWidget{parent}
        , triggerFilter( new TriggerFilter(_interface, this) )
        , energyFilter( new EnergyFilter(_interface, this) )
        , cfdFilter( new CFDFilter(_interface, this) )
        , baselineFilter( new BaselineFilter(_interface, this) )
        , timingSettings( new TimingSettings(_interface, this) )
        , qdcFilter( new QDCFilter(_interface, this) )
{

    QVBoxLayout *layout = new QVBoxLayout;
    QHBoxLayout *upperLayout = new QHBoxLayout;
    QVBoxLayout *upperLHSlayout = new QVBoxLayout;
    QVBoxLayout *upperRHSlayout = new QVBoxLayout;
    upperLHSlayout->addWidget(triggerFilter);
    upperLHSlayout->addWidget(energyFilter);
    upperLHSlayout->addWidget(cfdFilter);
    upperLHSlayout->addWidget(baselineFilter);
    upperLHSlayout->addStretch();
    upperRHSlayout->addWidget(timingSettings);
    upperRHSlayout->addWidget(qdcFilter);
    upperRHSlayout->addStretch();
    upperLayout->addStretch();
    upperLayout->addLayout(upperLHSlayout);
    upperLayout->addLayout(upperRHSlayout);
    upperLayout->addStretch();
    layout->addLayout(upperLayout);
    setLayout(layout);
}

void TimingFilterTab::UpdateView(const int &module, const int &channel)
{
    triggerFilter->UpdateView(module, channel);
    energyFilter->UpdateView(module, channel);
    cfdFilter->UpdateView(module, channel);
    baselineFilter->UpdateView(module, channel);
    timingSettings->UpdateView(module, channel);
    qdcFilter->UpdateView(module, channel);
}

void TimingFilterTab::UpdateSettings(const int &module, const int &channel)
{
    triggerFilter->UpdateSettings(module, channel);
    energyFilter->UpdateSettings(module, channel);
    cfdFilter->UpdateSettings(module, channel);
    baselineFilter->UpdateSettings(module, channel);
    timingSettings->UpdateSettings(module, channel);
    qdcFilter->UpdateSettings(module, channel);
}