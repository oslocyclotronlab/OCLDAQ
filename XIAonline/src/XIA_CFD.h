#ifndef XIA_CFD_H
#define XIA_CFD_H

/*!
 * File with functions that allows for extraction of floating point
 * time information from the CFD value supplied in the data stream
 * from the internal CFD algorithm of XIA.
 */

#include <cstdint>
#include <utility>

namespace XIA {

    enum ADCSamplingFreq {
        f000MHz = 0,
        f100MHz = 100,
        f125MHz = 125,
        f250MHz = 250,
        f500MHz = 500,
    };


    typedef std::pair<double, bool> XIA_CFD_t;

    /*!
     * Calculate the CFD time
     * \param frequency detector frequency
     * \param CFDvalue event CFD readout
     * \return pair with first CFD time correction to timestamp and second CFD fail.
     */
    XIA_CFD_t XIA_CFD_Decode(const ADCSamplingFreq &frequency, const uint16_t &CFDvalue);

    /*! Calculates the fractional value for the zero
     *  crossing of a signal measured with a 100MHz
     *  XIA card. This value is found by following:
     *  tf = 10.0*(CFDvalue[14:0]/32768.0)
     *  tf is then the time in ns.
     *  CFDvalue[15] is the fail bit and indicates
     *  if the CFD value should be voided.
     */
    double XIA_CFD_Fraction_100MHz(
            const uint16_t &CFDvalue, /*!< Factional value recorded by the XIA unit.	*/
            bool &fail /*! Fail flag. Set to 1 if fail bit is 1.	    */);

    /*! Calculates the fractional value for the zero
     *  crossing of a signal measured with a 250MHz
     *  XIA card. This value is found by following:
     *  tf = 4.0*(CFDvalue[13:0]/16384.0 - CFDvalue[14])
     *  tf is then the time in ns.
     *  CFDvalue[15] is the fail bit and indicates
     *  if the CFD value should be voided.
     */
    double XIA_CFD_Fraction_250MHz(
            const uint16_t &CFDvalue, /*!< Factional value recorded by the XIA unit.	*/
            bool &fail /*!< Fail flag. Set to 1 if fail bit is 1.	    */);

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
    double XIA_CFD_Fraction_500MHz(
            const uint16_t &CFDvalue, /*!< Factional value recorded by the XIA unit. */
            bool &fail /*!< Fail flag. Set to 1 if bit 15, 14 and 13 of CFDvalue is 1. */);

}

#endif  // XIA_CFD_H