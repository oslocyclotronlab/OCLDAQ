
#ifndef evb_camac_atdc_H
#define evb_camac_atdc_H 1

typedef struct CAMAC_ATDC_ {
        unsigned long box_id;

        unsigned long slot;
        long data, status, lam;
} CAMAC_ATDC_t;

void CAMAC_ATDC_open(CAMAC_ATDC_t *cat, unsigned long slot, int boxid);
void CAMAC_ATDC_close(CAMAC_ATDC_t *cat);
void CAMAC_ATDC_config(CAMAC_ATDC_t *cat);
void CAMAC_ATDC_data2buffer(CAMAC_ATDC_t *cat);
void CAMAC_ATDC_clear(CAMAC_ATDC_t *cat);

inline unsigned long camac_data(unsigned long x) { return  x&0x0fff; }

#endif /* evb_camac_atdc_H */

/* for emacs */
/*** Local Variables: ***/
/*** c-basic-offset:8 ***/
/*** indent-tabs-mode: nil ***/
/*** End: ***/
