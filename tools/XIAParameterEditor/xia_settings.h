#ifndef XIA_SETTINGS_H
#define XIA_SETTINGS_H


#define N_DSP_PAR                   1280      // number of DSP parameters (32-bit word)
#define PRESET_MAX_MODULES            24      // Preset maximum number of Pixie modules
#define NUMBER_OF_CHANNELS            16
#define DATA_MEMORY_ADDRESS      0x4A000      // DSP data memory address


#include <string>

#define N_DSP_CHAN_PAR                48      // Number of DSP parameters that are channel parameters

/*std::string chan_param_name[N_DSP_CHAN_PAR] = {
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
};*/

#endif // XIA_SETTINGS_H
