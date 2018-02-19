#include "mainwindow.h"

#include <iostream>
#include <fstream>
#include <sstream>
#include <algorithm>

#include <cstdio>

std::string chan_param_name[N_DSP_CHAN_PAR] = {
    "ChanCSRa",
    "ChanCSRb",
    "GainDAC",
    "OffsetDAC",
    "DigGain",
    "SlowLength",
    "SlowGap",
    "FastLength",
    "FastGap",
    "PeakSample",
    "PeakSep",
    "CFDThresh",
    "FastThresh",
    "ThreshWidth",
    "PAFlength",
    "TriggerDelay",
    "ResetDelay",
    "ChanTrigStretch",
    "TraceLength",
    "Xwait",
    "TrigOutLen",
    "EnergyLow",
    "Log2Ebin",
    "MultiplicityMaskL",
    "PSAoffset",
    "PSAlength",
    "Integrator",
    "BLcut",
    "BaselinePercent",
    "FtrigoutDelay",
    "Log2Bweight",
    "PreampTau",
    "Xavg",
    "MultiplicityMaskH",
    "FastTrigBackLen",
    "CFDDelay",
    "CFDScale",
    "ExtTrigStretch",
    "VetoStretch",
    "ExternDelayLen",
    "QDCLen0",
    "QDCLen1",
    "QDCLen2",
    "QDCLen3",
    "QDCLen4",
    "QDCLen5",
    "QDCLen6",
    "QDCLen7"
};

// Helper functions
inline bool next_line(std::istream &in, std::string &line)
{
    line = "";

    std::string tmp;
    while ( std::getline(in, tmp) ){
        size_t ls = tmp.size();
        if ( ls == 0 ){
            break;
        } else if ( tmp[ls-1] != '\\' ){
            line += tmp;
            break;
        } else {
            line += tmp.substr(0, ls-1);
        }
    }
    return in || !line.empty();
}


MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    // Create error message box.
    errmsg = new QErrorMessage(this);

    if ( !ReadConfFile("cfgXIA.txt")){
        // Do something...
    }
    addToolBar(Qt::TopToolBarArea, modToolBar = new QToolBar());
    moduleLabel = new QLabel(tr("Module:"), modToolBar);
    moduleNo = new QSpinBox(modToolBar);
    moduleNo->setMinimum(0);
    moduleNo->setMaximum(NumMod);
    moduleNo->setValue(0);

    modToolBar->addWidget(moduleLabel);
    modToolBar->addWidget(moduleNo);

    addToolBar(Qt::BottomToolBarArea, butToolBar = new QToolBar());
    saveButton = new QPushButton(tr("Save"), butToolBar);
    resetButton = new QPushButton(tr("Reset"), butToolBar);

    butToolBar->addWidget(saveButton);
    butToolBar->addWidget(resetButton);

    table = new QTableWidget(N_DSP_PAR, 2, this);
    table->setSizeAdjustPolicy(QTableWidget::AdjustToContents);
    table->setColumnWidth(0, 146);
    table->setColumnWidth(1, 97);
    table->setHorizontalHeaderItem(0, new QTableWidgetItem(tr("Parameter")));
    table->setHorizontalHeaderItem(1, new QTableWidgetItem(tr("Value")));

    setupContents(0);
    setCentralWidget(table);
    setFixedWidth(299);

    statusBar();
    connect(table, &QTableWidget::currentItemChanged, this, &MainWindow::updateChange);
    connect(table, &QTableWidget::itemChanged, this, &MainWindow::updateChange);
    connect(saveButton, &QPushButton::clicked, this, &MainWindow::saveButtonPressed);
    connect(resetButton, &QPushButton::clicked, this, &MainWindow::clearButtonPressed);
    connect(moduleNo, static_cast<void (QSpinBox::*)(int)>(&QSpinBox::valueChanged), this, &MainWindow::changeModule);
}

MainWindow::~MainWindow()
{

}

