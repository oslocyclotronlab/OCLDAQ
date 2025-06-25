//
// Created by Vetle Wegner Ingeberg on 02/12/2024.
//

#include "QDCFilterWidget.h"
#include "xiainterface.h"

#include <QSpinBox>
#include <QDoubleSpinBox>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QFormLayout>
#include <QLabel>

#include "helpers.h"

QDCFilter::QDCFilter(XIAInterface *_interface, QWidget *parent)
        : QGroupBox{"QDCFilter", parent}
        , interface( _interface )
        , qdcLen0( new QDoubleSpinBox(this) )
        , qdcLen1( new QDoubleSpinBox(this) )
        , qdcLen2( new QDoubleSpinBox(this) )
        , qdcLen3( new QDoubleSpinBox(this) )
        , qdcLen4( new QDoubleSpinBox(this) )
        , qdcLen5( new QDoubleSpinBox(this) )
        , qdcLen6( new QDoubleSpinBox(this) )
        , qdcLen7( new QDoubleSpinBox(this) )
{

    QHBoxLayout *layout = new QHBoxLayout;
    QVBoxLayout *unit_layout = new QVBoxLayout;
    QFormLayout *formLayout = new QFormLayout;

    param_map = {
            {qdcLen0, "QDCLen0"},
            {qdcLen1, "QDCLen1"},
            {qdcLen2, "QDCLen2"},
            {qdcLen3, "QDCLen3"},
            {qdcLen4, "QDCLen4"},
            {qdcLen5, "QDCLen5"},
            {qdcLen6, "QDCLen6"},
            {qdcLen7, "QDCLen7"},
    };

    for ( auto& qdc : param_map ) {
        reinterpret_cast<QDoubleSpinBox *>(qdc.first)->setDecimals(3);
        reinterpret_cast<QDoubleSpinBox *>(qdc.first)->setSingleStep(0.001);
    }

    formLayout->addRow(tr("QDC sum length %1:").arg(0), qdcLen0);
    unit_layout->addWidget(new QLabel(tr("us"), this));
    formLayout->addRow(tr("QDC sum length %1:").arg(1), qdcLen1);
    unit_layout->addWidget(new QLabel(tr("us"), this));
    formLayout->addRow(tr("QDC sum length %1:").arg(2), qdcLen2);
    unit_layout->addWidget(new QLabel(tr("us"), this));
    formLayout->addRow(tr("QDC sum length %1:").arg(3), qdcLen3);
    unit_layout->addWidget(new QLabel(tr("us"), this));
    formLayout->addRow(tr("QDC sum length %1:").arg(4), qdcLen4);
    unit_layout->addWidget(new QLabel(tr("us"), this));
    formLayout->addRow(tr("QDC sum length %1:").arg(5), qdcLen5);
    unit_layout->addWidget(new QLabel(tr("us"), this));
    formLayout->addRow(tr("QDC sum length %1:").arg(6), qdcLen6);
    unit_layout->addWidget(new QLabel(tr("us"), this));
    formLayout->addRow(tr("QDC sum length %1:").arg(7), qdcLen7);
    unit_layout->addWidget(new QLabel(tr("us"), this));

    layout->addLayout(formLayout);
    layout->addLayout(unit_layout);
    setLayout(layout);
}

void QDCFilter::UpdateLimits(const int &module, const int &channel)
{
    get_numeric_limits<double>(module, channel, qdcLen0, interface, param_map);
    get_numeric_limits<double>(module, channel, qdcLen1, interface, param_map);
    get_numeric_limits<double>(module, channel, qdcLen2, interface, param_map);
    get_numeric_limits<double>(module, channel, qdcLen3, interface, param_map);
    get_numeric_limits<double>(module, channel, qdcLen4, interface, param_map);
    get_numeric_limits<double>(module, channel, qdcLen5, interface, param_map);
    get_numeric_limits<double>(module, channel, qdcLen6, interface, param_map);
    get_numeric_limits<double>(module, channel, qdcLen7, interface, param_map);
}

void QDCFilter::UpdateView(const int &module, const int &channel)
{
    // Need to fetch the current settings for the module and populate the data.
    UpdateLimits(module, channel);
    set_widget_numeric_value<double>(module, channel, qdcLen0, interface, param_map);
    set_widget_numeric_value<double>(module, channel, qdcLen1, interface, param_map);
    set_widget_numeric_value<double>(module, channel, qdcLen2, interface, param_map);
    set_widget_numeric_value<double>(module, channel, qdcLen3, interface, param_map);
    set_widget_numeric_value<double>(module, channel, qdcLen4, interface, param_map);
    set_widget_numeric_value<double>(module, channel, qdcLen5, interface, param_map);
    set_widget_numeric_value<double>(module, channel, qdcLen6, interface, param_map);
    set_widget_numeric_value<double>(module, channel, qdcLen7, interface, param_map);
}

void QDCFilter::UpdateSettings(const int &module, const int &channel)
{
    write_channel_value(module, channel, interface, param_map[qdcLen0], qdcLen0->value());
    write_channel_value(module, channel, interface, param_map[qdcLen1], qdcLen1->value());
    write_channel_value(module, channel, interface, param_map[qdcLen2], qdcLen2->value());
    write_channel_value(module, channel, interface, param_map[qdcLen3], qdcLen3->value());
    write_channel_value(module, channel, interface, param_map[qdcLen4], qdcLen4->value());
    write_channel_value(module, channel, interface, param_map[qdcLen5], qdcLen5->value());
    write_channel_value(module, channel, interface, param_map[qdcLen6], qdcLen6->value());
    write_channel_value(module, channel, interface, param_map[qdcLen7], qdcLen7->value());

}