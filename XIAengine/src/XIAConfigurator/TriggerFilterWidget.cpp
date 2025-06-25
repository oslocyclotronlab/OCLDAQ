//
// Created by Vetle Wegner Ingeberg on 02/12/2024.
//

#include "TriggerFilterWidget.h"
#include "xiainterface.h"

#include <QSpinBox>
#include <QDoubleSpinBox>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QFormLayout>
#include <QLabel>

#include "helpers.h"

TriggerFilter::TriggerFilter(XIAInterface *_interface, QWidget *parent)
        : QGroupBox{"Trigger Filter", parent}
        , interface( _interface )
        , risetime( new QDoubleSpinBox(this) )
        , flattop( new QDoubleSpinBox(this) )
        , threshold( new QSpinBox(this) )
{
    risetime->setToolTip(tr("Risetime of the trigger filter."));
    risetime->setMinimumWidth(75);
    risetime->setDecimals(3);
    risetime->setSingleStep(0.001);
    flattop->setToolTip(tr("Flat-top of the trigger filter"));
    flattop->setMinimumWidth(75);
    flattop->setDecimals(3);
    flattop->setSingleStep(0.001);
    threshold->setToolTip(tr("Trigger threshold"));
    threshold->setMinimumWidth(75);

    param_map = {
            {risetime, "TRIGGER_RISETIME"},
            {flattop, "TRIGGER_FLATTOP"},
            {threshold, "TRIGGER_THRESHOLD"}
    };

    QHBoxLayout *layout = new QHBoxLayout;
    QVBoxLayout *unit_layout = new QVBoxLayout;
    QFormLayout *formLayout = new QFormLayout;

    formLayout->addRow(tr("Risetime:"), risetime);
    formLayout->addRow(tr("Flattop:"), flattop);
    formLayout->addRow(tr("Threshold:"), threshold);

    unit_layout->addWidget(new QLabel(tr("us"), this));
    unit_layout->addWidget(new QLabel(tr("us"), this));
    unit_layout->addWidget(new QLabel(tr("us"), this));

    layout->addLayout(formLayout);
    layout->addLayout(unit_layout);

    setLayout(layout);
}

void TriggerFilter::UpdateLimits(const int &module, const int &channel)
{
    qCDebug(logger) << "Updating trigger filter limits";
    get_numeric_limits<double>(module, channel, risetime, interface, param_map);
    get_numeric_limits<double>(module, channel, flattop, interface, param_map);
    get_numeric_limits<long long>(module, channel, threshold, interface, param_map);

}

void TriggerFilter::UpdateView(const int &module, const int &channel)
{
    // Update the limits
    qCDebug(logger) << "Updating view";
    UpdateLimits(module, channel);
    set_widget_numeric_value<double>(module, channel, risetime, interface, param_map);
    set_widget_numeric_value<double>(module, channel, flattop, interface, param_map);
    set_widget_numeric_value<long long>(module, channel, threshold, interface, param_map);
}

void TriggerFilter::UpdateSettings(const int &module, const int &channel)
{
    qCDebug(logger) << "Writing values";
    write_channel_value(module, channel, interface, param_map[risetime], risetime->value());
    write_channel_value(module, channel, interface, param_map[flattop], flattop->value());
    write_channel_value(module, channel, interface, param_map[threshold], threshold->value());
}