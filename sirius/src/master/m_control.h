// -*- c++ -*-

#ifndef M_CONTROL_H
#define M_CONTROL_H 1

extern void leave_prog();

extern void acq_start();
extern void acq_stop();
extern void acq_status();
extern void acq_reload();
extern void acq_clear();
extern void acq_dump();
extern void acq_storage( bool write_file );

#endif /* M_CONTROL_H */
