#ifndef PIXIE16APP_GLOBALS_H
#define PIXIE16APP_GLOBALS_H

/*----------------------------------------------------------------------
 * Copyright (c) 2005 - 2009, XIA LLC
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, 
 * with or without modification, are permitted provided 
 * that the following conditions are met:
 *
 *   * Redistributions of source code must retain the above 
 *     copyright notice, this list of conditions and the 
 *     following disclaimer.
 *   * Redistributions in binary form must reproduce the 
 *     above copyright notice, this list of conditions and the 
 *     following disclaimer in the documentation and/or other 
 *     materials provided with the distribution.
 *   * Neither the name of XIA LLC nor the names of its
 *     contributors may be used to endorse or promote
 *     products derived from this software without 
 *     specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND 
 * CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, 
 * INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF 
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. 
 * IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE 
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR 
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, 
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, 
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON 
 * ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR 
 * TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF 
 * THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF 
 * SUCH DAMAGE.
 *----------------------------------------------------------------------*/

/******************************************************************************
 *
 * File Name:
 *
 *		pixie16app_globals.h
 *
 * Description:
 *
 *		Declaration of global variables and data arrays.
 *
 * $Rev: 29161 $
 * $Id: pixie16app_globals.h 29161 2014-01-09 19:27:58Z htan $
 ******************************************************************************/

