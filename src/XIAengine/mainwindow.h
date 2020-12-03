#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(int num_mod, QWidget *parent = 0);
    ~MainWindow();

private slots:
    void on_current_mod_valueChanged(int arg1);

    void on_current_channel_valueChanged(int arg1);

    void on_WriteButton_clicked();

    void on_SaveButton_clicked();

    void on_SaveAsButton_clicked();

    void horizontalHeaderSectionDoubleClicked(int sec);

    void verticalHeaderSectionDoubleClicked(int sec);

    void on_ClearButton_clicked();

    void on_CopyButton_clicked();

    void on_AdjBLineC_clicked();

    void on_AdjBLine_clicked();

private:
    Ui::MainWindow *ui;

    //! Private value to store the module count.
    int n_modules;

    //! Current module selected.
    unsigned short current_module;

    //! Current channel selected.
    unsigned short current_channel;

    //! Slow filter range of current module.
    unsigned int current_slow_filter;

    //! Function responcible for determining the ranges of the parameters and update them accordingly.
    void UpdateLimits();

    //! Function responsible for reading from module and populating the values.
    void UpdateViewChannel();

    //! Method responsible for updating module view.
    void UpdateViewModule();

    //! Function to call when module is changed.
    void ChangeModule();

    //! Update the view.
    void UpdateView();

    //! Function to make table in 'copy' tab.
    void MakeCopyTable();

    //! Save settings to file.
    void SaveSettings(char *filename);
};

#endif // MAINWINDOW_H
