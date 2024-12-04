//
// Created by Vetle Wegner Ingeberg on 02/12/2024.
//

#include "ChannelRegisterTab.h"
#include "xiainterface.h"

#include <QHBoxLayout>
#include <QLabel>
#include <QComboBox>
#include <QLineEdit>
#include <QCheckBox>

#include "helpers.h"

unsigned int APP32_SetBit (unsigned short bit, unsigned int value)
{
    return(value | (unsigned int)(pow(2.0, (double)bit)));
}

unsigned int APP32_ClrBit (unsigned short bit, unsigned int value)
{
    value = APP32_SetBit(bit, value);
    return(value ^ (unsigned int)(pow(2.0, (double)bit)));
}

const char *CSRAnames[] = {
        "Fast trig. sel. [0]",
        "Ext. trig. sel. [1]",
        "Channel enabled [2]",
        "Chan. Trig. Sel. [3]",
        "Block DAQ [4]",
        "Polarity [5]",
        "Veto enable [6]",
        "Histogram enable [7]",
        "Trace enable [8]",
        "QDC enable [9]",
        "CFD enable [10]",
        "Glob. Trig. for validation [11]",
        "Energy sums [12]",
        "Validation enable [13]",
        "Input relay enable [14]",
        "Energy cut [17]",
        "Group trig. sel. [18]",
        "Chan. veto sel. [19]",
        "Mod. veto sel. [20]",
        "Ext. timestamp enable [21]"
};

static CSRmap_t CSRAmap[] = {
        {0, "Fast trig. sel. [0]"},
        {1, "Ext. trig. sel. [1]"},
        {2, "Channel enabled [2]"},
        {3, "Chan. Trig. Sel. [3]"},
        {4, "Block DAQ [4]"},
        {5, "Polarity [5]"},
        {6, "Veto enable [6]"},
        {7, "Histogram enable [7]"},
        {8, "Trace enable [8]"},
        {9, "QDC enable [9]"},
        {10, "CFD enable [10]"},
        {11, "Glob. Trig. for validation [11]"},
        {12, "Energy sums [12]"},
        {13, "Validation enable [13]"},
        {14, "Input relay enable [14]"},
        {17, "Energy cut [17]"},
        {18, "Group trig. sel. [18]"},
        {19, "Chan. veto sel. [19]"},
        {20, "Mod. veto sel. [20]"},
        {21, "Ext. timestamp enable [21]"}
};

ChannelRegisterTab::ChannelRegisterTab(XIAInterface *_interface, QWidget *parent)
        : QWidget{parent}
        , interface( _interface )
        , multMaskL( new QLineEdit(this) )
        , multMaskH( new QLineEdit(this) )
        , pileup( new QComboBox(this) )
{
    QVBoxLayout *layout = new QVBoxLayout;
    QHBoxLayout *csra_layout = new QHBoxLayout;
    auto *csra_lhs = new QVBoxLayout;
    auto *csra_rhs = new QVBoxLayout;
    for ( size_t n = 0 ; n < 20 ; ++n ){
        csra[n] = new QCheckBox(CSRAmap[n].name);
        if ( n / 10 == 0 )
            csra_lhs->addWidget(csra[n]);
        else
            csra_rhs->addWidget(csra[n]);
    }

    //csra_layout->addStretch();
    csra_layout->addLayout(csra_lhs);
    csra_layout->addLayout(csra_rhs);
    csra_layout->addStretch();
    layout->addLayout(csra_layout);

    auto multLayout = new QHBoxLayout;
    auto multLHS = new QVBoxLayout;
    auto multRHS = new QVBoxLayout;

    multMaskL->setInputMask("HHHHHHHH");
    multMaskL->setText("00000000");
    multMaskH->setInputMask("HHHHHHHH");
    multMaskH->setText("00000000");

    multLHS->addWidget(new QLabel(tr("Multiplicity mask L:")));
    multLHS->addWidget(new QLabel(tr("Multiplicity mask H:")));
    multRHS->addWidget(multMaskL);
    multRHS->addWidget(multMaskH);
    multLayout->addLayout(multLHS);
    multLayout->addLayout(multRHS);
    multLayout->addStretch();
    layout->addLayout(multLayout);

    pileup->addItem("Record all [00]");
    pileup->addItem("Reject pile-up [01]");
    pileup->addItem("Record all + pile-up trace [10]");
    pileup->addItem("Reject non-pileup [11]");
    layout->addWidget(pileup);
    layout->addStretch();
    setLayout(layout);
}

