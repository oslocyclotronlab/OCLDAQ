
#ifndef evb_mesytec_atdc_H
#define evb_mesytec_atdc_H 1

typedef struct MESYTEC_ATDC_ {
        unsigned long baseaddress;
        unsigned long box_id;

        unsigned short *pBUFFER_DATA_LENGTH;
        unsigned short *pDATA_LEN_FORMAT;
        unsigned short *pREADOUT_RESET;
        unsigned short *pMULTIEVENT;
        unsigned short *pMARKING_TYPE;
        unsigned short *pSTART_ACQ;
        unsigned short *pFIFO_RESET;
        unsigned short *pDATA_READY;
        unsigned short *pBANK_OPERATION;
        unsigned short *pADC_RESOLUTION;
        unsigned short *pINPUT_RANGE;
        unsigned short *pNIM_GAT1_OSC;

        unsigned short *pTHRMEM;
        unsigned long  *pOUTBUF;
} MESYTEC_ATDC_t;

void MESYTEC_ATDC_open(MESYTEC_ATDC_t *cat, unsigned long b, int boxid);
void MESYTEC_ATDC_close(MESYTEC_ATDC_t *cat);
void MESYTEC_ATDC_config(MESYTEC_ATDC_t *cat, const unsigned short *threshold=0);
void MESYTEC_ATDC_config_8k(MESYTEC_ATDC_t *cat, const unsigned short *threshold=0);
void MESYTEC_ATDC_data2buffer(MESYTEC_ATDC_t *cat);
void MESYTEC_ATDC_clear(MESYTEC_ATDC_t *cat);

//void CAEN_TDC_show_range(MESYTEC_ATDC_t *cat, const char* msg);

#endif /* evb_mesytec_atdc_H */

/* for emacs */
/*** Local Variables: ***/
/*** c-basic-offset:8 ***/
/*** indent-tabs-mode: nil ***/
/*** End: ***/
