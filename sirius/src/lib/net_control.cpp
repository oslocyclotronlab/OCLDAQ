
#include "net_control.h"

#include "utilities.h"

#include <arpa/inet.h>
#include <errno.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <netdb.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>

#include <iostream>
#include <string>

#define NDEBUG
#include "debug.h"

// ########################################################################
// ########################################################################

io_channel::io_channel(io_control& ioc_, int fd_)
    : _ioc(ioc_)
    , fd(fd_)
{
}

// ########################################################################

io_channel::~io_channel()
{
    close();
}

// ########################################################################

void io_channel::close()
{
    DBGL;
    if( fd<0 )
	return;

    ioc().remove(this);
    ::close(fd);
    fd = -1;
}

// ########################################################################
// ########################################################################

io_select::io_select()
{
    DBGL;
    FD_ZERO(&read_fd_set);
    FD_ZERO(&write_fd_set);
}

// ########################################################################

io_select::~io_select()
{
    DBGL;
}

// ########################################################################

void io_select::update(io_channel* c, bool read, bool write)
{
    DBGL;
    const int fd = c->get_fd();

    // remember which channel belongs to which file descriptor
    io_channels[fd] = c;

    // if channel can read data, add it to the read_fd_set, otherwise
    // remove it from there
    if( read )
	FD_SET(fd, &read_fd_set);
    else
	FD_CLR(fd, &read_fd_set);

    // if channel can read data, add it to the read_fd_set, otherwise
    // remove it from there
    if( write )
	FD_SET(fd, &write_fd_set);
    else
	FD_CLR(fd, &write_fd_set);
}

// ########################################################################

void io_select::remove(io_channel* c)
{
    // forget about the channel, remove it from the file descriptor
    // map and from the file-descriptor lists
    DBGL;
    const int fd = c->get_fd();
    io_channels.erase( fd );
    FD_CLR(fd, &read_fd_set);
    FD_CLR(fd, &write_fd_set);
}

// ########################################################################

bool io_select::run(timeval* timeout_p)
{
    //DBGL;
    // copy fd_set's, they will be modified in 'select'
    fd_set r_fd_set = read_fd_set, w_fd_set = write_fd_set;

#if 0 && !defined(NDEBUG)
    {   int r=0, w=0;
	for(int i = 0; i < FD_SETSIZE; ++i) {
            if(FD_ISSET(i, &r_fd_set))
		r += 1;
            if(FD_ISSET(i, &w_fd_set))
		w += 1;
	}
	DBGV(r);
	DBGV(w);
    }
#endif
    
    // check if input is available from any client, or if somebody
    // wants to connect
    int s = ::select(FD_SETSIZE, &r_fd_set, &w_fd_set, 0, timeout_p);

    if( s<0 ) {
        // all okay if interrupt/timeout, just continue; otherwise
        // it's really an error
        return ( errno == EINTR );
    }

    if( s>0 ) {
        // service all the sockets with input pending or output
        // possible
        for(int i = 0; i < FD_SETSIZE; ++i) {
            if(FD_ISSET(i, &r_fd_set)) {
		//DBGL;
		io_channel_map::iterator it = io_channels.find(i);
		if( it != io_channels.end() ) {
		    io_channel* c = io_channels.find(i)->second;
		    if( c )
			c->handle_read();
		}
            }
            if(FD_ISSET(i, &w_fd_set)) {
		//DBGL;
		io_channel_map::iterator it = io_channels.find(i);
		if( it != io_channels.end() ) {
		    io_channel* c = io_channels.find(i)->second;
		    if( c )
			c->handle_write();
		}
            }
        }
    }
    return true;
}

// ########################################################################
// ########################################################################

line_channel::line_channel(io_control& ioc_, int fd, line_callback *cb_dis,
			   line_callback *cb_hl)
    : io_channel(ioc_, fd)
    , cb_disconnected(cb_dis)
    , cb_have_line(cb_hl)
{
    DBGL;
    // we can always read data
    ioc().update(this, true, false);
}

// ########################################################################

line_channel::~line_channel()
{
    DBGL;
    delete cb_disconnected;
    delete cb_have_line;
}

// ########################################################################

/**
 * Search the input buffer for a CR or LF, and if found, copy the
 * beginning to 'line'.
 *
 * @param old_inbuf where to start searching
 */
void line_channel::check_line(size_t old_inbuf)
{
    while(true) {
        DBGV(old_inbuf);
        DBGV(escape(inbuf));

        size_t pos = inbuf.find_first_of("\r\n", old_inbuf);
        if( pos == std::string::npos )
            break;

        line = inbuf.substr(0, pos);

	size_t epos = inbuf.find_first_not_of("\r\n", pos);
        if( epos != std::string::npos )
            inbuf.erase(0, epos);
        else
            inbuf = "";

        DBGV(escape(inbuf));
        DBGV(escape(line));

	if( cb_have_line )
	    cb_have_line->run(this);
        old_inbuf = 0;
    }
}

// ########################################################################

/**
 * Send a message to the other endpoint of the connection. Actually
 * the message is buffered and then we try to flush the output buffer.
 *
 * @param message the message to send
 */
