//
// Created by Vetle Wegner Ingeberg on 21/11/2024.
//
#include "PlxApi.h"
#include "pixie16sys_defs.h"
#include "pixie16sys_globals.h"

#ifdef __cplusplus
extern "C" {
#endif

PLX_UINT_PTR 		VAddr[SYS_MAX_NUM_MODULES];			// PCI device virutal address
PLX_DEVICE_OBJECT	SYS_hDevice[SYS_MAX_NUM_MODULES];	// PCI device handle
unsigned short		SYS_Number_Modules;					// Total number of modules in the crate
unsigned short		SYS_Offline;						// SYS_Offline = 1: offline mode; SYS_Offline = 0: Online mode

double  Ns_Per_Cycle;									// The time needed for each cycle, in ns



#ifdef __cplusplus
}
#endif	// End of notice for C++ compilers