// -*- c++ -*-

#ifndef ACQ_GUI_H
#define ACQ_GUI_H 1

#include <X11/Intrinsic.h>

extern XtAppContext app_context;
int gui_setup(int& argc, char* argv[]);
void gui_update_state();


#endif /* ACQ_GUI_H */