void MainWindow::changeModule(int newMod)
{
    if (newMod >= NumMod){
        errmsg->showMessage("Unable to change module");
        moduleNo->setValue(NumMod-1);
        currentMod = NumMod-1;
    } else if (currentMod == newMod) {
        return;
    } else if ( !changes.empty() ){
        if ( saveChanges(currentMod) ){
            currentMod = newMod;
            moduleNo->setValue(newMod);
            return;
        } else {
            moduleNo->setValue(currentMod);
            return;
        }
    } else {
        currentMod = newMod;
        moduleNo->setValue(newMod);
        return;
    }
}

void MainWindow::updateChange(QTableWidgetItem *item)
{
    change_t the_change;
    if ( item && item == table->currentItem() ){
        the_change.row = item->row();
        if (the_change.row < 0)
            return;
        the_change.address = row_address[the_change.row];
        the_change.old_value = row_value[the_change.row];
        the_change.new_value = item->data(Qt::DisplayRole).toUInt();
        // If old value is the same as new, return without changing anything.
        if (the_change.old_value == the_change.new_value)
            return;
        if ( table->item(the_change.row, 0) )
            the_change.name = table->item(the_change.row, 0)->data(Qt::DisplayRole).toString().toStdString();
        // Check if this is "a change to the change"
        for (size_t i = 0 ; i < changes.size() ; ++i){
            if ( changes[i].address == the_change.address ){
                changes[i] = the_change;
                return; // We have added the change.
            }
        }

        changes.push_back(the_change); // We should only reach this point if this is a "new change".
    }
    return;
}

void MainWindow::saveButtonPressed()
{
    // Save changes to file? No, only save to internal memory.
    if (changes.empty())
        return;

    if ( saveChanges(moduleNo->value()) ){
        statusBar()->showMessage(tr("Changes saved"));
    }

}
void MainWindow::clearButtonPressed()
{
    // If no changes, return.
    if (changes.empty())
        return;

    std::string change_txt = "";
    change_txt += "Parameter:\tNew value:\tOld value:\n";
    for (size_t i = 0 ; i < changes.size() ; ++i){
        change_txt += changes[i].name;
        change_txt += "\t";
        change_txt += std::to_string(changes[i].new_value) + "\t";
        change_txt += std::to_string(changes[i].old_value) + "\n";
    }

    // Ask the user if we really want to change the values
    QMessageBox msgBox(QMessageBox::Warning, tr("Discard changes"), tr("Are you sure you want to discard changes?"), 0, this);

    msgBox.setDetailedText(change_txt.c_str());
    msgBox.addButton(tr("&Yes"), QMessageBox::AcceptRole);
    msgBox.addButton(tr("&No"), QMessageBox::RejectRole);


    if (msgBox.exec() == QMessageBox::AcceptRole){
        setupContents(moduleNo->value());
        statusBar()->showMessage(tr("All changes reset"));
    } else {
        statusBar()->showMessage(tr("Nothing..."));
    }


}

bool MainWindow::saveChanges(int module)
{
    // First we check if there is any changes to be written.
    // If no changes, then we return true.
    if ( changes.empty() )
        return true;

    // Create a human readable summary of changes.
    std::string change_txt = "";
    change_txt += "Parameter:\tNew value:\tOld value:\n";
    for (size_t i = 0 ; i < changes.size() ; ++i){
        change_txt += changes[i].name;
        change_txt += "\t";
        change_txt += std::to_string(changes[i].new_value) + "\t";
        change_txt += std::to_string(changes[i].old_value) + "\n";
    }

    // Ask the user if we really want to change the values
    QMessageBox msgBox(QMessageBox::Warning, tr("Save changes?"), tr("Do you want to save changes?"), 0, this);

    msgBox.setDetailedText(change_txt.c_str());
    msgBox.addButton(tr("&Save"), QMessageBox::AcceptRole);
    msgBox.addButton(tr("&Cancel"), QMessageBox::RejectRole);
    if (msgBox.exec() == QMessageBox::AcceptRole){
        saveToPrivate(module);
        savePrivateToFile();
        return true;
    } else {
        return false;
    }

    // We should never reach this point!
    return false;
}

