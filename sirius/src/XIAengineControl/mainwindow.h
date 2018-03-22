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
    void on_pushButton_clicked();

private:
    Ui::MainWindow *ui;

    //! Private value to store the module count.
    int n_modules;

    //! Current module selected.
    int current_module;

    //! Current channel selected.
    int current_channel;

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
};

#endif // MAINWINDOW_H
