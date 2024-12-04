//
// Created by Vetle Wegner Ingeberg on 02/12/2024.
//

#include "ModuleRegisterTab.h"
#include "xiainterface.h"

#include <QHBoxLayout>
#include <QLabel>
#include <QComboBox>
#include <QLineEdit>
#include <QCheckBox>
#include <QSpinBox>

#include "enumerate.h"
#include "helpers.h"

const char *CSRBnames[] = {
        "Pull-ups [0]",
        "Director [4]",
        "Chassis master [6]",
        "Global trigger source [7]",
        "External trigger source [8]",
        "External inhibit [10]",
        "Multicrate [11]",
        "Sort events [12]",
        "Backplane fast trigger [13]"
};

CSRmap_t CSRBmap[] = {
        {0, "Pull-ups [0]"},
        {4, "Director [4]"},
        {6, "Chassis master [6]"},
        {7, "Global trigger source [7]"},
        {8, "External trigger source [8]"},
        {10, "External inhibit [10]"},
        {11, "Multicrate [11]"},
        {12, "Sort events [12]"},
        {13, "Backplane fast trigger [13]"}
};

ModuleRegisterTab::ModuleRegisterTab(XIAInterface *_interface, QWidget *parent)
        : QWidget{parent}
        , interface( _interface )
        , fastFilterRange( new QSpinBox(this) )
        , slowFilterRange( new QSpinBox(this) )
        , trigConfig0( new QLineEdit(this) )
        , trigConfig1( new QLineEdit(this) )
        , trigConfig2( new QLineEdit(this) )
        , trigConfig3( new QLineEdit(this) )
{
    QVBoxLayout *layout = new QVBoxLayout;
    QHBoxLayout *csrb_layout = new QHBoxLayout;
    auto *csrb_lhs = new QVBoxLayout;
    auto *csrb_rhs = new QVBoxLayout;
    for ( size_t n = 0 ; n < 9 ; ++n ){
        modCSRB[n] = new QCheckBox(CSRBmap[n].name);
        if ( n / 5 == 0 )
            csrb_lhs->addWidget(modCSRB[n]);
        else
            csrb_rhs->addWidget(modCSRB[n]);
    }

    csrb_layout->addLayout(csrb_lhs);
    csrb_layout->addLayout(csrb_rhs);
    csrb_layout->addStretch();
    layout->addLayout(csrb_layout);

    auto *rangeLayout = new QHBoxLayout;
    rangeLayout->addWidget(new QLabel(tr("Fast filter range:"), this));
    rangeLayout->addWidget(fastFilterRange);
    rangeLayout->addWidget(new QLabel(tr("Slow filter range:"), this));
    rangeLayout->addWidget(slowFilterRange);
    rangeLayout->addStretch();
    layout->addLayout(rangeLayout);

    trigConfig0->setInputMask("HHHHHHHH");
    trigConfig1->setInputMask("HHHHHHHH");
    trigConfig2->setInputMask("HHHHHHHH");
    trigConfig3->setInputMask("HHHHHHHH");

    trigConfig0->setText("00000000");
    trigConfig1->setText("00000000");
    trigConfig2->setText("00000000");
    trigConfig3->setText("00000000");

    auto *trigLayout0 = new QHBoxLayout;
    trigLayout0->addWidget(new QLabel("TrigConfig0:", this));
    trigLayout0->addWidget(trigConfig0);
    trigLayout0->addWidget(new QLabel("TrigConfig1:", this));
    trigLayout0->addWidget(trigConfig1);
    layout->addLayout(trigLayout0);

    auto *trigLayout1 = new QHBoxLayout;
    trigLayout1->addWidget(new QLabel("TrigConfig2:", this));
    trigLayout1->addWidget(trigConfig2);
    trigLayout1->addWidget(new QLabel("TrigConfig3:", this));
    trigLayout1->addWidget(trigConfig3);
    layout->addLayout(trigLayout1);

    layout->addStretch();
    setLayout(layout);
}


void ModuleRegisterTab::UpdateView(const int &module, const int &channel)
{
    // Need to fetch the current settings for the module and populate the data.
    constexpr size_t bitmap[] = {0, 4, 6, 7, 8, 10, 11, 12, 13};
    unsigned int csrb = read_module_numeric_value<unsigned int>(module, interface, "MODULE_CSRB");

    for ( auto [n, modSel] : enumerate(modCSRB) ){
        modSel->setChecked(test_bit(CSRBmap[n].bit, csrb));
    }

    fastFilterRange->setValue(read_module_numeric_value<unsigned int>(module, interface, "FAST_FILTER_RANGE"));
    slowFilterRange->setValue(read_module_numeric_value<unsigned int>(module, interface, "SLOW_FILTER_RANGE"));

    trigConfig0->setText(n2hexstr(read_module_numeric_value<unsigned int>(module, interface, "TrigConfig0")).c_str());
    trigConfig1->setText(n2hexstr(read_module_numeric_value<unsigned int>(module, interface, "TrigConfig1")).c_str());
    trigConfig2->setText(n2hexstr(read_module_numeric_value<unsigned int>(module, interface, "TrigConfig2")).c_str());
    trigConfig3->setText(n2hexstr(read_module_numeric_value<unsigned int>(module, interface, "TrigConfig3")).c_str());

}

void ModuleRegisterTab::UpdateSettings(const int &module, const int &channel)
{
    unsigned int csrb = 0;
    for ( auto [n, modSel] : enumerate(modCSRB) ){
        csrb = set_bit(CSRBmap[n].bit, csrb, modSel->isChecked());
    }
    write_module_value(module, interface, "MODULE_CSRB", csrb);
    write_module_value(module, interface, "FAST_FILTER_RANGE", fastFilterRange->value());
    write_module_value(module, interface, "SLOW_FILTER_RANGE", slowFilterRange->value());
    write_module_value(module, interface, "TrigConfig0", std::stoul(trigConfig0->text().toStdString(), nullptr, 16));
    write_module_value(module, interface, "TrigConfig1", std::stoul(trigConfig1->text().toStdString(), nullptr, 16));
    write_module_value(module, interface, "TrigConfig2", std::stoul(trigConfig2->text().toStdString(), nullptr, 16));
    write_module_value(module, interface, "TrigConfig3", std::stoul(trigConfig3->text().toStdString(), nullptr, 16));
}