void MainWindow::saveToPrivate(int module)
{
    for (size_t i = 0 ; i < changes.size() ; ++i){
        DSP_var[module][changes[i].address] = changes[i].new_value;
    }
    changes.clear();
    return;
}

void MainWindow::savePrivateToFile()
{
    FILE *file = fopen(setfile.c_str(), "wb");

    if (!file){
        std::cerr << __PRETTY_FUNCTION__ << ": Unable to open file '" << setfile << "'" << std::endl;
        return;
    } else {
        for (int i = 0 ; i < PRESET_MAX_MODULES ; ++i){
            if (fwrite(DSP_var[i], sizeof(unsigned int), N_DSP_PAR, file) != N_DSP_PAR){
                std::cerr << __PRETTY_FUNCTION__ << ": Error while writing to file '" << setfile << "'" << std::endl;
                fclose(file);
                return;
            }
        }
    }
    // Close file
    fclose(file);
    return;
}


void MainWindow::setupContents(int module)
{
    // We reset all changes and values.
    changes.clear();
    for (int i = 0 ; i < N_DSP_PAR ; ++i){
        row_address[i] = 0;
        row_value[i] = 0;
    }

    table->clear();
    table->setHorizontalHeaderItem(0, new QTableWidgetItem(tr("Parameter")));
    table->setHorizontalHeaderItem(1, new QTableWidgetItem(tr("Value")));
    table->setColumnWidth(0, 146);
    table->setColumnWidth(1, 97);

    for (int i = 0 ; i < N_DSP_PAR ; ++i)
        table->setItem(i, 0, new QTableWidgetItem(tr("")));

    if ( var_map[module].empty() ){
        for (int i = 0 ; i < N_DSP_PAR ; ++i){
            QTableWidgetItem *itm = new QTableWidgetItem();
            itm->setData(Qt::EditRole, 0);
            table->setItem(i, 1, itm);
            table->item(i, 0)->setFlags(table->item(i, 0)->flags() ^ Qt::ItemIsEditable);
            row_address[i] = i;
        }
        return;
    }

    int row=0;
    bool is_chan_par = false;
    char tmp[2048];
    for (var_m_t::iterator it = var_map[module].begin() ; it != var_map[module].end() ; ++it){
        if (it->first.empty())
            continue;
        is_chan_par = false;
        for (int i = 0 ; i < N_DSP_CHAN_PAR ; ++i){
            if ( it->first == chan_param_name[i] ){
                is_chan_par = true;
                break;
            }
        }

        // TrigConfigX is four consequtive entries. This is not reflected in the var file.
        // we get the rest by catching that entry.
        if (it->first == "TrigConfig"){
            for (int i = 0 ; i < 4 ; ++i){
                sprintf(tmp, "%s%d", it->first.c_str(), i);
                table->setItem(row, 0, new QTableWidgetItem(tr(tmp)));
                unsigned int value = DSP_var[module][it->second + i - DATA_MEMORY_ADDRESS];
                row_address[row] = it->second - DATA_MEMORY_ADDRESS;
                row_value[row] = value;
                QTableWidgetItem *itm = new QTableWidgetItem();
                itm->setData(Qt::EditRole, value);
                table->setItem(row++, 1, itm);
            }
            continue;
        }


        if (is_chan_par){
            sprintf(tmp, "%s %d", it->first.c_str(), 0);
            table->setItem(row, 0, new QTableWidgetItem(tr(tmp)));
            unsigned int value = DSP_var[module][it->second + 0 - DATA_MEMORY_ADDRESS];
            row_address[row] = it->second + 0 - DATA_MEMORY_ADDRESS;
            row_value[row] = value;
            QTableWidgetItem *itm = new QTableWidgetItem();
            itm->setData(Qt::EditRole, value);
            table->setItem(row++, 1, itm);
            for (int i = 1 ; i < NUMBER_OF_CHANNELS ; ++i){
                sprintf(tmp, "%s %d", it->first.c_str(), i);
                table->setItem(row, 0, new QTableWidgetItem(tr(tmp)));
                value = DSP_var[module][it->second + i - DATA_MEMORY_ADDRESS];
                row_address[row] = it->second + i - DATA_MEMORY_ADDRESS;
                row_value[row] = value;
                itm = new QTableWidgetItem();
                itm->setData(Qt::EditRole, value);
                table->setItem(row++, 1, itm);
            }
        } else {
            sprintf(tmp, "%s", it->first.c_str());
            table->setItem(row, 0, new QTableWidgetItem(tr(tmp)));
            unsigned int value = DSP_var[module][it->second - DATA_MEMORY_ADDRESS];
            row_address[row] = it->second - DATA_MEMORY_ADDRESS;
            row_value[row] = value;
            QTableWidgetItem *itm = new QTableWidgetItem();
            itm->setData(Qt::EditRole, value);
            table->setItem(row++, 1, itm);
        }
    }

    // Make sure that the first column is read only.
    for (int i = 0 ; i < N_DSP_PAR ; ++i){
        table->item(i, 0)->setFlags(table->item(i, 0)->flags() ^ Qt::ItemIsEditable);
    }

    return;
}

