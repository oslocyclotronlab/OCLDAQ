//
// Created by Vetle Wegner Ingeberg on 02/12/2024.
//

#define PRESET_MAX_MODULES 24

#include "CopySettingsTab.h"

#include <iostream>

#include "xiainterface.h"

#include <QHBoxLayout>
#include <QLabel>
#include <QComboBox>
#include <QLineEdit>
#include <QCheckBox>
#include <QSpinBox>
#include <QTableWidget>
#include <QPushButton>
#include <QGroupBox>
#include <QHeaderView>

#include "helpers.h"

unsigned short APP16_SetBit (
    unsigned short bit,
    unsigned short value )
{
    return(value | (unsigned short)(pow(2.0, (double)bit)));
}

unsigned short APP16_ClrBit (
    unsigned short bit,
    unsigned short value )
{
    value = APP16_SetBit(bit, value);
    return(value ^ (unsigned short)(pow(2.0, (double)bit)));
}

const char *copyNames[] = {
        "Filter (energy/trigger) [0]",
        "Analog signal conditioning [1]",
        "Histogram settings [2]",
        "Decay time [3]",
        "PSA (trace len/delay) [4]",
        "Baseline control [5]",
        "Channel register (CSRA) [6]",
        "CFD settings [8]",
        "Trigger strech length [9]",
        "FIFO delay [10]",
        "Multiplicity mask [11]",
        "QDC [12]"
};

CopySettingsTab::CopySettingsTab(XIAInterface *_interface, QWidget *parent)
        : QWidget{parent}
        , interface( _interface )
        , number_of_modules( interface->GetNumModules() )
        , source_module( new QSpinBox(this) )
        , source_channel( new QSpinBox(this) )
        , table( new QTableWidget(this) )
        , copyBtn( new QPushButton(tr("Copy"), this) )
        , clearBtn( new QPushButton(tr("Clear"), this) )
{

    source_module->setMinimum(0);
    source_module->setMaximum(interface->GetNumModules()-1);

    source_channel->setMinimum(0);
    source_channel->setMaximum(NUMBER_OF_CHANNELS-1);


    QVBoxLayout *layout = new QVBoxLayout;

    QGroupBox *sourceBox = new QGroupBox(tr("Source:"), this);
    QHBoxLayout *source_layout = new QHBoxLayout;
    source_layout->addLayout(getLayoutUnitless(this, "Module:", source_module));
    source_layout->addLayout(getLayoutUnitless(this, "Channel:", source_channel));
    source_layout->addStretch();
    sourceBox->setLayout(source_layout);
    layout->addWidget(sourceBox);

    table->setColumnCount(NUMBER_OF_CHANNELS);
    table->setRowCount(interface->GetNumModules());

    for ( size_t i = 0 ; i < NUMBER_OF_CHANNELS ; ++i ){
        table->setHorizontalHeaderItem(i, new QTableWidgetItem(tr(std::to_string(i).c_str())));
    }

    for ( size_t i = 0 ; i < interface->GetNumModules() ; ++i ){
        table->setHorizontalHeaderItem(i, new QTableWidgetItem(tr(std::to_string(i).c_str())));
    }

    for (size_t i = 0 ; i < interface->GetNumModules() ; ++i){
        for (size_t j = 0 ; j < NUMBER_OF_CHANNELS ; ++j){
            table->setCellWidget(i, j, new QCheckBox());
        }
    }

    table->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    table->verticalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    connect(table->horizontalHeader(), &QHeaderView::sectionDoubleClicked,
            this, &CopySettingsTab::horizontalHeaderSectionDoubleClicked);
    connect(table->verticalHeader(), &QHeaderView::sectionDoubleClicked,
            this, &CopySettingsTab::verticalHeaderSectionDoubleClicked);
    layout->addWidget(table);

    QHBoxLayout *cp_layout = new QHBoxLayout;
    auto *cp_lhs = new QVBoxLayout;
    auto *cp_rhs = new QVBoxLayout;
    for ( size_t n = 0 ; n < 12 ; ++n ){
        copyMask[n] = new QCheckBox(copyNames[n]);
        if ( n / 6 == 0 )
            cp_lhs->addWidget(copyMask[n]);
        else
            cp_rhs->addWidget(copyMask[n]);
    }
    cp_layout->addLayout(cp_lhs);
    cp_layout->addLayout(cp_rhs);
    cp_layout->addStretch();
    layout->addLayout(cp_layout);

    copyBtn->setDefault(false);
    clearBtn->setDefault(false);

    auto *btn_layout = new QHBoxLayout;
    btn_layout->addWidget(copyBtn);
    btn_layout->addWidget(clearBtn);
    btn_layout->addStretch();
    layout->addLayout(btn_layout);

    connect(copyBtn, &QPushButton::clicked, this, &CopySettingsTab::copyBtn_push);
    connect(clearBtn, &QPushButton::clicked, this, &CopySettingsTab::clearBtn_push);

    layout->addStretch();
    setLayout(layout);
}

