//
// Created by Vetle Wegner Ingeberg on 02/12/2024.
//

#ifndef XIACONFIGURATOR_BASELINEFILTERWIDGET_H
#define XIACONFIGURATOR_BASELINEFILTERWIDGET_H

#include <map>
#include <QGroupBox>

QT_BEGIN_NAMESPACE
class QWidget;
class QDoubleSpinBox;
class QSpinBox;
QT_END_NAMESPACE

class XIAInterface;

class BaselineFilter : public QGroupBox
{
Q_OBJECT
private:
    XIAInterface *interface;
public:
    BaselineFilter(XIAInterface *interface, QWidget *parent = nullptr);

    void UpdateView(const int &module, const int &channel);
    void UpdateSettings(const int &module, const int &channel);

private:
    QDoubleSpinBox *baselineOffset;
    QSpinBox *baselinePercent;
    QSpinBox *baselineAverage;
    QSpinBox *baselineCut;
    std::map<QWidget *, const char*> param_map;

    void UpdateLimits(const int &module, const int &channel);
};

#endif //XIACONFIGURATOR_BASELINEFILTERWIDGET_H
