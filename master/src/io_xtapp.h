// -*- c++ -*-

#ifndef IO_XTAPP_H
#define IO_XTAPP_H 1

#include "net_control.h"

#include <X11/Intrinsic.h>

// ########################################################################

class io_xtapp : public io_control {
public:
    io_xtapp(XtAppContext app_context);
    ~io_xtapp();

    void remove_all();

    virtual void update(io_channel* ic, bool read, bool write);
    virtual void remove(io_channel* ic);

private:
    typedef std::map<XtInputId,io_channel*> io_channel_map;
    io_channel_map io_r_channels, io_w_channels;

    typedef std::map<io_channel*,XtInputId> io_id_map;
    io_id_map io_r_ids, io_w_ids;

    XtAppContext app_context;

    static void r_callback(XtPointer, int*, XtInputId*);
    static void w_callback(XtPointer, int*, XtInputId*);
};



#endif /* IO_XTAPP_H */
