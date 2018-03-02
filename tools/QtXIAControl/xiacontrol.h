#ifndef XIACONTROL_H
#define XIACONTROL_H

#include <QMainWindow>
#include <QTableWidgetItem>

#include "XIASetup.h"

namespace Ui {
class XIAControl;
}

class XIAControl : public QMainWindow
{
    Q_OBJECT

public:
    explicit XIAControl(QWidget *parent = 0);
    ~XIAControl();

private slots:
    void on_numModSB_valueChanged(int state);

    void on_modTable_itemChanged(QTableWidgetItem *item);

    void on_InitButton_clicked();

    void on_Exit_button_clicked();

    void on_Ready_button_clicked();

    void on_Boot_button_clicked();

    void on_offlineCB_stateChanged(int state);

    void on_syncModCB_stateChanged(int state);

    void on_simStCB_stateChanged(int state);

private:
    Ui::XIAControl *ui;


    // This method should be called whenever
    // we click any button and the user options
    // might have changed.
    void UpdateView();

    // We will override close event so that we make sure to ask the user
    // if the user actually wants to close the program while we are still
    // running.
    void closeEvent(QCloseEvent *event);

    // Method for communicating with the XIAengine to determine if there is
    // an ongoing run.
    bool CheckRun();

    // If system is initialized.
    bool isInit;

    // If system is booted
    bool isBooted;

    // If system is in a run.
    bool isRun;

    // List of current settings.
    XIASetup setup;

};

#endif // XIACONTROL_H
