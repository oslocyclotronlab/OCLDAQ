
#include "evb_camac_atdc.h"

/*Silena CAMAC event structure, words have 16 bits*/
inline unsigned long  hw(unsigned long x) { return (x&0x8000)>>15; }
inline unsigned long vdc(unsigned long x) { return (x&0x0f00)>>8; }
inline unsigned long sub(unsigned long x) { return (x&0x7000)>>12; }
/*inline unsigned long dtw(unsigned long x) { return  x&0x0fff; }*/

static const long cBRANCH = 1; /* rotary switch on CAMAC branch driver */
static const long cCRATE  = 1; /* rotary switch on CAMAC crate controller */

/* ######################################################################## */

static int cam_open = 0;

/* ######################################################################## */
/** Set-up of address pointers for Control and Data Registers  **/
void CAMAC_ATDC_open(CAMAC_ATDC_t *cam, unsigned long slot, int boxid)
{
        printf("Opening CAMAC box in slot %ld ...", slot);
        fflush(stdout);

        cam->box_id = boxid << 23;
        cam->slot = slot;

        printf("DONE \n");
}

/* ######################################################################## */
/** Release of address pointers for Control and Data Registers  **/
void CAMAC_ATDC_close(CAMAC_ATDC_t *cam)
{
        printf("Closing CAMAC box in slot %ld ...", cam->slot);
        fflush(stdout);

        cam_open -= 1;
        if(cam_open == 0 )
                cclose();
        
        printf("DONE\n");
}

/* ######################################################################## */
/** Configure and initialise CAEN ADC/TDC module. */
void CAMAC_ATDC_config(CAMAC_ATDC_t *cam)
{
        printf("Configuring CAMAC box in slot %ld ...", cam->slot);
        fflush(stdout);

        if(cam_open == 0 )
                copen();

        long idmy[2] = { 1, 0 };

        cdreg(&cam->data,   cBRANCH, cCRATE, cam->slot,  0); /* CAMAC ADC/TDC data register */
        cdreg(&cam->status, cBRANCH, cCRATE, cam->slot, 14); /* status register */
        cdlam(&cam->lam,    cBRANCH, cCRATE, cam->slot,  0, idmy); /* define LAM source */

        if(cam_open == 0 ) {
                cccz(cam->data);                  /* initialize crate */
                ccci(cam->data, 0);               /* clear crate inhibit flag */
        }
        cam_open += 1;


        long q;
        unsigned short statw = 0x7800;
        cssa(20, cam->status, &statw, &q);
        if(q == 0)
                printf(" -- no Q-resp. -- ");
        
        printf("DONE \n");
}

/* ######################################################################## */
/** Copy data to buffer.
 **/
void CAMAC_ATDC_data2buffer(CAMAC_ATDC_t *cam)
{
        long q;
        ctlm(cam->lam, &q);
        if(q == 0)
                return;

        /*Event ready, Look-At-Me (LAM) is set*/
        unsigned short s_word;
        int f = 0;
        cssa(f, cam->data, &s_word, &q); /*Read first buffer line of ADC*/
        if(hw(s_word) != 1)
                return;

        /*Header word if bit 15 set*/
        const int j =  vdc(s_word); /*Number of valid data following pattern*/
        cssa(f, cam->data, &s_word, &q); /*Read dummy buffer line with pattern (not used)*/
        for(int i=0; i<j; i++){
                cssa(f, cam->data, &s_word, &q);
                const unsigned long chn = (sub(s_word) << 16); /*Channel number*/
                unsigned long dta = camac_data(s_word); /*Converted data value*/
                if(dta > 3839)
                        dta = 0;
                buffer_put(cam->box_id, chn, dta);
        }
}

/* ######################################################################## */
/** Clear ADC/TDC.
 **/
void CAMAC_ATDC_clear(CAMAC_ATDC_t *cam)
{
        cccc(cam->data);                          /* reset CAMAC for next event*/
        cclc(cam->lam);                           /* clear all CAMAC LAMs, maybe not neccessary */
}

/* ######################################################################## */
/* ######################################################################## */

/* for emacs */
/*** Local Variables: ***/
/*** c-basic-offset:8 ***/
/*** indent-tabs-mode: nil ***/
/*** End: ***/
