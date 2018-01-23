
#ifndef evb_caen_atdc_H
#define evb_caen_atdc_H 1

typedef struct CAEN_ATDC_ {
        unsigned long baseaddress;
        unsigned long box_id;
        int is_tdc;

        unsigned short *pCONTROL1, *pSTATUS1, *pSTATUS2, *pBITSET1, *pBITSET2;
        unsigned short *pBITCLEAR1, *pBITCLEAR2, *pEVCNTRST, *pCRASEL, *pTHRMEM;
        unsigned short *pFSRREG; /* only TDC 775 */
        unsigned long  *pOUTBUF;
} CAEN_ATDC_t;

void CAEN_ATDC_open(CAEN_ATDC_t *cat, unsigned long b, int boxid, int is_tdc);
void CAEN_ATDC_close(CAEN_ATDC_t *cat);
void CAEN_ATDC_config(CAEN_ATDC_t *cat, const unsigned short *threshold=0);
void CAEN_ATDC_config_timerange(CAEN_ATDC_t *cat, unsigned short timerange);
void CAEN_ATDC_data2buffer(CAEN_ATDC_t *cat);
void CAEN_ATDC_clear(CAEN_ATDC_t *cat);

//void CAEN_TDC_show_range(CAEN_ATDC_t *cat, const char* msg);

#endif /* evb_caen_atdc_H */

/* for emacs */
/*** Local Variables: ***/
/*** c-basic-offset:8 ***/
/*** indent-tabs-mode: nil ***/
/*** End: ***/
