//
// Created by Vetle Wegner Ingeberg on 02/12/2024.
//

#ifndef XIACONFIGURATOR_COPYSETTINGSTAB_H
#define XIACONFIGURATOR_COPYSETTINGSTAB_H

#include <QWidget>

QT_BEGIN_NAMESPACE
class QSpinBox;
class QTableWidget;
class QCheckBox;
class QPushButton;
QT_END_NAMESPACE

class XIAInterface;

class CopySettingsTab : public QWidget
{
    Q_OBJECT
private:
    XIAInterface *interface;
public:
    CopySettingsTab(XIAInterface *interface, QWidget *parent = nullptr);

signals:
    void UpdateView();

private slots:
    void horizontalHeaderSectionDoubleClicked(int sec);
    void verticalHeaderSectionDoubleClicked(int sec);

    void copyBtn_push();
    void clearBtn_push();

private:
    size_t number_of_modules;
    QSpinBox *source_module;
    QSpinBox *source_channel;
    QTableWidget *table;
    QCheckBox *copyMask[13];
    QPushButton *copyBtn;
    QPushButton *clearBtn;
};

#endif //XIACONFIGURATOR_COPYSETTINGSTAB_H
