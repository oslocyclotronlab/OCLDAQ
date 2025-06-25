//
// Created by Vetle Wegner Ingeberg on 02/12/2024.
//

#include "TimingSettingsWidget.h"
#include "xiainterface.h"

#include <QDoubleSpinBox>
#include <QSpinBox>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QFormLayout>
#include <QLabel>

#include "helpers.h"

TimingSettings::TimingSettings(XIAInterface *_interface, QWidget *parent)
        : QGroupBox{"Timing", parent}
        , interface( _interface )
        , traceLength( new QDoubleSpinBox(this) )
        , traceDelay( new QDoubleSpinBox(this) )
        , fastTrigBackLen( new QDoubleSpinBox(this) )
        , extTrigStrech( new QDoubleSpinBox(this) )
        , externDelayLen( new QDoubleSpinBox(this) )
        , ftrigoutDelay( new QDoubleSpinBox(this) )
        , vetoStrech( new QDoubleSpinBox(this) )
        , chanTrigStrech( new QDoubleSpinBox(this) )
{
    traceLength->setMinimumWidth(75);
    traceDelay->setMinimumWidth(75);
    fastTrigBackLen->setMinimumWidth(75);
    extTrigStrech->setMinimumWidth(75);
    externDelayLen->setMinimumWidth(75);
    ftrigoutDelay->setMinimumWidth(75);
    vetoStrech->setMinimumWidth(75);
    chanTrigStrech->setMinimumWidth(75);
    param_map = {
            {traceLength, "TRACE_LENGTH"},
            {traceDelay, "TRACE_DELAY"},
            {fastTrigBackLen, "FASTTRIGBACKLEN"},
            {extTrigStrech, "ExtTrigStretch"},
            {externDelayLen, "ExternDelayLen"},
            {ftrigoutDelay, "FtrigoutDelay"},
            {vetoStrech, "VetoStretch"},
            {chanTrigStrech, "ChanTrigStretch"}
    };

    for (auto& param : param_map) {
        reinterpret_cast<QDoubleSpinBox *>(param.first)->setDecimals(3);
        reinterpret_cast<QDoubleSpinBox *>(param.first)->setSingleStep(0.001);
    }

    QHBoxLayout *layout = new QHBoxLayout;
    QVBoxLayout *unit_layout = new QVBoxLayout;
    QFormLayout *formLayout = new QFormLayout;

    formLayout->addRow(tr("Trace length:"), traceLength);
    formLayout->addRow(tr("Trace delay:"), traceDelay);
    formLayout->addRow(tr("FastTrigBackLen:"), fastTrigBackLen);
    formLayout->addRow(tr("ExtTrigStrech:"), extTrigStrech);
    formLayout->addRow(tr("ExternDelayLen:"), externDelayLen);
    formLayout->addRow(tr("FTrigOutDelay:"), ftrigoutDelay);
    formLayout->addRow(tr("Veto Strech:"), vetoStrech);
    formLayout->addRow(tr("ChanTrigStrech:"), chanTrigStrech);


    unit_layout->addWidget(new QLabel(tr("us"), this));
    unit_layout->addWidget(new QLabel(tr("us"), this));
    unit_layout->addWidget(new QLabel(tr("us"), this));
    unit_layout->addWidget(new QLabel(tr("us"), this));
    unit_layout->addWidget(new QLabel(tr("us"), this));
    unit_layout->addWidget(new QLabel(tr("us"), this));
    unit_layout->addWidget(new QLabel(tr("us"), this));
    unit_layout->addWidget(new QLabel(tr("us"), this));

    layout->addLayout(formLayout);
    layout->addLayout(unit_layout);
    setLayout(layout);
}

void TimingSettings::UpdateLimits(const int &module, const int &channel)
{
    get_numeric_limits<double>(module, channel, traceLength, interface, param_map);
    get_numeric_limits<double>(module, channel, traceDelay, interface, param_map);
    get_numeric_limits<double>(module, channel, fastTrigBackLen, interface, param_map);
    get_numeric_limits<double>(module, channel, extTrigStrech, interface, param_map);
    get_numeric_limits<double>(module, channel, externDelayLen, interface, param_map);
    get_numeric_limits<double>(module, channel, ftrigoutDelay, interface, param_map);
    get_numeric_limits<double>(module, channel, vetoStrech, interface, param_map);
    get_numeric_limits<double>(module, channel, chanTrigStrech, interface, param_map);
}

void TimingSettings::UpdateView(const int &module, const int &channel)
{
    // Need to fetch the current settings for the module and populate the data.
    UpdateLimits(module, channel);
    set_widget_numeric_value<double>(module, channel, traceLength, interface, param_map);
    set_widget_numeric_value<double>(module, channel, traceDelay, interface, param_map);
    set_widget_numeric_value<double>(module, channel, fastTrigBackLen, interface, param_map);
    set_widget_numeric_value<double>(module, channel, extTrigStrech, interface, param_map);
    set_widget_numeric_value<double>(module, channel, externDelayLen, interface, param_map);
    set_widget_numeric_value<double>(module, channel, ftrigoutDelay, interface, param_map);
    set_widget_numeric_value<double>(module, channel, vetoStrech, interface, param_map);
    set_widget_numeric_value<double>(module, channel, chanTrigStrech, interface, param_map);
}

void TimingSettings::UpdateSettings(const int &module, const int &channel)
{
    write_channel_value(module, channel, interface, param_map[traceLength], traceLength->value());
    write_channel_value(module, channel, interface, param_map[traceDelay], traceDelay->value());
    write_channel_value(module, channel, interface, param_map[fastTrigBackLen], fastTrigBackLen->value());
    write_channel_value(module, channel, interface, param_map[extTrigStrech], extTrigStrech->value());
    write_channel_value(module, channel, interface, param_map[externDelayLen], externDelayLen->value());
    write_channel_value(module, channel, interface, param_map[ftrigoutDelay], ftrigoutDelay->value());
    write_channel_value(module, channel, interface, param_map[vetoStrech], vetoStrech->value());
    write_channel_value(module, channel, interface, param_map[chanTrigStrech], chanTrigStrech->value());
}