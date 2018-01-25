
#include "evb_buffer.h"

#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>


unsigned long *buffer;
long idx_data=1, idx_start=0;

const unsigned long EVENT_HEADER = 0xc << 28;     // event header identification code
const unsigned long BUFFER_END   = 0x8 << 28;     // buffer end identification code

// ########################################################################

void buffer_open()
{
    printf("Opening eventbuffer...");
    buffer = new unsigned long[MAXBUF];
    buffer_reset();
    printf("DONE \n");
}

// ########################################################################

void buffer_close()
{
    printf("Closing eventbuffer...");
    delete buffer;
    printf("DONE \n");
}

// ########################################################################

void buffer_accept()
{
    buffer[idx_start] = EVENT_HEADER | (idx_data - idx_start - 1);
    idx_start = idx_data;
    idx_data += 1;
}

// ########################################################################

void buffer_copy(long start, long count, unsigned long* dest)
{
    // copy a part of the event buffer
#if 1
    const unsigned long *p = buffer + start;
    for(int j=0; j<count; ++j)
	dest[j] = p[j];
#else
    memcpy(dest, buffer+start, count*sizeof(buffer[0]));
#endif
#if 0
    int k=0;
    for(int j=0; j<count; ++j) {
	if( dest[j] != 0 )
	    k += 1;
    }
    printf("k=%d\n", k);
#endif
}

// ########################################################################

void buffer_fill0()
{
    buffer[idx_start] = BUFFER_END;
#if 1
    for(int i=idx_data; i<MAXBUF; i++)
	buffer[i] = 0;
#else
    memset(buffer+idx_data, 0, (MAXBUF-idx_data)*sizeof(buffer[0]));
#endif
}

// ########################################################################

void buffer_dump(const char* filename)
{
    // create file with or truncate it if it exists
    int fd = open(filename, O_WRONLY|O_CREAT|O_TRUNC, 0644);
    write(fd, buffer, MAXBUF*sizeof(buffer[0]));
    close(fd);
}

