#ifndef XIA_CFD_H
#define XIA_CFD_H

/*!
 * File with functions that allows for extraction of floating point
 * time information from the CFD value supplied in the data stream
 * from the internal CFD algorithm of XIA.
 */

// We should probably compile it as C and not C++,
// as C should, in principle be faster.
#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

#include <stdint.h>

/*! Calculates the fractional value for the zero
 *  crossing of a signal measured with a 100MHz
 *  XIA card. This value is found by following:
 *  tf = 10.0*(CFDvalue[14:0]/32768.0)
 *  tf is then the time in ns.
 *  CFDvalue[15] is the fail bit and indicates
 *  if the CFD value should be voided.
 */
double XIA_CFD_Fraction_100MHz(uint16_t CFDvalue,	/*!< Factional value recorded by the XIA unit.	*/
                               char* fail           /*! Fail flag. Set to 1 if fail bit is 1.	    */);

/*! Calculates the fractional value for the zero
 *  crossing of a signal measured with a 250MHz
 *  XIA card. This value is found by following:
 *  tf = 4.0*(CFDvalue[13:0]/16384.0 - CFDvalue[14])
 *  tf is then the time in ns.
 *  CFDvalue[15] is the fail bit and indicates
 *  if the CFD value should be voided.
 */
double XIA_CFD_Fraction_250MHz(uint16_t CFDvalue,	/*!< Factional value recorded by the XIA unit.	*/
                               char* fail           /*!< Fail flag. Set to 1 if fail bit is 1.	    */);

/*! Calculates the fractional value for the zero
 *  crossing of a signal measured with a 500MHz
 *  XIA card. This value is found by following:
 *  tf = 2.0*(CFDvalue[12:0]/8192.0 - CFDvalue[15:13] - 1.0)
 *  tf is then the time in ns.
 *  The CFDvalue of the 500MHz does not have a spesific
 *  fail bit, but the trigger is forced (ie. no ZC fraction is found)
 *  whenever both bit 15, 14 and 13 of CFDvalue are 1. Thus, having
 *  the same result as the fail bit in the other models.
 */
double XIA_CFD_Fraction_500MHz(uint16_t CFDvalue,   /*!< Factional value recorded by the XIA unit.						*/
                               char* fail           /*!< Fail flag. Set to 1 if bit 15, 14 and 13 of CFDvalue is 1.		*/);




////// Warning: functions bellow are not able to privide sufficient resolution, due to rounding errors.
////// If you intend to subtract two timestamps with CFD correction, then you should do it as following:
////// 1. Get the CFD correction for each of the timestamps and store this number separate from the
//////    FPGA timestamp.
////// 2. Calculate a "course" time difference (difference between FPGA t-stamps). Since the FPGA timestamp of the two
//////    pulses should be of the same order, the difference should be (tick units) of maximally a few
//////    thousand units (which is far less then the size of the numbers, which can be in the order of 10^14).
////// 3. Calculate a "fine" time difference (difference between CFD corrections). The corrections obtained
//////    from the CFD is a floating-point number and can allways be represented exactly as long as the data
//////    type used for storing this number have a sufficient number of bits (this is because the denominator
//////    are powers of 2 and any such fraction can be represented exactly with a finite number of bits).
////// 4. The difference found in 2 and 3 can now be added together (remember to multiply the difference found
//////    in 2 with the right factor to give ns). These two number should be within a 0-3 orders of magnitude of
//////    each other, thus no numerical errors should occure.

/*!
 *  Combines the timestamp and the fractional time to give the full timestamp in ns.
 *  The time is calculated by the following:
 *  t = 10.0*timestamp + XIA_CFD_Fraction_100MHz(CFDvalue)
 *  In case of the fail bit being set, a random floating-point number between 0 and 10 is choosen for
 *  the fractional time.
 */
double XIA_time_in_ns_100MHz(int64_t timestamp,	/*!< Course FPGA timestamp					*/
                             uint16_t CFDvalue	/*!< Fractional value found by the XIA unit	*/);

/*!
 *  Combines the timestamp and the fractional time to give the full timestamp in ns.
 *  The time is calculated by the following:
 *  t = 8.0*timestamp + XIA_CFD_Fraction_250MHz(CFDvalue)
 *  In case of the fail bit being set, a random floating-point number between 1 and 8 is choosen for
 *  the fractional time.
 */
double XIA_time_in_ns_250MHz(int64_t timestamp,	/*!< Course FPGA timestamp					*/
                             uint16_t CFDvalue	/*!< Fractional value found by the XIA unit	*/);

/*!
 *  Combines the timestamp and the fractional time to give the full timestamp in ns.
 *  The time is calculated by the following:
 *  t = 2.0*timestamp + XIA_CFD_Fraction_500MHz(CFDvalue)
 *  In case of the 15th, 14th and 13th bit in CFDvalue both being 1, a random floating-point number
 *  between 0 and 10 is choosen for the fractional time.
 */
double XIA_time_in_ns_500MHz(int64_t timestamp,	/*!< Course FPGA timestamp					*/
                             uint16_t CFDvalue	/*!< Fractional value found by the XIA unit	*/);


#ifdef __cplusplus
}
#endif // __cplusplus
#endif // XIA_CFD_H
