#include "xiacontrol.h"
#include "ui_xiacontrol.h"

#include <iostream>
#include <thread>
#include <chrono>

#include <QMessageBox>
#include <QCloseEvent>

#define TEST_MESSAGES

XIAControl::XIAControl(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::XIAControl),
    isInit(false),
    isBooted(false),
    isRun(false)
{
    ui->setupUi(this);

    // First check if we can read settings from file.
    // If unable, use a default version of setup.

    // Read from file

    // else
    {
        setup.numMod = 1;
        for (int i = 0 ; i < 24 ; ++i){
            setup.PXISlotMap[i] = 2+i;
        }
        setup.SetFileName = "settings.set";
        setup.FirmwareFile = "firmware.dat";
    }

    UpdateView();
}

XIAControl::~XIAControl()
{
    delete ui;
}

void XIAControl::closeEvent(QCloseEvent *event)
{
    // Check if still running...
    if (isRun){
        // If we are running we will check and ask the user to close.
        isRun = CheckRun();

    }

    // Close application if and only if the isRun is false.
    if ( !isRun )
        event->accept();
    else
        event->ignore();

}

void XIAControl::UpdateView()
{
    ui->numModSB->setValue(setup.numMod);
    ui->numModSB->setDisabled( isInit );

    ui->modTable->setRowCount(setup.numMod);
    ui->modTable->setColumnCount(2);

    ui->modTable->setHorizontalHeaderItem(0, new QTableWidgetItem(tr("Module no.")));
    ui->modTable->setHorizontalHeaderItem(1, new QTableWidgetItem(tr("PXI slot")));
    ui->modTable->setColumnWidth(0, 90);
    ui->modTable->setColumnWidth(1, 90);

    for (unsigned int i = 0 ; i < setup.numMod && i < PRESET_MAX_MODULES ; ++i){

        if (ui->modTable->item(i,0))
            delete ui->modTable->item(i,0);
        if (ui->modTable->item(i,1))
            delete ui->modTable->item(i,1);

        QTableWidgetItem *itm = new QTableWidgetItem();
        itm->setData(Qt::EditRole, setup.PXISlotMap[i]);
        char tmp[1024];
        sprintf(tmp, "%d", i);
        ui->modTable->setItem(i, 0, new QTableWidgetItem(tr(tmp)));
        ui->modTable->setItem(i, 1, itm);
        ui->modTable->item(i, 0)->setFlags(ui->modTable->item(i, 0)->flags() ^ Qt::ItemIsEditable);
        if (isInit)
            ui->modTable->item(i, 1)->setFlags(ui->modTable->item(i, 1)->flags() ^ Qt::ItemIsEditable);
    }

    ui->offlineCB->setDisabled(isInit);
    ui->offlineCB->setChecked(setup.offlineMode);

    ui->InitButton->setDisabled(isInit);
    ui->Exit_button->setDisabled( (!isInit || isRun) );

    ui->syncModCB->setChecked(setup.SynchMod);
    ui->syncModCB->setDisabled(isRun);

    ui->simStCB->setChecked(setup.StartSync);
    ui->simStCB->setDisabled(isRun);

    ui->Boot_button->setDisabled( (!isInit || isRun) );
    ui->BLine_button->setDisabled( (isRun || !isBooted) );
    ui->BCut_button->setDisabled( (isRun || !isBooted) );

    if ( isRun )
        ui->Ready_button->setText("Stop");
    else
        ui->Ready_button->setText("Start");
    ui->Ready_button->setDisabled( (!isInit || !isBooted ) );

}

bool XIAControl::CheckRun()
{
#ifdef TEST_MESSAGES
    int ret = QMessageBox::critical(this, tr("Run still active"),
                                    tr("There is an active run in the XIA modules. Are you sure you want to force quit?"),
                                    QMessageBox::No | QMessageBox::Yes, QMessageBox::No);

    switch ( ret ) {
    case QMessageBox::Yes:
        return false;
        break;
    case QMessageBox::No:
        return true;
        break;
    default:
        return true;
        break;
    }
#endif // TEST_MESSAGES

}

void XIAControl::on_numModSB_valueChanged(int num)
{
    setup.numMod = num;
    UpdateView();
}

void XIAControl::on_modTable_itemChanged(QTableWidgetItem *item)
{
    if (!item || item != ui->modTable->currentItem())
        return; // Some error... we need to stop everything now...

    // Module number is the row!
    setup.PXISlotMap[item->row()] = item->data(Qt::DisplayRole).toUInt();
}

void XIAControl::on_InitButton_clicked()
{
    isInit = true;
    ui->statusBar->showMessage("Initialized");
    UpdateView();
}

void XIAControl::on_Exit_button_clicked()
{
    isInit = false;
    isBooted = false;
    isRun = false;
    ui->statusBar->showMessage("Un-initialized");
    UpdateView();
}

void XIAControl::on_Ready_button_clicked()
{
    if ( isRun ){

        // Check if there is an ongoing run.
        isRun = CheckRun();

    } else {
        ui->statusBar->showMessage("Ready for run");
        isRun = true;
        // Tell the rest of the system that we are ready for data.
    }

    UpdateView();
}

void XIAControl::on_Boot_button_clicked()
{
    ui->statusBar->showMessage("Booting...");
#ifdef TEST_MESSAGES
    std::this_thread::sleep_for(std::chrono::seconds(1));
#endif // TEST_MESSAGES
    ui->statusBar->showMessage("Booted");
    isBooted = true;
    UpdateView();
}

void XIAControl::on_offlineCB_stateChanged(int state)
{
    setup.offlineMode = (state < 1) ? 0 : 1;
    UpdateView();
}

void XIAControl::on_syncModCB_stateChanged(int state)
{
    setup.SynchMod = (state < 1) ? 0 : 1;
    UpdateView();
}

void XIAControl::on_simStCB_stateChanged(int state)
{
    setup.StartSync = (state < 1) ? 0 : 1;
    UpdateView();
}
