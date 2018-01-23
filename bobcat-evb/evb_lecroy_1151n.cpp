
#include "evb_lecroy_1151n.h"

#include "evb_buffer.h"
#include "vmeutil.h"

#include <stdio.h>
#include <unistd.h>

/* ######################################################################## */
/* ######################################################################## */

/** offsets for module registers */
typedef enum {
    oRESET     = 0x0000, // write => scaler reset
    oREADbase  = 0x0080, // read scalers 1..16
} _LECROY_1151N_offsets;

/* ######################################################################## */
/** Set-up of address pointers for Control and Data Registers  **/
void LECROY_1151N_open(LECROY_1151N_t *lcr, unsigned long b, int boxid, unsigned long scalers)
{
    lcr->baseaddress = b;
    lcr->box_id = boxid << 23;
    lcr->scalers = scalers;

    printf("Opening LeCroy 1151N VME scaler at address %08lX ...\n",
           lcr->baseaddress);
    fflush(stdout);

    do_vme_map(lcr->pRESET,  b+oRESET, AM24, "RESET");
    lcr->pREADbase = (unsigned long*)do_vme_map(b+oREADbase, 16*4, AM24, "SCALERS");
    
    printf("   DONE \n");
}

/* ######################################################################## */
/** Release of address pointers for Control and Data Registers  **/
void LECROY_1151N_close(LECROY_1151N_t *lcr)
{
    printf("Closing LeCroy 1151N VME scaler at address %08lX ...\n",
           lcr->baseaddress);
    fflush(stdout);

    do_vme_rel(lcr->pRESET);
    vme_rel((uint)lcr->pREADbase, 16*4);

    printf("   DONE\n");
}

/* ######################################################################## */
/** Copy data to buffer.
 **/
void LECROY_1151N_data2buffer(LECROY_1151N_t *lcr)
{
    for(int i=0; i<16; ++i) {
        // check if the scaler channel is to be read
        if( lcr->scalers & (1<<i) ) {
            // read scaler count, no reset
            const unsigned long lcr_count = lcr->pREADbase[i];

            // limit the data size to 16bit
            buffer_put(lcr->box_id,  i     << 16,  lcr_count     &0xFFFF);
            buffer_put(lcr->box_id, (i+16) << 16, (lcr_count>>16)&0xFFFF);
        }
    }
}

/* ######################################################################## */
/** Reset all scalers.
 **/
void LECROY_1151N_clear(LECROY_1151N_t *lcr)
{
    *lcr->pRESET = 0; // reset all scalers
}

/* ######################################################################## */
/* ######################################################################## */

/* for emacs */
/*** Local Variables: ***/
/*** indent-tabs-mode: nil ***/
/*** End: ***/
