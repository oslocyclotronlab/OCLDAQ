//
// Created by Vetle Wegner Ingeberg on 02/12/2024.
//

#include "BaselineFilterWidget.h"
#include "xiainterface.h"

#include <QDoubleSpinBox>
#include <QSpinBox>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QFormLayout>
#include <QLabel>

#include "helpers.h"

BaselineFilter::BaselineFilter(XIAInterface *_interface, QWidget *parent)
        : QGroupBox{"Baseline settings", parent}
        , interface( _interface )
        , baselineOffset( new QDoubleSpinBox(this) )
        , baselinePercent( new QSpinBox(this) )
        , baselineAverage( new QSpinBox(this) )
        , baselineCut( new QSpinBox(this) )
{

    baselineOffset->setMinimumWidth(75);
    baselinePercent->setMinimumWidth(75);
    baselineAverage->setMinimumWidth(75);
    baselineCut->setMinimumWidth(75);

    param_map = {
            {baselineOffset, "VOFFSET"},
            {baselinePercent, "BASELINE_PERCENT"},
            {baselineAverage, "BASELINE_AVERAGE"},
            {baselineCut, "BLCUT"}
    };

    QHBoxLayout *layout = new QHBoxLayout;
    QVBoxLayout *unit_layout = new QVBoxLayout;
    QFormLayout *formLayout = new QFormLayout;

    formLayout->addRow(tr("Baseline offset:"), baselineOffset);
    formLayout->addRow(tr("Baseline percent:"), baselinePercent);
    formLayout->addRow(tr("Baseline average:"), baselineAverage);
    formLayout->addRow(tr("Baseline cut:"), baselineCut);

    unit_layout->addWidget(new QLabel(tr("V"), this));
    unit_layout->addWidget(new QLabel(tr("%"), this));
    unit_layout->addWidget(new QLabel(tr(""), this));
    unit_layout->addWidget(new QLabel(tr(""), this));

    layout->addLayout(formLayout);
    layout->addLayout(unit_layout);
    setLayout(layout);
}
#include <iostream>

void BaselineFilter::UpdateLimits(const int &module, const int &channel)
{
    get_numeric_limits<double>(module, channel, baselineOffset, interface, param_map);
    get_numeric_limits<double>(module, channel, baselinePercent, interface, param_map);
    get_numeric_limits<double>(module, channel, baselineAverage, interface, param_map);
    get_numeric_limits<double>(module, channel, baselineCut, interface, param_map);
}

void BaselineFilter::UpdateView(const int &module, const int &channel)
{
    // Need to fetch the current settings for the module and populate the data.
    UpdateLimits(module, channel);
    set_widget_numeric_value<double>(module, channel, baselineOffset, interface, param_map);
    set_widget_numeric_value<double>(module, channel, baselinePercent, interface, param_map);
    set_widget_numeric_value<double>(module, channel, baselineAverage, interface, param_map);
    set_widget_numeric_value<double>(module, channel, baselineCut, interface, param_map);
}

void BaselineFilter::UpdateSettings(const int &module, const int &channel)
{
    write_channel_value(module, channel, interface, param_map[baselineOffset], baselineOffset->value());
    write_channel_value(module, channel, interface, param_map[baselinePercent], baselinePercent->value());
    write_channel_value(module, channel, interface, param_map[baselineAverage], baselineAverage->value());
    write_channel_value(module, channel, interface, param_map[baselineCut], baselineCut->value());
}