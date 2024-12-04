//
// Created by Vetle Wegner Ingeberg on 02/12/2024.
//

#ifndef XIACONFIGURATOR_QDCFILTERWIDGET_HH
#define XIACONFIGURATOR_QDCFILTERWIDGET_HH

#include <map>
#include <QGroupBox>

QT_BEGIN_NAMESPACE
class QWidget;
class QDoubleSpinBox;
class QSpinBox;
QT_END_NAMESPACE

class XIAInterface;

class QDCFilter : public QGroupBox
{
    Q_OBJECT
private:
    XIAInterface *interface;
public:
    QDCFilter(XIAInterface *interface, QWidget *parent = nullptr);

    void UpdateView(const int &module, const int &channel);
    void UpdateSettings(const int &module, const int &channel);

private:
    QDoubleSpinBox *qdcLen0;
    QDoubleSpinBox *qdcLen1;
    QDoubleSpinBox *qdcLen2;
    QDoubleSpinBox *qdcLen3;
    QDoubleSpinBox *qdcLen4;
    QDoubleSpinBox *qdcLen5;
    QDoubleSpinBox *qdcLen6;
    QDoubleSpinBox *qdcLen7;
    std::map<QWidget *, const char*> param_map;

    void UpdateLimits(const int &module, const int &channel);

};

#endif //XIACONFIGURATOR_QDCFILTERWIDGET_HH
