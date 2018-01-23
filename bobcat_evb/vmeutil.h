
#ifndef vmeutil_H
#define vmeutil_H 1

#include "vlib.h"

// ########################################################################
// map VME address and check the result
void* do_vme_map(unsigned int addr, unsigned int size, unsigned int mode, char* message);

// ########################################################################
template<typename T>
void do_vme_map(T* &ptr, unsigned int addr, unsigned int mode, char* message)
{
        ptr = (T*)do_vme_map(addr,sizeof(T),mode,message);
}

// ########################################################################
template<typename T>
void do_vme_rel(T* &ptr)
{
        vme_rel((uint)ptr,sizeof(T));
}


#endif /* vmeutil_H */

/* for emacs */
/*** Local Variables: ***/
/*** c-basic-offset:8 ***/
/*** indent-tabs-mode: nil ***/
/*** mode: c++ ***/
/*** End: ***/
