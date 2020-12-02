
#include "net_client.h"

#include <arpa/inet.h>
#include <errno.h>
#include <netinet/in.h>
#include <netdb.h>
#include <signal.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>

#include "utilities.h"

// ########################################################################

int nc_connect(const char* hostname, int hostport)
{
    int sock = socket(PF_INET, SOCK_STREAM, 0);
    if(sock < 0)
        return -1;
    
    // connect to the server
    struct sockaddr_in servername;
    init_sockaddr(&servername, hostname, hostport);
	
    while(0 > connect(sock,(struct sockaddr*)&servername, sizeof(servername))) {
        switch( errno ) {
        case ETIMEDOUT:
        case EADDRNOTAVAIL:
        case ECONNREFUSED:
            return -2;
        default:
            return -1;
        }
    }

    return sock;
}

// ########################################################################

#if 0
int main(int argc, char* argv[])
{
    if(argc != 3) {
        fprintf(stderr, "net-client hostname port\n");
        exit(EXIT_FAILURE);
    }

    const char* hostname = argv[1];
    const int   hostport = atoi(argv[2]);

    int sock = nc_connect(hostname, hostport);

    char buf[1024];
    read(sock, buf, sizeof(buf));

    write_all(sock, "quit\r", 5);

    //read(sock, buf, sizeof(buf));

    close(sock);
    
    exit(EXIT_SUCCESS);
}
#endif
