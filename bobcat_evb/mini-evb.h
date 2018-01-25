
#ifndef mini_evb_H
#define mini_evb_H 1

extern char leaveloop;

/// returns a non-zero value if the acq shall be interrupted
inline int check_interrupt()
{
    return leaveloop == 'y';
}

#endif /* mini_evb_H */

/* for emacs */
/*** Local Variables: ***/
/*** indent-tabs-mode: nil ***/
/*** mode: c++ ***/
/*** End: ***/
