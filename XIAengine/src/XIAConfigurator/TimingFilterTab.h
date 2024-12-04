//
// Created by Vetle Wegner Ingeberg on 02/12/2024.
//

#ifndef XIACONFIGURATOR_TIMINGFILTERTABWIDGET_H
#define XIACONFIGURATOR_TIMINGFILTERTABWIDGET_H

#include <QWidget>

class XIAInterface;
class TriggerFilter;
class EnergyFilter;
class CFDFilter;
class BaselineFilter;
class TimingSettings;
class QDCFilter;

class TimingFilterTab : public QWidget
{
    Q_OBJECT
public:
    TimingFilterTab(XIAInterface *interface, QWidget *parent = nullptr);

    void UpdateView(const int &module, const int &channel);
    void UpdateSettings(const int &module, const int &channel);

private:
    TriggerFilter *triggerFilter;
    EnergyFilter *energyFilter;
    CFDFilter *cfdFilter;
    BaselineFilter *baselineFilter;
    TimingSettings *timingSettings;
    QDCFilter *qdcFilter;

};

#endif //XIACONFIGURATOR_TIMINGFILTERTABWIDGET_H
