
#ifndef evb_tiger_net_H
#define evb_tiger_net_H 1

#include "evb_transfer.h"

//! Wait for tiger (i.e. sirius start button).
//! Returns LEAVELOOP or NEXTEVENT.
loopstate_t tiger_net_wait();


//! Transfer (passively) the buffers to tiger.
//! Returns LEAVELOOP or NEXTEVENT or WAIT4TIGER.
loopstate_t tiger_net_flush();


//! Notify tiger about the state of the VME acq.
void tiger_net_acq_running(int running);


void tiger_net_open();
void tiger_net_close();

#endif /* evb_tiger_net_H */

/* for emacs */
/*** Local Variables: ***/
/*** c-basic-offset:8 ***/
/*** indent-tabs-mode: nil ***/
/*** mode: c++ ***/
/*** End: ***/
