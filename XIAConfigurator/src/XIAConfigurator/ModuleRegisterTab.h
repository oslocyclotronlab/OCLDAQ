//
// Created by Vetle Wegner Ingeberg on 02/12/2024.
//

#ifndef XIACONFIGURATOR_MODULEREGISTERTAB_H
#define XIACONFIGURATOR_MODULEREGISTERTAB_H

#include <QWidget>

QT_BEGIN_NAMESPACE
class QCheckBox;
class QSpinBox;
class QLineEdit;
QT_END_NAMESPACE

class XIAInterface;

class ModuleRegisterTab : public QWidget
{
    Q_OBJECT
private:
    XIAInterface *interface;
public:
    ModuleRegisterTab(XIAInterface *interface, QWidget *parent = nullptr);

    void UpdateView(const int &module, const int &channel);
    void UpdateSettings(const int &module, const int &channel);

private:
    QCheckBox *modCSRB[9];
    QSpinBox *fastFilterRange;
    QSpinBox *slowFilterRange;

    QLineEdit *trigConfig0;
    QLineEdit *trigConfig1;
    QLineEdit *trigConfig2;
    QLineEdit *trigConfig3;

    void UpdateLimits();
};

#endif //XIACONFIGURATOR_MODULEREGISTERTAB_H
