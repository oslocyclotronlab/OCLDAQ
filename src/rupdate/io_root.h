// -*- c++ -*-

#ifndef IO_XTAPP_H
#define IO_XTAPP_H 1

#include "net_control.h"

#include <TSysEvtHandler.h>

// ########################################################################

class io_root : public io_control {
public:
    io_root();
    ~io_root();

    void remove_all();

    virtual void update(io_channel* ic, bool read, bool write);
    virtual void remove(io_channel* ic);

private:
    class InputHandler : public TFileHandler {
        io_root& ior;
    public:
        InputHandler(io_root& i, int fd)
            : TFileHandler(fd, 1), ior(i) { }
        
        Bool_t ReadNotify()  { return ior.HandleRead(this); }
        Bool_t WriteNotify() { return ior.HandleWrite(this); }
        Bool_t Notify()      { return ior.HandleOther(this); }
    };

    typedef std::map<InputHandler*,io_channel*> io_channel_map;
    io_channel_map io_channels;

    typedef std::map<io_channel*,InputHandler*> io_handler_map;
    io_handler_map io_handlers;

    bool HandleRead(InputHandler* h);
    bool HandleWrite(InputHandler* h);
    bool HandleOther(InputHandler* h);
};

#endif /* IO_XTAPP_H */
