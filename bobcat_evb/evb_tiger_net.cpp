
#include "evb_tiger_net.h"

#include "mini-evb.h"
#include "evb_buffer.h"

#include <stdio.h>
#include <errno.h>
#include <fcntl.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
//#include <sys/socket.h>
#include <socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <string.h>
#include <signal.h>

static const char* hostname = "192.168.0.33";
//static const char* hostname = "127.0.0.1";
static const int   hostport = 41066;

static unsigned long* net_buffer = 0;
static const int MAXBUF_char = MAXBUF*sizeof(net_buffer[0]);

static int closed = 0;

static int sock = -1;

// ########################################################################

static void init_sockaddr(struct sockaddr_in *name, const char *hostname, uint16_t port)
{
    struct hostent *hostinfo = gethostbyname(const_cast<char*>(hostname));
    if( !hostinfo ) {
        fprintf(stderr, "Unknown host %s.\n", hostname);
        exit(EXIT_FAILURE);
    }

    name->sin_addr = *(struct in_addr *) hostinfo->h_addr;
    name->sin_family = AF_INET;
    name->sin_port = htons(port);
}

// ########################################################################

static int write_all(int filedes, char* data, int datalen)
{
    int written = 0;
    while( written < datalen ) {
	if( closed != 0 )
	    return -1;
        int nbytes = write(filedes, data+written, datalen-written);
	if(nbytes < 0) {
            switch(errno) {
            case EPIPE:
            case ECONNRESET:
                return -1;
                break;
            case EINTR:
		if( !check_interrupt() )
		    return -1;
                continue;
            default:
                printf("error: %d %d %d\n", errno, EPIPE, EINTR);
                perror("write");
                exit(EXIT_FAILURE);
            }
        } else if( nbytes==0 ) {
            return -1;
        } else {
            written += nbytes;
        }
    }
    return 0;
}

// ########################################################################

static int read_all(int filedes, char* data, int bufsize)
{
    int read_so_far = 0;
    while( read_so_far<bufsize ) {
	if( closed != 0 )
	    return -1;
        const int nbytes = read(filedes, data+read_so_far, bufsize-read_so_far);
        if( nbytes < 0 ) {
            switch(errno) {
            case EINTR:
                continue;
            case EPIPE:
            case ECONNRESET:
                return -1;
                break;
            default:
                /* Read error. */
                perror("read");
                exit(EXIT_FAILURE);
            }
        } else if(nbytes == 0) {
            /* End-of-file. */
            return -1;
        } else {
            read_so_far += nbytes;
        }
    }
    return 0;
}

// ########################################################################

static void pipe_handler(int sig_num)
{
    if( sig_num != SIGPIPE )
	return;
    closed = 1;
}

// ########################################################################

static void close_socket()
{
    if( sock>=0 ) {
        close(sock);
        sock = -1;
    }
}

// ########################################################################

static int make_connection(const char* hostname, int hostport)
{
    printf("Connecting to %s:%d...", hostname, hostport);
    fflush(stdout);
    
    while( !check_interrupt() ) {
	// create the socket.
	int sock = socket(PF_INET, SOCK_STREAM, 0);
	if(sock < 0) {
	    perror("eventbuilder: could not create socket");
	    return -1;
	}
	closed = 0;

	/* Connect to the server. */
	struct sockaddr_in servername;
	init_sockaddr(&servername, hostname, hostport);
	
	while(0 > connect(sock,(struct sockaddr*)&servername, sizeof(servername))) {
	    switch( errno ) {
	    case ETIMEDOUT:
	    case EADDRNOTAVAIL:
	    case ECONNREFUSED:
		printf(".");
		fflush(stdout);
		sleep(1);
		continue;
	    default:
                perror("eventbuilder: could not connect");
		return -1;
	    }
	}
	
	char buf[8];
	const int read_fail = read_all(sock, buf, 8);
	const int cmp = strncmp("HELLOBOB", buf, 8);

	if( !closed && !read_fail && !cmp ) {
	    printf(" done\n");
	    return sock;
	}
        close(sock);

	printf(":");
	fflush(stdout);
	sleep(1);
    }
    return -1;
}

// ########################################################################
// ########################################################################

//! Wait for tiger (i.e. sirius start button).
//! Returns LEAVELOOP or NEXTEVENT.
loopstate_t tiger_net_wait()
{
    if( sock < 0 )
        sock = make_connection(hostname, hostport);

    if( sock==-1 )
        return LEAVELOOP;
    else
        return NEXTEVENT;
}

// ########################################################################
//! Transfer (passively) the buffers to tiger.
//! Returns LEAVELOOP or NEXTEVENT or WAIT4ENGINE.
loopstate_t tiger_net_flush()
{
    buffer_copy(0, MAXBUF, net_buffer); // XXX inefficient, no need to copy

    if( !write_all(sock, (char*)net_buffer, MAXBUF_char) ) {
        return NEXTEVENT;
    } else if( check_interrupt() ) {
        return LEAVELOOP;
    } else {
        close_socket();
        printf("Disconnected.\n");
        return WAIT4ENGINE;
    }
}

// ########################################################################

void tiger_net_open()
{
    signal(SIGPIPE, pipe_handler);
    net_buffer = new unsigned long[MAXBUF];
    sock = -1;
}

// ########################################################################

void tiger_net_close()
{
    close_socket();
    delete net_buffer;
}

// ########################################################################

void tiger_net_acq_running(int running)
{
    // disconnect from tiger if acq is stopped
    if( !running )
        close_socket();
}
