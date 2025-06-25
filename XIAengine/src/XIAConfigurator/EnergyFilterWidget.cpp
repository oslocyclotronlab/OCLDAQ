//
// Created by Vetle Wegner Ingeberg on 02/12/2024.
//

#include "EnergyFilterWidget.h"
#include "xiainterface.h"

#include <QSpinBox>
#include <QDoubleSpinBox>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QFormLayout>
#include <QLabel>

#include "helpers.h"

EnergyFilter::EnergyFilter(XIAInterface *_interface, QWidget *parent)
        : QGroupBox{"Energy Filter", parent}
        , interface( _interface )
        , risetime( new QDoubleSpinBox(this) )
        , flattop( new QDoubleSpinBox(this) )
        , tau( new QDoubleSpinBox(this) )
{
    risetime->setToolTip(tr("Risetime of the energy filter. Should be longer than the risetime of the analog signal."));
    flattop->setToolTip(tr("tips"));
    tau->setToolTip(tr("Tips"));

    risetime->setMinimumWidth(75);
    flattop->setMinimumWidth(75);
    tau->setMinimumWidth(75);

    risetime->setDecimals(3);
    flattop->setDecimals(3);
    tau->setDecimals(3);

    risetime->setSingleStep(0.001);
    flattop->setSingleStep(0.001);
    tau->setSingleStep(0.001);

    param_map = {
            {risetime, "ENERGY_RISETIME"},
            {flattop, "ENERGY_FLATTOP"},
            {tau, "TAU"}
    };

    QHBoxLayout *layout = new QHBoxLayout;
    QVBoxLayout *unit_layout = new QVBoxLayout;
    QFormLayout *formLayout = new QFormLayout;


    formLayout->addRow(tr("Risetime:"), risetime);
    formLayout->addRow(tr("Flattop:"), flattop);
    formLayout->addRow(tr("Tau:"), tau);

    unit_layout->addWidget(new QLabel(tr("us"), this));
    unit_layout->addWidget(new QLabel(tr("us"), this));
    unit_layout->addWidget(new QLabel(tr("us"), this));

    layout->addLayout(formLayout);
    layout->addLayout(unit_layout);

    setLayout(layout);
}

void EnergyFilter::UpdateLimits(const int &module, const int &channel)
{
    get_numeric_limits<double>(module, channel, risetime, interface, param_map);
    get_numeric_limits<double>(module, channel, flattop, interface, param_map);
    get_numeric_limits<double>(module, channel, tau, interface, param_map);
}

void EnergyFilter::UpdateView(const int &module, const int &channel)
{
    // Need to fetch the current settings for the module and populate the data.
    UpdateLimits(module, channel);
    set_widget_numeric_value<double>(module, channel, risetime, interface, param_map);
    set_widget_numeric_value<double>(module, channel, flattop, interface, param_map);
    set_widget_numeric_value<double>(module, channel, tau, interface, param_map);
}

void EnergyFilter::UpdateSettings(const int &module, const int &channel)
{
    write_channel_value(module, channel, interface, param_map[risetime], risetime->value());
    write_channel_value(module, channel, interface, param_map[flattop], flattop->value());
    write_channel_value(module, channel, interface, param_map[tau], tau->value());
}