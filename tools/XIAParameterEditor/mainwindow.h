#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QtWidgets>

#include <map>
#include <string>
#include <vector>

#include "xia_settings.h"

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = 0);
    ~MainWindow();

public slots:
    void updateChange(QTableWidgetItem *item);
    void saveButtonPressed();
    void clearButtonPressed();
    void changeModule(int newMod);

protected:

    void setupContents(int module);
    bool saveChanges(int module);

    void saveToPrivate(int module);
    void savePrivateToFile();

private:

    QLabel *moduleLabel;
    QSpinBox *moduleNo;
    QToolBar *modToolBar;

    QTableWidget *table;

    QPushButton *saveButton;
    QPushButton *resetButton;
    QToolBar *butToolBar;

    QVBoxLayout *mainLayout;

    QErrorMessage *errmsg;

    // Mapping of Parameter name to
    // parameter address.
    typedef std::map<std::string, int> var_m_t;
    var_m_t var_map[PRESET_MAX_MODULES];

    // Variable to store all parameters.
    unsigned int DSP_var[PRESET_MAX_MODULES][N_DSP_PAR];

    // Number of modules
    int NumMod;

    // Current selected module.
    int currentMod;

    // Mapping of row and DSP address.
    int row_address[N_DSP_PAR];

    // Copy of original values.
    unsigned int row_value[N_DSP_PAR];

    typedef struct {
        int row;
        int address;
        int old_value;
        int new_value;
        std::string name;
    } change_t;

    // List of changes.
    std::vector<change_t> changes;

    // Path to .set file
    std::string setfile;

    // Check if we have changed any of the values.
    bool IsChanged();

    bool ReadConfFile(const char *filename);

    bool ReadSetFile(const char *filename);

    bool ReadVarFile(const char *filename, int modNo);
};

#endif // MAINWINDOW_H