void line_channel::send(const std::string& message)
{
    DBGV(escape(message));
    outbuf.append(message);
    handle_write();
}

// ########################################################################

/**
 * Try to read some data into the input buffer, then search for a
 * line-end.
 *
 * Will call disconnect() in case of errors or EOF or if the buffer
 * gets too large.
 */
void line_channel::handle_read()
{
    DBGL;
#if 0
    const bool blocking_io = false;
    const size_t max_inbuf_size = 32768;
    while( inbuf.size() < max_inbuf_size ) {
        // fetch some data in a temporary buffer
	char tmp_buf[512];
	const int n_read = read(get_fd(), tmp_buf, sizeof(tmp_buf));

        const bool would_have_blocked = blocking_io || (n_read<0 && errno == EAGAIN);
	if( n_read == 0 || (n_read<0 && !would_have_blocked) )
            // some kind of error, or disconnection
	    break;

        const int old_size = inbuf.size();
	if( n_read > 0 )
	    inbuf.append(tmp_buf, n_read);
	if( would_have_blocked ) {
	    check_line(old_size);
	    return;
	}
    }
    // this is reached only if an error occurred
    disconnect();
#else
    // fetch some data in a temporary buffer
    char tmp_buf[512];
    const int n_read = read(get_fd(), tmp_buf, sizeof(tmp_buf));

    const bool would_have_blocked = (n_read<0 && errno == EAGAIN);
    if( n_read == 0 || (n_read<0 && !would_have_blocked) ) {
        // some kind of error, or disconnection
        disconnect();
        return;
    }
    if( n_read > 0 ) {
        const int old_size = inbuf.size();
#ifndef NDEBUG
        { const std::string received(escape(std::string(tmp_buf, n_read))); DBGV(received); }
#endif
        inbuf.append(tmp_buf, n_read);
        check_line(old_size);
    }
#endif
}

// ########################################################################

/**
 * Try to send the output buffer.
 * Will call disconnect() in case of errors or EOF.
 */
void line_channel::handle_write()
{
    DBGL;
    const size_t os = outbuf.size();
    if( os == 0 )
	return;

    int w = write(get_fd(), outbuf.data(), os);
    if( w == 0 || (w<0 && errno != EAGAIN) ) {
	disconnect();
    } else if( w > 0 ) {
#ifndef NDEBUG
        { const std::string wrote(escape(outbuf.substr(0, w))); DBGV(wrote); }
#endif
	outbuf.erase(0, w);
	ioc().update(this, true, !outbuf.empty());
    }
}

// ########################################################################

/**
 * Close the connection and invole the call-back 'cb_disconnected'.
 */
void line_channel::disconnect()
{
    DBGL;
    close();
    if( cb_disconnected )
	cb_disconnected->run(this);
}

// ########################################################################

/**
 * Utility function to connect to host:port.
 * Sets to non-blocking IO.
 *
 * @param ioc              object handling select() calls
 * @param host             host we should try to connect to
 * @param port             port we should try to connect to on host
 * @param cb_disconnected  callback for disconnection/error
 * @param cb_have_line     callback if new line is available
 */
line_channel* line_connect(io_control& ioc, const char* host, int port,
			   line_callback *cb_disconnected, line_callback *cb_have_line)
{
    DBGL;
    int sock = socket(PF_INET, SOCK_STREAM, 0);
    if(sock < 0)
	return 0;
    
    struct sockaddr_in servername;
    init_sockaddr(&servername, host, port);
	
    if(connect(sock,(struct sockaddr*)&servername, sizeof(servername))<0) {
	close(sock);
	return 0;
    }

    fcntl(sock, F_SETFL, O_NONBLOCK);
    return new line_channel(ioc, sock, cb_disconnected, cb_have_line);
}

// ########################################################################
// ########################################################################

/**
 * Initialize a basic TCP server, but WITHOUT making it 'listen'.
 *
 * @param ioc  object handling select() calls
 * @param port port we should later listen on
 * @param nam  name of the server
 */
tcp_server::tcp_server(io_control& ioc, int port, std::string const& nam)
    : io_channel(ioc, make_socket(port, true))
    , name(nam)
{
    DBGL;
}

// ########################################################################

/**
 * Finalize the server, delete'ing all it's connections.
 */
tcp_server::~tcp_server()
{
    DBGL;
    std::set<io_channel*>::iterator it;
    for(it=io_channels.begin(); it!=io_channels.end(); ++it)
	delete *it;
}
    
// ########################################################################

/**
 * Make the server listen at its port.
 *
 * @return true if listening was successfully started
 */
bool tcp_server::listen()
{
    DBGL;
    // set the socket to accept connections
    if( ::listen(get_fd(), 1) >= 0 ) {
	ioc().update(this, true, false);
	return true;
    } else {
	return false;
    }
}

// ########################################################################

/**
 * Accept a new client.
 *
 * Calls new_channel() to create the client object, and refuses
 * connection if this returns 0.
 */
