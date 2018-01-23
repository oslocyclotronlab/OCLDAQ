
#ifndef evb_tiger_sbs_H
#define evb_tiger_sbs_H 1

#include "evb_transfer.h"

//! Wait for tiger (i.e. sirius start button).
//! Returns LEAVELOOP or NEXTEVENT.
loopstate_t tiger_sbs_wait();


//! Check if tiger is listening.
//! Returns LEAVELOOP or NEXTEVENT.
bool tiger_sbs_is_listening();


//! Transfer (passively) the buffers to tiger.
//! Returns LEAVELOOP or NEXTEVENT or WAIT4TIGER.
loopstate_t tiger_sbs_flush();


//! Notify tiger about the state of the VME acq.
void tiger_sbs_acq_running(int running);


void tiger_sbs_open();
void tiger_sbs_close();

#endif /* evb_tiger_sbs_H */

/* for emacs */
/*** Local Variables: ***/
/*** c-basic-offset:8 ***/
/*** indent-tabs-mode: nil ***/
/*** mode: c++ ***/
/*** End: ***/