bool MainWindow::ReadConfFile(const char *filename)
{
    // We will read the conf file to find the .var files and
    // the .set files.

    std::ifstream infile(filename);
    std::string line;

    if ( !infile.is_open() )
        return false;

    while ( next_line(infile, line) ){
        if ( line.empty() || line[0] == '#' )
            continue;

        // We are looking for "SET 'filename'", "VAR <modNo> 'filename'" and "nmod <NumMod>"
        std::istringstream icmd(line.c_str());
        std::string cmd, tmp;

        icmd >> cmd;
        std::transform(cmd.begin(), cmd.end(), cmd.begin(), ::toupper);
        if (cmd == "SET" ) {
            icmd >> tmp;
            if ( !ReadSetFile(tmp.c_str()) )
                return false;
            setfile = tmp;
        } else if ( cmd == "VAR" ){

            int curr_mod;
            icmd >> curr_mod;
            if ( curr_mod > NumMod )
                return false;
            icmd >> tmp;
            if ( !ReadVarFile(tmp.c_str(), curr_mod) )
                return false;
        } else if (cmd == "NMOD" ){
            int num;
            icmd >> num;
            if (num > 0)
                NumMod = num;
            else
                return false;
        }
    }
    return true;
}

bool MainWindow::ReadSetFile(const char *filename)
{
    FILE *file = fopen(filename, "rb");

    if ( !file ){
        std::cerr << __PRETTY_FUNCTION__ << ": Could not open file '" << filename << "'" << std::endl;
        return false;
    }

    // Check file size
    int totalWords = 0;
    fseek(file, 0, SEEK_END);
    totalWords = (ftell(file) + 1) / 4;

    if (totalWords != (N_DSP_PAR * PRESET_MAX_MODULES) ){
        std::cerr << __PRETTY_FUNCTION__ << ": Size mismatch in file '" << filename << "'" << std::endl;
        fclose(file);
        return false;
    }

    // Move back to the beginning of the file
    fseek(file, 0, SEEK_SET);

    for (int i = 0 ; i < PRESET_MAX_MODULES ; ++i){

        if ( fread(DSP_var[i], sizeof(unsigned int), N_DSP_PAR, file) != N_DSP_PAR ){
            std::cerr << __PRETTY_FUNCTION__ << ": Errror while reading settings from file '" << filename << "'" << std::endl;
            fclose(file);
            return false;
        }

        DSP_var[i][0] = i;
    }

    fclose(file);
    return true;
}


bool MainWindow::ReadVarFile(const char *filename, int modNo)
{
    // Make sure there are nothing in the current var_mapping
    var_map[modNo].clear();

    std::ifstream file(filename);
    while ( file ){
        int address=0;
        std::string par_name;
        file >> std::hex >> address >> par_name;
        if (var_map[modNo].find(par_name) != var_map[modNo].end() ){
            std::cerr << __PRETTY_FUNCTION__ << ": Error while reading file '" << filename << "'" << std::endl;
            return false;
        }
        var_map[modNo][par_name] = address;
    }

    return true;
}
