
#include "evb_mesytec_atdc.h"

#include "evb_buffer.h"
#include "vmeutil.h"

#include <stdio.h>
#include <unistd.h>

/* ######################################################################## */
/* ######################################################################## */

/* mesytec MADC32 event structure, words have 32 bits*/

inline unsigned int header(unsigned long x) { return (x&0xFF000000)>>24; }
inline unsigned int  dcnts(unsigned long x) { return  x&0x00000fff;      }

inline unsigned int   chan(unsigned long x) { return (x&0x001f0000)>>16; }
inline unsigned int   data(unsigned long x) { return  x&0x00001fff;      }

/** offsets for module registers */
typedef enum {
        oOUTBUF             = 0x0000, /*...  :  x*D32 output buffers   */
        oTHRMEM             = 0x4000, /*...3E: 32*D16 threshold values */

        oBUFFER_DATA_LENGTH = 0x6030,
        oDATA_LEN_FORMAT    = 0x6032,
        oREADOUT_RESET      = 0x6034,
        oMULTIEVENT         = 0x6036,
        oMARKING_TYPE       = 0x6038,
        oSTART_ACQ          = 0x603A,
        oFIFO_RESET         = 0x603C,
        oDATA_READY         = 0x603E,
        oBANK_OPERATION     = 0x6040,
        oADC_RESOLUTION     = 0x6042,
        oINPUT_RANGE        = 0x6060,
        oNIM_GAT1_OSC       = 0x606A,

} _MESYTEC_ATDC_offsets;

/* ######################################################################## */
/** Set-up of address pointers for Control and Data Registers  **/
void MESYTEC_ATDC_open(MESYTEC_ATDC_t *cat, unsigned long b, int boxid)
{
        cat->baseaddress = b;
        cat->box_id = boxid << 23;

        printf("Opening MADC32 VME box at address %08lX ...\n",
               cat->baseaddress);
        fflush(stdout);

#define MAP_VME(x) do_vme_map(cat->p ## x, b+o ## x, AM32, #x);

        MAP_VME( BUFFER_DATA_LENGTH );
        MAP_VME( DATA_LEN_FORMAT );
        MAP_VME( READOUT_RESET );
        MAP_VME( MULTIEVENT );
        MAP_VME( MARKING_TYPE );
        MAP_VME( START_ACQ );
        MAP_VME( FIFO_RESET );
        MAP_VME( DATA_READY );
        MAP_VME( BANK_OPERATION );
        MAP_VME( ADC_RESOLUTION );
        MAP_VME( INPUT_RANGE );
        MAP_VME( NIM_GAT1_OSC );

        cat->pOUTBUF = 0;
        cat->pTHRMEM = 0;

        cat->pOUTBUF = (unsigned  long*)do_vme_map(b+oOUTBUF, 4096, AM32, "OUTBUF");
        cat->pTHRMEM = (unsigned short*)do_vme_map(b+oTHRMEM,   64, AM32, "THRMEM");

        printf("   DONE \n");
#undef MAP_VME
}

/* ######################################################################## */
/** Release of address pointers for Control and Data Registers  **/
void MESYTEC_ATDC_close(MESYTEC_ATDC_t *cat)
{
        printf("Closing MADC32 VME box at address %08lX ...\n",
               cat->baseaddress);
        fflush(stdout);

        do_vme_rel(cat->pBUFFER_DATA_LENGTH);
        do_vme_rel(cat->pDATA_LEN_FORMAT);
        do_vme_rel(cat->pREADOUT_RESET);
        do_vme_rel(cat->pMULTIEVENT);
        do_vme_rel(cat->pMARKING_TYPE);
        do_vme_rel(cat->pSTART_ACQ);
        do_vme_rel(cat->pFIFO_RESET);
        do_vme_rel(cat->pDATA_READY);
        do_vme_rel(cat->pBANK_OPERATION);
        do_vme_rel(cat->pADC_RESOLUTION);
        do_vme_rel(cat->pINPUT_RANGE);
        do_vme_rel(cat->pNIM_GAT1_OSC);

        vme_rel((uint)cat->pTHRMEM,   64);
        vme_rel((uint)cat->pOUTBUF, 4096);

        printf("   DONE\n");
}

