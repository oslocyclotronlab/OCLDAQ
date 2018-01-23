
#include "vmeutil.h"

#include <stdio.h>

// ########################################################################
// map VME address and check the result
void* do_vme_map(unsigned int addr, unsigned int size, unsigned int mode, char* message)
{
        void* p = (void*)vme_map(addr, size, mode);
        if( p==0 ) {
                fprintf(stderr,"Unable to map VME address %s\n", message);
                exit(0);
        }
        return p;
}

/* for emacs */
/*** Local Variables: ***/
/*** c-basic-offset:8 ***/
/*** indent-tabs-mode: nil ***/
/*** End: ***/
