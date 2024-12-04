//
// Created by Vetle Wegner Ingeberg on 02/12/2024.
//

#ifndef XIACONFIGURATOR_TIMINGSETTINGSWIDGET_H
#define XIACONFIGURATOR_TIMINGSETTINGSWIDGET_H

#include <map>
#include <QGroupBox>

QT_BEGIN_NAMESPACE
class QWidget;
class QDoubleSpinBox;
class QSpinBox;
QT_END_NAMESPACE

class XIAInterface;

class TimingSettings : public QGroupBox
{
    Q_OBJECT
private:
    XIAInterface *interface;
public:
    TimingSettings(XIAInterface *interface, QWidget *parent = nullptr);

    void UpdateView(const int &module, const int &channel);
    void UpdateSettings(const int &module, const int &channel);

private:
    QDoubleSpinBox *traceLength;
    QDoubleSpinBox *traceDelay;
    QDoubleSpinBox *fastTrigBackLen;
    QDoubleSpinBox *extTrigStrech;
    QDoubleSpinBox *externDelayLen;
    QDoubleSpinBox *ftrigoutDelay;
    QDoubleSpinBox *vetoStrech;
    QDoubleSpinBox *chanTrigStrech;
    std::map<QWidget *, const char*> param_map;

    void UpdateLimits(const int &module, const int &channel);
};

#endif //XIACONFIGURATOR_TIMINGSETTINGSWIDGET_H
