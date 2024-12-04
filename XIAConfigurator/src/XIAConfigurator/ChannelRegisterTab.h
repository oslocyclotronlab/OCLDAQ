//
// Created by Vetle Wegner Ingeberg on 02/12/2024.
//

#ifndef XIACONFIGURATOR_CHANNELREGISTERTAB_H
#define XIACONFIGURATOR_CHANNELREGISTERTAB_H

#include <map>
#include <QWidget>

QT_BEGIN_NAMESPACE
class QCheckBox;
class QLineEdit;
class QComboBox;
QT_END_NAMESPACE

class XIAInterface;

class ChannelRegisterTab : public QWidget
{
    Q_OBJECT
private:
    XIAInterface *interface;
public:
    ChannelRegisterTab(XIAInterface *interface, QWidget *parent = nullptr);

    void UpdateView(const int &module, const int &channel);
    void UpdateSettings(const int &module, const int &channel);

private:
    QCheckBox *csra[20];
    QLineEdit *multMaskL;
    QLineEdit *multMaskH;
    QComboBox *pileup;
    std::map<QWidget *, int> csra_bit_map;

    void UpdateLimits(const int &module, const int &channel);
};

#endif //XIACONFIGURATOR_CHANNELREGISTERTAB_H
