
#include "io_root.h"

#include <TSystem.h>

#define NDEBUG
#include "debug.h"

// ########################################################################
// ########################################################################

io_root::io_root()
{
    DBGL;
}

// ########################################################################

io_root::~io_root()
{
    DBGL;
    remove_all();
}

// ########################################################################

bool io_root::HandleRead(InputHandler* h)
{
    DBGL;
    io_channel_map::iterator it = io_channels.find(h);
    if( it != io_channels.end() ) {
        io_channel* c = it->second;
        if( c )
            c->handle_read();
    }
    return true;
}

// ########################################################################

bool io_root::HandleWrite(InputHandler* h)
{
    DBGL;
    io_channel_map::iterator it = io_channels.find(h);
    if( it != io_channels.end() ) {
        io_channel* c = it->second;
        if( c )
            c->handle_write();
    }
    return true;
}

// ########################################################################

bool io_root::HandleOther(InputHandler*)
{
    DBGL;
    return true;
}

// ########################################################################

void io_root::update(io_channel* c, bool read, bool write)
{
    DBGL;
    io_handler_map::iterator it = io_handlers.find(c);
    bool was_listed = ( it != io_handlers.end() );
    if( (read || write) && !was_listed ) {
        DBGL;
        InputHandler* h = new InputHandler(*this, c->get_fd());
        io_handlers[c] = h;
        io_channels[h] = c;
        gSystem->AddFileHandler( h );
    } else if( !(read || write) && was_listed ) {
        DBGL;
        gSystem->RemoveFileHandler( it->second );
        io_handlers.erase(c);
        io_channels.erase(it->second);
    }
}

// ########################################################################

void io_root::remove(io_channel* c)
{
    // forget about the channel, remove it from the file descriptor
    // map and from the file-descriptor lists
    DBGL;
    this->update(c, false, false);
}

// ########################################################################

void io_root::remove_all()
{
    DBGL;
    io_handler_map::iterator it;
    for( it=io_handlers.begin(); it!=io_handlers.end(); ++it )
        gSystem->RemoveFileHandler( it->second );
    io_channels.clear();
    io_handlers.clear();
}