/* ######################################################################## */
/** Configure and initialise Mesytec ADC module (for Delta E and NaI). */
void MESYTEC_ATDC_config(MESYTEC_ATDC_t *cat, const unsigned short *threshold)
{
        printf("Configuring MADC32 VME box at address %08lX ...\n",
               cat->baseaddress);
        fflush(stdout);

        *cat->pSTART_ACQ = 0;         /* ??? start acquistition -- we send a gate via cable */
        *cat->pFIFO_RESET = 1;        /* initialise FIFO */
        *cat->pMULTIEVENT = 4;        /* enable single-event mode, eob at end */
        *cat->pMARKING_TYPE = 0;      /* store event counter in end-of-event */
        *cat->pDATA_LEN_FORMAT = 2;   /* use 32bit data format */
        *cat->pBANK_OPERATION = 0;    /* banks connected */
        
        *cat->pADC_RESOLUTION = 1;    /* 4k low-res (see manual page 9) */
        // *cat->pADC_RESOLUTION = 3;    /* 8k low-res (see manual page 9) */
        *cat->pINPUT_RANGE = 1;       /* 10V input range (1), 8V (2) or 4V (0) */
        *cat->pNIM_GAT1_OSC = 0;      /* 2nd LEMO from top is gate1 */
        
        /* thresholds passed as parameter */
        const unsigned short thr_default = 35;
        for(int i=0; i<32; i++) {
                cat->pTHRMEM[i] = threshold ? threshold[i] : thr_default;
                if( threshold && (threshold[i] == 0x1FFF ) )
                        printf("   Channel %d is killed.\n", i);
        }

        *cat->pSTART_ACQ = 1;
        printf("   DONE \n");
}

/* ######################################################################## */
/** Configure and initialise Mesytec ADC module for Ge, 8k. */
void MESYTEC_ATDC_config_8k(MESYTEC_ATDC_t *cat, const unsigned short *threshold)
{
    printf("Configuring MADC32 VME box (Ge, 8k) at address %08lX ...\n",
           cat->baseaddress);
    fflush(stdout);
    
    *cat->pSTART_ACQ = 0;         /* ??? start acquistition -- we send a gate via cable */
    *cat->pFIFO_RESET = 1;        /* initialise FIFO */
    *cat->pMULTIEVENT = 4;        /* enable single-event mode, eob at end */
    *cat->pMARKING_TYPE = 0;      /* store event counter in end-of-event */
    *cat->pDATA_LEN_FORMAT = 2;   /* use 32bit data format */
    *cat->pBANK_OPERATION = 0;    /* banks connected */
    
    *cat->pADC_RESOLUTION = 3;    /* 8k low-res (see manual page 9) */
    *cat->pINPUT_RANGE = 1;       /* 10V input range (1), 8V (2) or 4V (0) */
    *cat->pNIM_GAT1_OSC = 0;      /* 2nd LEMO from top is gate1 */
    
    /* thresholds passed as parameter */
    const unsigned short thr_default = 35;
    for(int i=0; i<32; i++) {
        cat->pTHRMEM[i] = threshold ? threshold[i] : thr_default;
        if( threshold && (threshold[i] == 0x1FFF ) )
            printf("   Channel %d is killed.\n", i);
    }
    
    *cat->pSTART_ACQ = 1;
    printf("   DONE \n");
}


/* ######################################################################## */
/** Copy data to buffer.
 **/
void MESYTEC_ATDC_data2buffer(MESYTEC_ATDC_t *cat)
{
        if( *cat->pDATA_READY == 0)
                return;
 
        /* event ready */
        const unsigned long cat_hword = *cat->pOUTBUF; /* read header word from ADC */
        if(header(cat_hword) != 0x40) {                /* check for header signature */
                printf("bad hdr\n");
                return;
        }
        const int j = dcnts(cat_hword);            /* number of valid data, including end-of-event */
//        bool have = false;
//        printf("j=%2d ", j);
        for(int i=1; i<j; i++) {                   /* read data, but not end-of-event */
                unsigned long cat_lword = *cat->pOUTBUF;
//                printf(" %08lX", cat_lword);
                if( (cat_lword & 0x4000)==0 && header(cat_lword)==0x04 ) { /* skip overflow values, only look at data */
                        const unsigned long cat_data = data(cat_lword); /* converted data value */
                        const unsigned long chn = chan(cat_lword);
                        const unsigned long cat_chn = (chn << 16); /* channel number */
//                        if( ch == 0xe || ch == 0xf ) {
//                                have = true;
//                                printf("i=%2d lword=%08lX data=%04lX chn=%2ld\n", i, cat_lword, cat_data, cat_chn>>16);
//                        }
                        buffer_put(cat->box_id, cat_chn, cat_data);
                }
        }
//       printf("\n");
        // read end-of-buffer word
        //const unsigned long cat_eword = *cat->pOUTBUF; /* read header word from ADC */
        //if( (header(cat_eword)&0xf0) != 0xc0) {                /* check for header signature */
        //        printf("bad eoe\n");
        //}
//        if( have )
//                printf("n=%d lword=%08lX\n\n", j, cat_hword);
}

/* ######################################################################## */
/** Clear ADC/TDC.
 **/
void MESYTEC_ATDC_clear(MESYTEC_ATDC_t *cat)
{
        *cat->pREADOUT_RESET = 1; /* reset readout, FIFO now can take events */
}

/* ######################################################################## */
/* ######################################################################## */

/* for emacs */
/*** Local Variables: ***/
/*** c-basic-offset:8 ***/
/*** indent-tabs-mode: nil ***/
/*** End: ***/
