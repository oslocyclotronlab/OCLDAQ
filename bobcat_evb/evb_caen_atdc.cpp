
#include "evb_caen_atdc.h"

#include "evb_buffer.h"
#include "vmeutil.h"

#include <stdio.h>
#include <unistd.h>

/* ######################################################################## */
/* ######################################################################## */

/*CAEN event structure, words have 32 bits*/

inline unsigned int header(unsigned long x) { return (x&0x07000000)>>24; }
inline unsigned int  dcnts(unsigned long x) { return (x&0x00003f00)>>8;  }
inline unsigned int   chan(unsigned long x) { return (x&0x001f0000)>>16; }
inline unsigned int   data(unsigned long x) { return  x&0x00000fff;      }

/** offsets for module registers */
typedef enum {
        oOUTBUF    = 0x0000, /*Output Event Buffers*/
        oBITSET1   = 0x1006, /*Bit Set 1 Register*/
        oBITCLEAR1 = 0x1008, /*Bit Clear 1 Register*/
        oSTATUS1   = 0x100E, /*Status Register 1*/
        oCONTROL1  = 0x1010, /*Control Register 1*/
        oSTATUS2   = 0x1022, /*Status Register 2*/
        oBITSET2   = 0x1032, /*Bit Set 2 Register*/
        oBITCLEAR2 = 0x1034, /*Bit Clear 2 Register*/
        oCRASEL    = 0x103C, /*Crate Select Register */
        oEVCNTRST  = 0x1040, /*Reset event counter*/
        oTHRMEM    = 0x1080, /*Threshold memories*/

        oFSRREG    = 0x1060  /*Full Scale Range Register, only for TDC 775 */
} _CAEN_ATDC_offsets;

/* ######################################################################## */

void CAEN_TDC_show_range(CAEN_ATDC_t* cat, const char* msg)
{
        if( cat->is_tdc ) {            
                const unsigned short fsr = *cat->pFSRREG;
                const float perchannel = 8.9/fsr;
                printf("   TDC range %s: @%p, 0x%02x, %5.4f ns/ch, %6.1f ns total\n",
                       msg, cat->pFSRREG, fsr, perchannel, 3840*perchannel);
        }
}

/* ######################################################################## */
/** Set-up of address pointers for Control and Data Registers  **/
void CAEN_ATDC_open(CAEN_ATDC_t *cat, unsigned long b, int boxid, int is_tdc)
{
        cat->baseaddress = b;
        cat->box_id = boxid << 23;
        cat->is_tdc = is_tdc;

        printf("Opening %s VME box at address %08lX ...\n",
               cat->is_tdc ? "TDC 775" : "ADC 785",
               cat->baseaddress);
        fflush(stdout);

#define MAP_VME(x) do_vme_map(cat->p ## x, b+o ## x, AM32, #x);

        MAP_VME( BITSET1 );
        MAP_VME( BITCLEAR1 );
        MAP_VME( STATUS1 );
        MAP_VME( CONTROL1 );
        MAP_VME( STATUS2 );
        MAP_VME( BITSET2 );
        MAP_VME( BITCLEAR2 );
        MAP_VME( CRASEL );
        MAP_VME( EVCNTRST );

        cat->pOUTBUF = 0;
        cat->pTHRMEM = 0;

        cat->pOUTBUF = (unsigned  long*)do_vme_map(b+oOUTBUF, 4096, AM32, "OUTBUF");
        cat->pTHRMEM = (unsigned short*)do_vme_map(b+oTHRMEM,   64, AM32, "THRMEM");

        if( cat->is_tdc )
                MAP_VME( FSRREG );
        
        printf("   DONE \n");
#undef MAP_VME
}

/* ######################################################################## */
/** Release of address pointers for Control and Data Registers  **/
void CAEN_ATDC_close(CAEN_ATDC_t *cat)
{
        printf("Closing %s VME box at address %08lX ...\n",
               cat->is_tdc ? "TDC 775" : "ADC 785",
               cat->baseaddress);
        fflush(stdout);

        do_vme_rel(cat->pCONTROL1);
        do_vme_rel(cat->pSTATUS1);
        do_vme_rel(cat->pSTATUS2);
        do_vme_rel(cat->pBITSET1);
        do_vme_rel(cat->pBITCLEAR1);
        do_vme_rel(cat->pBITSET2);
        do_vme_rel(cat->pBITCLEAR2);
        do_vme_rel(cat->pEVCNTRST);
        do_vme_rel(cat->pCRASEL);

        vme_rel((uint)cat->pTHRMEM,   64);
        vme_rel((uint)cat->pOUTBUF, 4096);

        if( cat->is_tdc )
                do_vme_rel(cat->pFSRREG);
        printf("   DONE\n");
}

