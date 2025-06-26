#ifndef XIACONFIGURATOR_H
#define XIACONFIGURATOR_H

#include <QDialog>
#include <QGroupBox>

QT_BEGIN_NAMESPACE
class QSpinBox;
class QLineEdit;
class QTabWidget;
class QCheckBox;
QT_END_NAMESPACE

#include <map>
#include <string>

class XIAInterface;
class TimingFilterTab;
class ChannelRegisterTab;
class ModuleRegisterTab;
class CopySettingsTab;

class Buttons : public QWidget
{
Q_OBJECT
public:
    explicit Buttons(QWidget *parent = nullptr);

protected:
    QCheckBox *all_chan;
    QPushButton *blcutAdjustBtn;
    QPushButton *blAdjustBtn;
    QPushButton *writeBtn;
    QPushButton *saveBtn;
    QPushButton *saveAsBtn;
    friend class XIAConfigurator;
};

class XIAConfigurator : public QDialog
{
Q_OBJECT
public:
    XIAConfigurator(XIAInterface *interface, QWidget *parent = nullptr);
    ~XIAConfigurator() = default;

    void UpdateView(const int &modID, const int &chanID);
    void WriteView(const int &modID, const int &chanID);

private slots:
    void module_change(int);
    void channel_change(int);
    void WriteButtonClick(bool);
    void MeasureBaselineCut(bool);
    void MeasureBaselineOffset(bool);
    void SaveSettingsButtonClick(bool);
    void SaveAsSettingsButtonClick(bool);
    void DoUpdate();

private:
    XIAInterface *interface;
    QSpinBox *module;
    QSpinBox *channel;
    QLineEdit *module_revision;
    QLineEdit *module_ADCbits;
    QLineEdit *module_ADCmsps;
    QLineEdit *module_serial;
    QTabWidget *tabWidget;
    TimingFilterTab *timingFilterTab;
    ChannelRegisterTab *channelRegisterTab;
    ModuleRegisterTab *moduleRegisterTab;
    CopySettingsTab *copySettingsTab;
    Buttons *buttons;
};
#endif // XIACONFIGURATOR_H
