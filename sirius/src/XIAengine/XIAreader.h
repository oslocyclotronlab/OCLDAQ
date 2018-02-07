#ifndef XIAREADER_H
#define XIAREADER_H

/*!
 * \brief ReadSettingsFile - Reads the configuration file for the current experiment and updates internal variables to reflect the setup.
 * \param cfgfile
 * \return True if the configuration file was successfully parsed, false otherwise.
 */
bool ReadSettingsFile(const char *cfgfile);


/*!
 * \brief StartReadout - Main thread for readout of the XIA modules.
 * \param MinReadout - Minimum number of words in one of the modules before forcing readout of all modules.
 */
void StartReadout(unsigned int MinReadout = 16384);

#endif // XIAREADER_H
