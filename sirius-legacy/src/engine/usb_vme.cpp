
#include "usb_vme.h"

#include "CAENVMElib.h"

static int32_t BHandle;

static void test_error( const char* what, CVErrorCodes err )
{
    if( err != cvSuccess ) {
	printf("\nError %s\n >> %s <<\n", what, CAENVME_DecodeError(err));
	exit(1);
    }
}

void CAEN_V1718_open()
{
    // Initialize the Board
    CVBoardTypes VMEBoard = cvV1718;
    short Device = 0;
    short Link = 0;
    CVErrorCodes err = CAENVME_Init(VMEBoard, Device, Link, &BHandle);
    test_error("opening the USB device", err);
#if 0
    DO NOT USE THIS -- IT SEEMS TO MAKE A FULL RESET ON THE VME BUS, NOT ONLY THE USB MODULE.

    err = CAENVME_SystemReset(BHandle);
    test_error("resetting the USB device", err);
#endif
}

void CAEN_V1718_close()
{
    CAENVME_End(BHandle);
}

void CAEN_V1718_write(unsigned char  data, address8  a, unsigned long base, const char* errmsg)
{
    CVErrorCodes err = CAENVME_WriteCycle(BHandle, a.a+base, &data,
					  a.am32 ? cvA32_U_DATA : cvA24_U_DATA, cvD8);
    test_error(errmsg, err);
}

void CAEN_V1718_write(unsigned short data, address16 a, unsigned long base, const char* errmsg)
{
    CVErrorCodes err = CAENVME_WriteCycle(BHandle, a.a+base, &data,
					  a.am32 ? cvA32_U_DATA : cvA24_U_DATA, cvD16);
    test_error(errmsg, err);
}

void CAEN_V1718_write(unsigned long  data, address32 a, unsigned long base, const char* errmsg)
{
    CVErrorCodes err = CAENVME_WriteCycle(BHandle, a.a+base, &data,
					  a.am32 ? cvA32_U_DATA : cvA24_U_DATA, cvD32);
    test_error(errmsg, err);
}

unsigned char  CAEN_V1718_read(address8  a, unsigned long base, const char* errmsg)
{
    unsigned char data = 0;
    CVErrorCodes err = CAENVME_ReadCycle(BHandle, a.a+base, &data,
					 a.am32 ? cvA32_U_DATA : cvA24_U_DATA, cvD8);
    test_error(errmsg, err);
    return data;
}

unsigned short CAEN_V1718_read(address16 a, unsigned long base, const char* errmsg)
{
    unsigned short data = 0;
    CVErrorCodes err = CAENVME_ReadCycle(BHandle, a.a+base, &data,
					 a.am32 ? cvA32_U_DATA : cvA24_U_DATA, cvD16);
    test_error(errmsg, err);
    return data;
}

unsigned long  CAEN_V1718_read(address32 a, unsigned long base, const char* errmsg)
{
    unsigned long data = 0;
    CVErrorCodes err = CAENVME_ReadCycle(BHandle, a.a+base, &data,
					 a.am32 ? cvA32_U_DATA : cvA24_U_DATA, cvD32);
    test_error(errmsg, err);
    return data;
}

int CAEN_V1718_readM32(unsigned long address, unsigned char* out, int count, const char* errmsg)
{
    int actual_count = 0;
    CVErrorCodes err = CAENVME_BLTReadCycle
        (BHandle, address, out, count, cvA24_U_BLT, cvD32, &actual_count);
    test_error(errmsg, err);
    return actual_count;
}

/* ######################################################################## */
/* ######################################################################## */

/* for emacs */
/*** Local Variables: ***/
/*** c-basic-offset:4 ***/
/*** indent-tabs-mode: nil ***/
/*** End: ***/