void CopySettingsTab::horizontalHeaderSectionDoubleClicked(int sec)
{
    // If all is checked, then we uncheck.
    int n_checked = 0;
    for (size_t i = 0 ; i < number_of_modules ; ++i){
        QCheckBox *box = (QCheckBox *)table->cellWidget(i, sec);
        if (box->isChecked())
            ++n_checked;
    }
    for (size_t i = 0 ; i < number_of_modules ; ++i){
        QCheckBox *box = (QCheckBox *)table->cellWidget(i, sec);
        if (n_checked == number_of_modules)
            box->setChecked(false);
        else
            box->setChecked(true);
    }
}

void CopySettingsTab::verticalHeaderSectionDoubleClicked(int sec)
{
    // If all is checked, then we uncheck.
    int n_checked = 0;
    for (size_t i = 0 ; i < NUMBER_OF_CHANNELS ; ++i){
        QCheckBox *box = (QCheckBox *)table->cellWidget(sec, i);
        if (box->isChecked())
            ++n_checked;
    }
    for (size_t i = 0 ; i < NUMBER_OF_CHANNELS ; ++i){
        QCheckBox *box = (QCheckBox *)table->cellWidget(sec, i);
        if (n_checked == NUMBER_OF_CHANNELS)
            box->setChecked(false);
        else
            box->setChecked(true);
    }
}


void CopySettingsTab::copyBtn_push()
{
    unsigned short destinationMask[NUMBER_OF_CHANNELS * PRESET_MAX_MODULES];
    for (int i = 0 ; i < number_of_modules ; ++i){
        for (int j = 0 ; j < NUMBER_OF_CHANNELS ; ++j){
            auto box = reinterpret_cast<QCheckBox *>(table->cellWidget(i, j));
            destinationMask[i*NUMBER_OF_CHANNELS + j] = ( box->isChecked() ) ? 1 : 0;
        }
    }

    unsigned short cp_mask = 0;

    for ( int n = 0 ; n < 12 ; ++n ) {
        if ( n == 6 ) {
            cp_mask = APP16_ClrBit(n, cp_mask);
            continue;
        }
        cp_mask = ( copyMask[n]->isChecked() ) ? APP16_SetBit(n, cp_mask) : APP16_ClrBit(n, cp_mask);
    }

    interface->CopyDSPParameters(cp_mask,
        source_module->value(), source_channel->value(), destinationMask);
}

void CopySettingsTab::clearBtn_push()
{
    for ( size_t i = 0 ; i < number_of_modules ; ++i ){
        for ( size_t j = 0 ; j < NUMBER_OF_CHANNELS ; ++j ){
            auto box = reinterpret_cast<QCheckBox *>(table->cellWidget(i, j));
            box->setChecked(false);
        }
    }

    for (auto & n : copyMask){
        n->setChecked(false);
    }
}