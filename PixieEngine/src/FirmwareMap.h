//
// Created by Vetle Wegner Ingeberg on 11/02/2021.
//

#ifndef PIXIEENGINE_FIRMWAREMAP_H
#define PIXIEENGINE_FIRMWAREMAP_H

#include <map>
#include <string>

/*!
 * \brief A small wrapper to read and store firmware mapping
 */

class FirmwareMap_t {

public:
    struct key_t {
        unsigned short rev;     //! Module revision
        unsigned short bit;    //! Module ADC bits
        unsigned short mhz;     //! Module ADC sample frequency

        bool is_valid() const {
            if (!(rev == 0xb || rev == 0xc || rev == 0xd || rev == 0xf))
                return false;
            if ( !(bit == 12 || bit == 14 || bit == 16) )
                return false;
            if ( !(mhz == 100 || mhz == 250 || mhz == 500) )
                return false;
            return true;
        }
    };

    struct value_t {
        std::string comFPGA;
        std::string SPFPGA;
        std::string DSPcode;
        std::string DSPvar;
    };

private:
    typedef std::map<key_t, value_t> map_t;

    //! Member storing the mapping
    map_t fwmap;

public:

    //! Create by reading from INI file
    explicit FirmwareMap_t(const char *file);

    value_t FindFirmware(const key_t &key) const;

};

#endif //PIXIEENGINE_FIRMWAREMAP_H