void ChannelRegisterTab::UpdateLimits(const int &module, const int &channel)
{

}

#ifndef CCSRA_PILEUPCTRL
#define CCSRA_PILEUPCTRL 15
#endif // CCSRA_PILEUPCTRL

#ifndef CCSRA_INVERSEPILEUP
#define CCSRA_INVERSEPILEUP 16
#endif // CCSRA_INVERSEPILEUP


void ChannelRegisterTab::UpdateView(const int &module, const int &channel)
{
    // Need to fetch the current settings for the module and populate the data.
    unsigned long long csra_value = read_channel_numeric_value<unsigned long long>(module, channel, interface, "CHANNEL_CSRA");
    for ( size_t i = 0 ; i < 20 ; ++i ){
        csra[i]->setChecked(test_bit(CSRAmap[i].bit, csra_value));
    }

    if ( !test_bit(CCSRA_PILEUPCTRL, csra_value) && !test_bit(CCSRA_INVERSEPILEUP, csra_value) ){
        pileup->setCurrentIndex(0);
    } else if ( test_bit(CCSRA_PILEUPCTRL, csra_value) && !test_bit(CCSRA_INVERSEPILEUP, csra_value) ){
        pileup->setCurrentIndex(1);
    } else if ( !test_bit(CCSRA_PILEUPCTRL, csra_value) && test_bit(CCSRA_INVERSEPILEUP, csra_value) ){
        pileup->setCurrentIndex(2);
    } else if ( test_bit(CCSRA_PILEUPCTRL, csra_value) && test_bit(CCSRA_INVERSEPILEUP, csra_value) ){
        pileup->setCurrentIndex(3);
    }

    multMaskL->setText(n2hexstr(read_channel_numeric_value<unsigned int>(module, channel, interface, "MultiplicityMaskL")).c_str());
    multMaskH->setText(n2hexstr(read_channel_numeric_value<unsigned int>(module, channel, interface, "MultiplicityMaskH")).c_str());

}

void ChannelRegisterTab::UpdateSettings(const int &module, const int &channel)
{
    unsigned long long csra_value = 0;
    for ( size_t i = 0 ; i < 20 ; ++i ){
        csra_value = set_bit(CSRAmap[i].bit, csra_value, csra[i]->isChecked());
    }

    switch( pileup->currentIndex() ){
        case 0:
            csra_value = set_bit(CCSRA_PILEUPCTRL, csra_value, false);
            csra_value = set_bit(CCSRA_INVERSEPILEUP, csra_value, false);
            break;
        case 1:
            csra_value = set_bit(CCSRA_PILEUPCTRL, csra_value, true);
            csra_value = set_bit(CCSRA_INVERSEPILEUP, csra_value, false);
            break;
        case 2:
            csra_value = set_bit(CCSRA_PILEUPCTRL, csra_value, false);
            csra_value = set_bit(CCSRA_INVERSEPILEUP, csra_value, true);
            break;
        case 3:
            csra_value = set_bit(CCSRA_PILEUPCTRL, csra_value, false);
            csra_value = set_bit(CCSRA_INVERSEPILEUP, csra_value, true);
            break;
        default:
            // This is an error...
            break;
    }

    write_channel_value(module, channel, interface, "CHANNEL_CSRA", csra_value);
    write_channel_value(module, channel, interface, "MultiplicityMaskL", std::stoull(multMaskL->text().toStdString(), nullptr, 16));
    write_channel_value(module, channel, interface, "MultiplicityMaskH", std::stoull(multMaskH->text().toStdString(), nullptr, 16));

}