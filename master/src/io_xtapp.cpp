
#include "io_xtapp.h"

#define NDEBUG
#include "debug.h"

// ########################################################################
// ########################################################################

io_xtapp::io_xtapp(XtAppContext ac)
    : app_context(ac)
{
    DBGL;
}

// ########################################################################

io_xtapp::~io_xtapp()
{
    DBGL;
    remove_all();
}

// ########################################################################

void io_xtapp::r_callback(XtPointer user_data, int*, XtInputId* xt_id)
{
    DBGL;
    io_xtapp* thys = (io_xtapp*)user_data;

    io_channel_map::iterator it = thys->io_r_channels.find(*xt_id);
    if( it != thys->io_r_channels.end() ) {
        io_channel* c = it->second;
        if( c )
            c->handle_read();
    }
}


// ########################################################################

void io_xtapp::w_callback(XtPointer user_data, int*, XtInputId* xt_id)
{
    DBGL;
    io_xtapp* thys = (io_xtapp*)user_data;

    io_channel_map::iterator it = thys->io_w_channels.find(*xt_id);
    if( it != thys->io_w_channels.end() ) {
        io_channel* c = it->second;
        if( c )
            c->handle_write();
    }
}

// ########################################################################

void io_xtapp::update(io_channel* c, bool read, bool write)
{
    DBGL;
    io_id_map::iterator r_it = io_r_ids.find(c);
    bool was_r = ( r_it != io_r_ids.end() );
    if( read && !was_r ) {
        const XtInputId xt_id = XtAppAddInput
            (app_context, c->get_fd(), (XtPointer)XtInputReadMask, r_callback, this);
        // remember which channel belongs to which id
        io_r_channels[xt_id] = c;
        io_r_ids[c] = xt_id;
    } else if( was_r && !read ) {
        io_r_ids.erase(c);
        io_r_channels.erase(r_it->second);
        XtRemoveInput(r_it->second);
    }

    DBGL;
    io_id_map::iterator w_it = io_w_ids.find(c);
    bool was_w = ( w_it != io_w_ids.end() );
    if( write && !was_w ) {
        const XtInputId xt_id = XtAppAddInput
            (app_context, c->get_fd(), (XtPointer)XtInputWriteMask, w_callback, this);
        // remember which channel belongs to which id
        io_w_channels[xt_id] = c;
        io_w_ids[c] = xt_id;
    } else if( was_w && !write ) {
        io_w_ids.erase(c);
        io_w_channels.erase(w_it->second);
        XtRemoveInput(w_it->second);
    }
}

// ########################################################################

void io_xtapp::remove(io_channel* c)
{
    // forget about the channel, remove it from the file descriptor
    // map and from the file-descriptor lists
    DBGL;
    this->update(c, false, false);
}

// ########################################################################

void io_xtapp::remove_all()
{
    io_id_map::iterator it;
    for( it=io_r_ids.begin(); it!=io_r_ids.end(); ++it )
        XtRemoveInput(it->second);
    io_r_channels.clear();
    io_r_ids.clear();

    for( it=io_w_ids.begin(); it!=io_w_ids.end(); ++it )
        XtRemoveInput(it->second);
    io_w_channels.clear();
    io_w_ids.clear();
}