void tcp_server::handle_read()
{
    DBGL;
    // connection request on original socket
    struct sockaddr_in clientname;
    socklen_t size = sizeof(clientname);

    int new_fd = accept(get_fd(), (struct sockaddr*)&clientname, &size);
    if(new_fd < 0)
        return;

    io_channel* ic = new_channel(new_fd);
    if( !ic ) {
	std::cerr << name << ": client not accepted." << std::endl;
        ::close(new_fd);
    } else {
	io_channels.insert(ic);
    }
}

// ########################################################################

/**
 * Remove an io_channel from the list, without delete'ing it.
 *
 * @param ic channel to be removed
 */
void tcp_server::remove_channel(io_channel* ic)
{
    DBGL;
    io_channels.erase(ic);
}

// ########################################################################
// ########################################################################

/* callback helpers */
static void line_server_cb_disconnected(line_channel* lc, void* user_data)
{
    line_server* ls = static_cast<line_server*>(user_data);
    ls->client_disconnected(lc);
}

static void line_server_cb_have_line(line_channel* lc, void* user_data)
{
    line_server* ls = static_cast<line_server*>(user_data);
    ls->client_has_line(lc);
}

// ########################################################################

/**
 * Initialize a server for line-wise communication.
 * 
 * Callbacks are available for connection and disconnection of
 * clients, and if a client received a new 'command' line. The server
 * will 'listen()' and the program will exit if this fails.
 * 
 * @param ioc     object handling select() calls
 * @param port    port we should later listen on
 * @param name    name of the server
 * @param cb_conn callback for new client connection
 * @param cb_dis  callback for disconnection/error
 * @param cb_hl   callback if new line is available
 */
line_server::line_server(io_control& ioc, int port, std::string const& name,
			 line_callback *cb_conn, line_callback *cb_dis, line_callback *cb_hl)
    : tcp_server(ioc, port, name)
    , cb_connected(cb_conn)
    , cb_disconnected(cb_dis)
    , cb_have_line(cb_hl)
{
    DBGL;
    if( !listen() ) {
	std::cerr << name << ": could not listen on port " << port
		  << "Exit." << std::endl;
	exit(EXIT_FAILURE);
    }
}

// ########################################################################

/**
 * Does nothing.
 */
line_server::~line_server()
{
    DBGL;
    delete cb_connected;
    delete cb_disconnected;
    delete cb_have_line;
}
    
// ########################################################################

/**
 * Called by the tcp_server baseclass if a new client has connected.
 * 
 * Calls cb_connected. Sets IO to non-blocking. Does not accept more
 * than 16 clients.
 *
 * @param fd file-descriptor for the connection to the client
 * @return an io_channel to accept, 0 to refuse
 */
io_channel* line_server::new_channel(int fd)
{
    DBGL;
    if( io_channels.size()>=16 )
	return 0;

    fcntl(fd, F_SETFL, O_NONBLOCK);
    line_channel* lc =  new line_channel(ioc(), fd,
					 new line_cb(line_server_cb_disconnected, this),
					 new line_cb(line_server_cb_have_line, this));
    if( cb_connected )
	cb_connected->run(lc);
    return lc;
}

// ########################################################################

/**
 * Send a message to all clients.
 *
 * @param message message to be broadcasted
 */
void line_server::send_all(const std::string& message)
{
    DBGL;
    std::set<io_channel*>::iterator it;
    for(it=io_channels.begin(); it!=io_channels.end(); ++it) {
	line_channel* lc = static_cast<line_channel*>( *it );
	lc->send( message );
    }
}

// ########################################################################

/*
 * Invoked if a client was disconnected.
 */
void line_server::client_disconnected(line_channel* lc)
{
    DBGL;
    remove_channel(lc);
    if( cb_disconnected )
	cb_disconnected->run(lc);
}

// ########################################################################

/*
 * Invoked if a client has received a line.
 */
void line_server::client_has_line(line_channel* lc)
{
    DBGL;
    if( cb_have_line )
	cb_have_line->run(lc);
}

// ########################################################################
// ########################################################################

/**
 * Send the buffered message.
 */
void line_sender::send()
{
    if( sent )
	return;
    
    *this << "\r";
    if( lc )
        lc->send(str());
    sent = true;
}

// ########################################################################
// ########################################################################

void command_cb::run(line_channel* lc)
{
    std::string line = lc->get_line();
    const int line_size = line.size();

    for(command* cmd = commands; cmd && cmd->name; ++cmd) {
        const std::string cmd_name = cmd->name;
        const int cmd_name_size = cmd_name.size();

	if( ( ( !  cmd->has_args && line_size == cmd_name_size )
	      || ( cmd->has_args && line_size >  cmd_name_size+1 && line[cmd_name_size]==' ' ) )
	    && line.compare(0, cmd_name_size, cmd_name)==0 )
	{
            cmd->callback(lc, line, cmd->user_data);
            return;
        }
    }

    line_sender ls(lc);
    ls << error_prefix << " Command '" << escape(line) << "' unknown. Possible commands:";
    for(command* cmd = commands; cmd && cmd->name; ++cmd) {
        if( cmd != commands )
            ls << ';';
        ls << cmd->name;
    }
    ls << '\n';
}
