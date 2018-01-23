
#ifndef evb_lecroy_1151n_H
#define evb_lecroy_1151n_H 1

typedef struct LECROY_1151N_ {
  unsigned long baseaddress;
  unsigned long box_id;
  unsigned long scalers; // bits set here indicate that the scaler should be read

  unsigned short *pRESET;
  unsigned long  *pREADbase;
} LECROY_1151N_t;

void LECROY_1151N_open(LECROY_1151N_t *lcr, unsigned long b, int boxid, unsigned long scalers);
void LECROY_1151N_close(LECROY_1151N_t *lcr);
void LECROY_1151N_data2buffer(LECROY_1151N_t *lcr);
void LECROY_1151N_clear(LECROY_1151N_t *lcr);

#endif /* evb_lecroy_1151n_H */

/* for emacs */
/*** Local Variables: ***/
/*** indent-tabs-mode: nil ***/
/*** End: ***/
