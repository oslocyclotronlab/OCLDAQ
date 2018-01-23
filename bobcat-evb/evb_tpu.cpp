
#include "evb_tpu.h"

#include "evb_buffer.h"
#include "vmeutil.h"

#include <stdio.h>

/* ######################################################################## */
/* ######################################################################## */

/** offsets for module registers */
typedef enum {
        oSTATUS  = 0,
        oNUMREG  = 5,
        oPATTERN = 6,
        oNEXTREG = 9
} _UiO_TPU_offsets;

// ########################################################################
void UiO_TPU_open(UiO_TPU_t *ut, unsigned long base, int boxid)
{
        printf("Opening TPU at 0x%08lx...\n", base); fflush(stdout);
        ut->box_id = boxid<<23;
        do_vme_map(ut->pSTATUS,  base+oSTATUS,  AM24, "TPU STATUS register");
        do_vme_map(ut->pNUMREG,  base+oNUMREG,  AM24, "TPU NUMREG");
        do_vme_map(ut->pPATTERN, base+oPATTERN, AM24, "TPU PATTERN register");
        do_vme_map(ut->pNEXTREG, base+oNEXTREG, AM24, "TPU NEXTREG");
        printf("   DONE \n");
}

// ########################################################################

void UiO_TPU_clear(UiO_TPU_t *ut)
{
        // reset TPU before running
        *ut->pSTATUS = 0;
}

// ########################################################################

void UiO_TPU_close(UiO_TPU_t *ut)
{
        printf("Closing TPU ...\n"); fflush(stdout);
        do_vme_rel(ut->pSTATUS);
        do_vme_rel(ut->pNUMREG);
        do_vme_rel(ut->pPATTERN);
        do_vme_rel(ut->pNEXTREG);
        printf("   DONE \n");
}

/* ######################################################################## */
/* ######################################################################## */

/* for emacs */
/*** Local Variables: ***/
/*** c-basic-offset:8 ***/
/*** indent-tabs-mode: nil ***/
/*** End: ***/