/* ######################################################################## */
/** Configure and initialise CAEN ADC/TDC module. */
void CAEN_ATDC_config(CAEN_ATDC_t *cat, const unsigned short *threshold)
{
        printf("Configuring %s VME box at address %08lX ...\n",
               cat->is_tdc ? "TDC 775" : "ADC 785",
               cat->baseaddress);
        fflush(stdout);
        *cat->pBITSET1 = 0x0040;                  /* start software reset by setting bit 7 in BITSET1 */
        usleep(5);
        
        *cat->pBITCLEAR1 = 0x0040;                /* finish software reset by clearing bit 7 */
        usleep(5);
        
        
        //if( cat->is_tdc ) printf("   BITSET1 before rotary address: 0x%04x\n", *cat->pBITSET1);
        *cat->pBITCLEAR1 = 0x0010;                /* select address by rotary switch */
        //if( cat->is_tdc ) printf("   BITSET1 after rotary address: 0x%04x\n", *cat->pBITSET1);
        
        // if( cat->is_tdc ) printf("   BITSET2 before clear: 0x%04x\n", *cat->pBITSET2);
        *cat->pBITCLEAR2 = 0x7fff;                /* clear everything in BITSET2
                                                     also selects common start for the TDC */
        *cat->pBITSET2   = 0x4884;                /* clear data (0x4), all trg (0x4000), auto incr (0x800),
                                                     slide enable (0x80) */
       
        // if( cat->is_tdc ) printf("   BITSET2 after settings 1: 0x%04x\n", *cat->pBITSET2);
        *cat->pBITCLEAR2 = 0x0004;                /* remove clear data bit */
        //if( cat->is_tdc ) printf("   BITSET2 after settings 2: 0x%04x\n", *cat->pBITSET2);
        
        *cat->pEVCNTRST = 0x0001;                 /* reset event counter by writing anything */
        *cat->pCRASEL   = 0x0000;                 /* VME crate no 0 */

        if( cat->is_tdc ) {
                //*cat->pFSRREG  = 0x0059;           /*409.6 ns range, 0.1 ns per channel*/
                //*cat->pFSRREG  = 0x0040;           /* 0.6 us range, 0.14 ns per channel*/
                *cat->pFSRREG  = 0x001e;           /* 1.35 us range, 0.33 ns per channel*/
                CAEN_TDC_show_range(cat, "at config");
        }
 
        /* thresholds passed as parameter */
        const unsigned short t_def = cat->is_tdc ? 8 : 7;
        for(int i=0; i<32; i++) {
                cat->pTHRMEM[i] = threshold ? threshold[i] : t_def;
                if( threshold && (threshold[i] & 0x100) )
                        printf("   Channel %d is killed.\n", i);
                // else printf("   Channel %d threshold = %d\n", i, cat->pTHRMEM[i]);

        }

        printf("   DONE \n");

        // sliding scale affects range, buffering, time scale, zero suppression, threshold
}

/* ######################################################################## */
/** Configure CAEN TDC time range. */
void CAEN_ATDC_config_timerange(CAEN_ATDC_t *cat, unsigned short timerange)
{
        if( cat->is_tdc ) {
                *cat->pFSRREG  = timerange;
                CAEN_TDC_show_range(cat, "at config_timerange");
        }
 
}

/* ######################################################################## */
/** Copy data to buffer.
 **/
void CAEN_ATDC_data2buffer(CAEN_ATDC_t *cat)
{
        if( bit(*cat->pSTATUS1,0) == 0)
                return;
        
        /* event ready */
        unsigned long cat_lword = *cat->pOUTBUF;  /* read first buffer line of ADC/TDC */
        if(header(cat_lword) != 2)                /* header begin-of-event (boe) if = 2 */
                return;
        
        const int j = dcnts(cat_lword);           /* number of valid data*/
        for(int i=0; i<j; i++) {
                cat_lword = *cat->pOUTBUF;        /* OUTBUF is auto-increment!? */
                const unsigned long cat_data = data(cat_lword); /* converted data value */
                if(cat_data <= 3839) {            /* check for overflow */
                        const unsigned long cat_chn = (chan(cat_lword) << 16); /* channel number */
                        buffer_put(cat->box_id, cat_chn, cat_data);
                }
        }
}

/* ######################################################################## */
/** Clear ADC/TDC.
 **/
void CAEN_ATDC_clear(CAEN_ATDC_t *cat)
{
        *cat->pBITSET2   = 0x0004; /* clear data and read/write pointers */
        *cat->pBITCLEAR2 = 0x0004;
}

/* ######################################################################## */
/* ######################################################################## */

/* for emacs */
/*** Local Variables: ***/
/*** c-basic-offset:8 ***/
/*** indent-tabs-mode: nil ***/
/*** End: ***/
