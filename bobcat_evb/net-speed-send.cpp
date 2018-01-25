
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
#include <sys/types.h>  // for select
#include <time.h>       // for select
#include <stropts.h>    // for poll
#include <poll.h>       // for poll
 

static const char* hostname = "192.168.0.33";
static const int   hostport = 41066;

const int MAXBUF = 0x8000; // size of buffer (in 4-byte words)
static unsigned long* net_buffer = 0;
static const int MAXBUF_char = MAXBUF*sizeof(net_buffer[0]);

static int closed = 0;

static int sock = -1;

static char leaveloop = 'n';
inline int check_interrupt()
{
    return leaveloop == 'y';
}

inline int max(int a, int b) { return (a>b) ? a : b; }

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


int main(int argc, char* argv[])
{
    bool do_select = ( argc == 2 && !strcmp(argv[1], "select"));
    bool do_poll   = ( argc == 2 && !strcmp(argv[1], "poll"));
    signal(SIGPIPE, pipe_handler);

    if( (sock=make_connection(hostname, hostport)) <= 0 ) {
        fprintf(stderr, "Could not connect.\n");
        exit(-1);
    }

    struct timeval time_start, time_stop;
    gettimeofday(&time_start, 0);

    net_buffer = new unsigned long[MAXBUF];
    for(int j=0; j<500; ++j) {
        for(int i=0; i<MAXBUF; ++i)
            net_buffer[i] = (i<<16) | j;
        write_all(sock, (char*)net_buffer, MAXBUF_char);

        if( do_select ) {
            fd_set r_fds, w_fds;
            FD_ZERO(&r_fds);
            FD_ZERO(&w_fds);
            FD_SET(sock, &r_fds);
            int maxfd = max(sock, 0);
            struct timeval t = {0,0};
            int s = select(maxfd+1, &r_fds, &w_fds, 0,&t);
            if( s != 0 ) {
                printf("s=%d\n", s);
            }
        } else if( do_poll ) {
            struct pollfd pfds[1];
            pfds[0].fd     = sock;
            pfds[0].events = POLLIN;

            int p = poll(pfds, 1, 0);
            if( p < 0 ) {
                int e = errno;
                perror("poll");
                printf("errno = %d\n", e);
            } else if( p != 0 ) {
                printf("p=%d\n", p);
            }
        }
    }

    gettimeofday(&time_stop, 0);
    double timediff = (time_stop .tv_sec + 1e-6*time_stop .tv_usec)
        - (time_start.tv_sec + 1e-6*time_start.tv_usec);
    printf("time: %f µs\n", timediff);
    
    delete net_buffer;
    close_socket();
    return 0;
}

// g++ -Wall -W -O2 -g -o net-speed-send net-speed-send.cpp -lnetinet
