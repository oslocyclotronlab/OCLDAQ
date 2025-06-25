//
// Created by Vetle Wegner Ingeberg on 02/12/2024.
//

#include "CFDFilterWidget.h"
#include "xiainterface.h"

#include <QDoubleSpinBox>
#include <QSpinBox>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QFormLayout>
#include <QLabel>

#include "helpers.h"

CFDFilter::CFDFilter(XIAInterface *_interface, QWidget *parent)
        : QGroupBox{"CFD settings", parent}
        , interface( _interface )
        , delay( new QDoubleSpinBox(this) )
        , scale( new QSpinBox(this) )
        , threshold( new QSpinBox(this) )
        , resetDelay( new QSpinBox(this) )
{

    delay->setMinimumWidth(75);
    scale->setMinimumWidth(75);
    threshold->setMinimumWidth(75);
    resetDelay->setMinimumWidth(75);

    delay->setDecimals(3);
    delay->setSingleStep(0.001);

    param_map = {
            {delay, "CFDDelay"},
            {scale, "CFDScale"},
            {threshold, "CFDThresh"},
            {resetDelay, "ResetDelay"}
    };

    QHBoxLayout *layout = new QHBoxLayout;
    QVBoxLayout *unit_layout = new QVBoxLayout;
    QFormLayout *formLayout = new QFormLayout;

    formLayout->addRow(tr("Delay:"), delay);
    formLayout->addRow(tr("Scale:"), scale);
    formLayout->addRow(tr("Threshold:"), threshold);
    formLayout->addRow(tr("Reset delay:"), resetDelay);

    unit_layout->addWidget(new QLabel(tr("us"), this));
    unit_layout->addWidget(new QLabel(tr(""), this));
    unit_layout->addWidget(new QLabel(tr(""), this));
    unit_layout->addWidget(new QLabel(tr(""), this));

    layout->addLayout(formLayout);
    layout->addLayout(unit_layout);

    setLayout(layout);
}

void CFDFilter::UpdateLimits(const int &module, const int &channel)
{
    get_numeric_limits<double>(module, channel, delay, interface, param_map);
    get_numeric_limits<double>(module, channel, scale, interface, param_map);
    get_numeric_limits<long long>(module, channel, threshold, interface, param_map);
    get_numeric_limits<long long>(module, channel, resetDelay, interface, param_map);
}

void CFDFilter::UpdateView(const int &module, const int &channel)
{
    // Need to fetch the current settings for the module and populate the data.
    UpdateLimits(module, channel);
    set_widget_numeric_value<double>(module, channel, delay, interface, param_map);
    set_widget_numeric_value<double>(module, channel, scale, interface, param_map);
    set_widget_numeric_value<long long>(module, channel, threshold, interface, param_map);
    set_widget_numeric_value<long long>(module, channel, resetDelay, interface, param_map);
}

void CFDFilter::UpdateSettings(const int &module, const int &channel)
{
    write_channel_value(module, channel, interface, param_map[delay], delay->value());
    write_channel_value(module, channel, interface, param_map[scale], scale->value());
    write_channel_value(module, channel, interface, param_map[threshold], threshold->value());
    write_channel_value(module, channel, interface, param_map[resetDelay], resetDelay->value());
}