// If this is compiled by a C++ compiler, make it
// clear that these are C routines.
#ifdef __cplusplus
extern "C" {
#endif

#ifndef __PIXIE16APP_DEFS_H
	#include "pixie16app_defs.h"
#endif


extern unsigned short Number_Modules;                       // Total number of modules in the crate
extern unsigned short Random_Set[RANDOMINDICES_LENGTH];     // Random indices
extern unsigned short Offline;   // Offline = 1: offline analysis; Offline = 0: online analysis


/*-----------------------------------------------------------------
    Pixie global data structure
  -----------------------------------------------------------------*/

struct Pixie_Configuration
{
	
	// DSP I/O parameter values
	unsigned int DSP_Parameter_Values[N_DSP_PAR];

};

// Define PRESET_MAX_MODULES Pixie devices
extern struct Pixie_Configuration Pixie_Devices[PRESET_MAX_MODULES];


struct Module_Info
{
	// Module information
	unsigned short Module_Rev;
	unsigned int   Module_SerNum;
	unsigned short Module_ADCBits;
	unsigned short Module_ADCMSPS;
};

// Define PRESET_MAX_MODULES Pixie devices
extern struct Module_Info Module_Information[PRESET_MAX_MODULES];


/*-----------------------------------------------------------------
    DSP I/O parameters address
		We initialize these addressee when we boot the modules, then
		we can use them when we need to change DSP parameters.
  -----------------------------------------------------------------*/

//--------------------
//	Module parameters
//--------------------

extern unsigned int ModNum_Address[PRESET_MAX_MODULES];         // ModNum
extern unsigned int ModCSRA_Address[PRESET_MAX_MODULES];        // ModCSRA
extern unsigned int ModCSRB_Address[PRESET_MAX_MODULES];        // ModCSRB
extern unsigned int ModFormat_Address[PRESET_MAX_MODULES];      // ModFormat
extern unsigned int RunTask_Address[PRESET_MAX_MODULES];        // RunTask
extern unsigned int ControlTask_Address[PRESET_MAX_MODULES];    // ControlTask
extern unsigned int MaxEvents_Address[PRESET_MAX_MODULES];      // MaxEvents
extern unsigned int CoincPattern_Address[PRESET_MAX_MODULES];   // CoincPattern
extern unsigned int CoincWait_Address[PRESET_MAX_MODULES];      // CoincWait
extern unsigned int SynchWait_Address[PRESET_MAX_MODULES];      // SynchWait
extern unsigned int InSynch_Address[PRESET_MAX_MODULES];        // InSynch
extern unsigned int Resume_Address[PRESET_MAX_MODULES];         // Resume
extern unsigned int SlowFilterRange_Address[PRESET_MAX_MODULES];// SlowFilterRange
extern unsigned int FastFilterRange_Address[PRESET_MAX_MODULES];// FastFilterRange
extern unsigned int ChanNum_Address[PRESET_MAX_MODULES];        // ChanNum
extern unsigned int HostIO_Address[PRESET_MAX_MODULES];         // HostIO
extern unsigned int UserIn_Address[PRESET_MAX_MODULES];         // UserIn
extern unsigned int U00_Address[PRESET_MAX_MODULES];            // U00
extern unsigned int FastTrigBackplaneEna_Address[PRESET_MAX_MODULES]; // Enabling sending fast trigger to backplane
extern unsigned int CrateID_Address             [PRESET_MAX_MODULES]; // CrateID
extern unsigned int SlotID_Address              [PRESET_MAX_MODULES]; // SlotID
extern unsigned int ModID_Address               [PRESET_MAX_MODULES]; // ModID
extern unsigned int TrigConfig_Address          [PRESET_MAX_MODULES]; // General configuration registers
extern unsigned int HRTP_Address[PRESET_MAX_MODULES];           // HostRunTimePreset

//--------------------
//	Channel parameters
//--------------------

extern unsigned int ChanCSRa_Address[PRESET_MAX_MODULES];       // ChanCSRa
extern unsigned int ChanCSRb_Address[PRESET_MAX_MODULES];       // ChanCSRb

extern unsigned int GainDAC_Address[PRESET_MAX_MODULES];        // GainDAC

extern unsigned int OffsetDAC_Address[PRESET_MAX_MODULES];      // OffsetDAC

extern unsigned int DigGain_Address[PRESET_MAX_MODULES];        // DigGain

extern unsigned int SlowLength_Address[PRESET_MAX_MODULES];     // SlowLength
extern unsigned int SlowGap_Address[PRESET_MAX_MODULES];        // SlowGap
extern unsigned int FastLength_Address[PRESET_MAX_MODULES];     // FastLength
extern unsigned int FastGap_Address[PRESET_MAX_MODULES];        // FastGap
extern unsigned int PeakSample_Address[PRESET_MAX_MODULES];     // PeakSample
extern unsigned int PeakSep_Address[PRESET_MAX_MODULES];        // PeakSep

extern unsigned int CFDThresh_Address[PRESET_MAX_MODULES];      // CFDThresh

extern unsigned int FastThresh_Address[PRESET_MAX_MODULES];     // FastThresh
extern unsigned int ThreshWidth_Address[PRESET_MAX_MODULES];    // ThreshWidth
extern unsigned int PAFlength_Address[PRESET_MAX_MODULES];      // PAFlength
extern unsigned int TriggerDelay_Address[PRESET_MAX_MODULES];   // TriggerDelay
extern unsigned int ResetDelay_Address[PRESET_MAX_MODULES];     // ResetDelay
extern unsigned int ChanTrigStretch_Address[PRESET_MAX_MODULES];// ChanTrigStretch
extern unsigned int TraceLength_Address[PRESET_MAX_MODULES];    // TraceLength
extern unsigned int TrigOutLen_Address[PRESET_MAX_MODULES];     // TrigOutLen
extern unsigned int EnergyLow_Address[PRESET_MAX_MODULES];      // EnergyLow
extern unsigned int Log2Ebin_Address[PRESET_MAX_MODULES];       // Log2Ebin

extern unsigned int MultiplicityMaskL_Address[PRESET_MAX_MODULES]; // Multiplicity contribution mask - low 32-bit

extern unsigned int PSAoffset_Address[PRESET_MAX_MODULES];      // PSAoffset
extern unsigned int PSAlength_Address[PRESET_MAX_MODULES];      // PSAlength
extern unsigned int Integrator_Address[PRESET_MAX_MODULES];     // Integrator

extern unsigned int BLcut_Address[PRESET_MAX_MODULES];          // BLcut
extern unsigned int BaselinePercent_Address[PRESET_MAX_MODULES];// BaselinePercent

extern unsigned int FtrigoutDelay_Address[PRESET_MAX_MODULES];  // Fast trigger output delay for system synchronization

extern unsigned int Log2Bweight_Address[PRESET_MAX_MODULES];    // Log2Bweight
extern unsigned int PreampTau_Address[PRESET_MAX_MODULES];      // PreampTau

extern unsigned int MultiplicityMaskH_Address[PRESET_MAX_MODULES]; // Multiplicity contribution mask - high 32-bit

extern unsigned int FastTrigBackLen_Address[PRESET_MAX_MODULES];	// FastTrigBackLen

extern unsigned int CFDDelay_Address      [PRESET_MAX_MODULES];	// CFD delay
extern unsigned int CFDScale_Address      [PRESET_MAX_MODULES];	// CFD scale
extern unsigned int ExternDelayLen_Address[PRESET_MAX_MODULES];	// delay length for each channel's input signal
extern unsigned int ExtTrigStretch_Address[PRESET_MAX_MODULES];	// external trigger stretch
extern unsigned int VetoStretch_Address   [PRESET_MAX_MODULES];	// veto stretch
extern unsigned int QDCLen0_Address       [PRESET_MAX_MODULES];	// QDC #0 length
extern unsigned int QDCLen1_Address       [PRESET_MAX_MODULES];	// QDC #1 length
extern unsigned int QDCLen2_Address       [PRESET_MAX_MODULES];	// QDC #2 length
extern unsigned int QDCLen3_Address       [PRESET_MAX_MODULES];	// QDC #3 length
extern unsigned int QDCLen4_Address       [PRESET_MAX_MODULES];	// QDC #4 length
extern unsigned int QDCLen5_Address       [PRESET_MAX_MODULES];	// QDC #5 length
extern unsigned int QDCLen6_Address       [PRESET_MAX_MODULES];	// QDC #6 length
extern unsigned int QDCLen7_Address       [PRESET_MAX_MODULES];	// QDC #7 length

extern unsigned int Xwait_Address[PRESET_MAX_MODULES];				// Xwait


//--------------------
//	Results parameters
//--------------------

extern unsigned int RealTimeA_Address[PRESET_MAX_MODULES];    // RealTimeA
extern unsigned int RealTimeB_Address[PRESET_MAX_MODULES];    // RealTimeB
extern unsigned int RunTimeA_Address[PRESET_MAX_MODULES];     // RunTimeA
extern unsigned int RunTimeB_Address[PRESET_MAX_MODULES];     // RunTimeB
extern unsigned int GSLTtime_Address[PRESET_MAX_MODULES];     // GSLTtime
extern unsigned int NumEventsA_Address[PRESET_MAX_MODULES];   // NumEventsA
extern unsigned int NumEventsB_Address[PRESET_MAX_MODULES];   // NumEventsB
extern unsigned int DSPerror_Address[PRESET_MAX_MODULES];     // DSPerror
extern unsigned int SynchDone_Address[PRESET_MAX_MODULES];    // SynchDone
extern unsigned int BufHeadLen_Address[PRESET_MAX_MODULES];   // BufHeadLen
extern unsigned int EventHeadLen_Address[PRESET_MAX_MODULES]; // EventHeadLen
extern unsigned int ChanHeadLen_Address[PRESET_MAX_MODULES];  // ChanHeadLen
extern unsigned int UserOut_Address[PRESET_MAX_MODULES];      // UserOut
extern unsigned int AOutBuffer_Address[PRESET_MAX_MODULES];   // AOutBuffer
extern unsigned int LOutBuffer_Address[PRESET_MAX_MODULES];   // LOutBuffer
extern unsigned int AECorr_Address[PRESET_MAX_MODULES];       // AECorr
extern unsigned int LECorr_Address[PRESET_MAX_MODULES];       // LECorr
extern unsigned int HardwareID_Address[PRESET_MAX_MODULES];   // HardwareID
extern unsigned int HardVariant_Address[PRESET_MAX_MODULES];  // HardVariant
extern unsigned int FIFOLength_Address[PRESET_MAX_MODULES];   // FIFOLength
extern unsigned int FippiID_Address[PRESET_MAX_MODULES];      // FippiID
extern unsigned int FippiVariant_Address[PRESET_MAX_MODULES]; // FippiVariant
extern unsigned int DSPrelease_Address[PRESET_MAX_MODULES];   // DSPrelease
extern unsigned int DSPbuild_Address[PRESET_MAX_MODULES];     // DSPbuild
extern unsigned int DSPVariant_Address[PRESET_MAX_MODULES];   // DSPVariant
extern unsigned int U20_Address[PRESET_MAX_MODULES];          // U20
extern unsigned int LiveTimeA_Address[PRESET_MAX_MODULES];    // LiveTimeA
extern unsigned int LiveTimeB_Address[PRESET_MAX_MODULES];    // LiveTimeB
extern unsigned int FastPeaksA_Address[PRESET_MAX_MODULES];   // FastPeaksA
extern unsigned int FastPeaksB_Address[PRESET_MAX_MODULES];   // FastPeaksB
extern unsigned int OverflowA_Address[PRESET_MAX_MODULES];    // OverflowA
extern unsigned int OverflowB_Address[PRESET_MAX_MODULES];    // OverflowB
extern unsigned int InSpecA_Address[PRESET_MAX_MODULES];      // InSpecA
extern unsigned int InSpecB_Address[PRESET_MAX_MODULES];      // InSpecB
extern unsigned int UnderflowA_Address[PRESET_MAX_MODULES];   // UnderflowA
extern unsigned int UnderflowB_Address[PRESET_MAX_MODULES];   // UnderflowB
extern unsigned int ChanEventsA_Address[PRESET_MAX_MODULES];  // ChanEventsA
extern unsigned int ChanEventsB_Address[PRESET_MAX_MODULES];  // ChanEventsB
extern unsigned int AutoTau_Address[PRESET_MAX_MODULES];      // AutoTau
extern unsigned int U30_Address[PRESET_MAX_MODULES];          // U30



#ifdef __cplusplus
}
#endif	// End of notice for C++ compilers

#endif	// End of pixie16app_globals.h

