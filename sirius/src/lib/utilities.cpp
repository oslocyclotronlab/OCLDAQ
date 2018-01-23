
#include "utilities.h"

#include <sstream>
#include <iostream>

#include <arpa/inet.h>
#include <errno.h>
#include <netinet/in.h>
#include <netdb.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

// ########################################################################

std::string ioprintf(const char* format, ...)
{
    char message[512];

    va_list ap;
    va_start(ap, format);
    vsnprintf(message, sizeof(message), format, ap);
    va_end(ap);

    return std::string(message);
}

/** Very simple version of a file existance check.

    @param filename name/path of the file to check
    @return true if lstat returns without error
 */
// ########################################################################

bool file_exists(std::string const& filename)
{
    struct stat s;
    return lstat(filename.c_str(), &s) == 0;
}

// ########################################################################

/**
 * Tries to find a shared memory segment and to attach it to this process.
 * 
 * @return a pointer to the shared memory segment, if successful; 0 otherwise
 */
int* attach_shared(key_t key, size_t size, bool create)
{
    int* shmptr;

    /* allocate shared memory segment */
    int shmid = shmget(key, size, 0666 | (create ? IPC_CREAT : 0));
    if (shmid == -1)
        return 0;

    /* attach shared memory segment to process */
    shmptr = (int*)shmat(shmid, NULL, 0);
    if( shmptr == (void*)-1 )
        return 0;

    return shmptr;
}

// ########################################################################

int make_socket(int port, bool reuse_addr)
{
    /* Create the socket. */
    int sock = socket(PF_INET, SOCK_STREAM, 0);
    if(sock < 0) {
        perror("socket creation");
        return -1;
    }

    if( reuse_addr ) {
        // allow socket to re-use address; this is important to be able to
        // re-start engine immediately after it stopped (otherwise, up to
        // 4min waiting is required), but it is theoretically possible
        // that some errors occur from old packets (unlikely because it
        // listens only to localhost)
        const int yes = 1;
        if( setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes))<0 ) {
            perror("setsockopt");
            ::close(sock);
            return -1;
        }
    }

    // bind socket to localhost:port
    struct sockaddr_in name;
    bzero(&name, sizeof(name));
    name.sin_family = AF_INET;
    //if( inet_aton("localhost", &name.sin_addr)!=0 ) {
    //    perror("inet_aton");
    //    ::close(sock);
    //    return -1;
    //}
    name.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
    name.sin_port = htons(port);
    if(bind(sock,(struct sockaddr *) &name, sizeof(name)) < 0) {
        perror("binding address to socket");
        ::close(sock);
        return -1;
    }

    return sock;
}

// ########################################################################

void init_sockaddr(struct sockaddr_in *name, const char *hostname, int port)
{
    struct hostent *hostinfo = gethostbyname(const_cast<char*>(hostname));
    if( !hostinfo ) {
        std::cerr << "unknown host " << hostname << std::endl;
        exit(EXIT_FAILURE);
    }

    name->sin_addr = *(struct in_addr *) hostinfo->h_addr;
    name->sin_family = AF_INET;
    name->sin_port = htons(port);
}

// ########################################################################

std::string escape(const std::string& txt)
{
    std::ostringstream out;
    for(unsigned int i=0; i<txt.size(); ++i) {
        char ch = txt[i];
        if( ch == '\n' )
            out << "\\n";
        else if( ch == '\r' )
            out << "\\r";
        else if( ch < ' ' )
            out << "\\" << (int)ch;
        else
            out << ch;
    }
    return out.str();
}

// ########################################################################

std::istream& skipsymbols(std::istream& fp)
{
    while( true ) {
        const char ch = fp.peek();
	if(ch == ',' || ch == ':' || ch ==';' || ch == '!' || ch == '/')
            fp.ignore(1);
        else
            break;
    }
    return fp;
}

// ########################################################################

std::string strip(const std::string& s)
{
    std::string::size_type start = s.find_first_not_of(" \t\r\n");
    if( start==std::string::npos )
        start = 0;

    std::string::size_type stop = s.find_last_not_of(" \t\r\n");
    if( stop==std::string::npos )
        stop = s.size()-1;
    
    return s.substr(start, stop+1-start);
}

