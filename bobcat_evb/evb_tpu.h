
#ifndef evb_tpu_H
#define evb_tpu_H 1

#include "evb_buffer.h"

typedef struct UiO_TPU_ {
        unsigned long box_id;

        unsigned short *pSTATUS, *pPATTERN;
        unsigned char  *pNUMREG, *pNEXTREG;
} UiO_TPU_t;

/// allocate VME resources for a TPU unit
void UiO_TPU_open(UiO_TPU_t *ut, unsigned long base, int boxid);

/// release VME resources
void UiO_TPU_close(UiO_TPU_t *ut);

/// clear the status register
void UiO_TPU_clear(UiO_TPU_t *ut);


/// store the trigger pattern in the buffer
inline void UiO_TPU_data2buffer(UiO_TPU_t *ut)
{
        buffer_put(ut->box_id, 0, *ut->pPATTERN);
}


/// read the TPU numreg
inline unsigned char UiO_TPU_numreg(UiO_TPU_t *ut)
{
        return *ut->pNUMREG;
}


/// make TPU ready for the next event
inline void UiO_TPU_nextev(UiO_TPU_t *ut)
{
        // any access would suffice
        *ut->pNEXTREG = 0;
}

#endif /* evb_tpu_H */

/* for emacs */
/*** Local Variables: ***/
/*** c-basic-offset:8 ***/
/*** indent-tabs-mode: nil ***/
/*** mode: c++ ***/
/*** End: ***/
