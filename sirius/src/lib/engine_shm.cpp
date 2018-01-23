
#include "engine_shm.h"

#include <string.h>
#include <sys/shm.h>

static const int PERMS_WRITE = 0666; // read for all, write for all
static const int PERMS_READ  = 0444; // read for all, write for none

enum {
    BUFFER_OFF   = 128,
    BUFFER_SIZE  = 32768
};

static unsigned int *shared_mem = 0;

// ########################################################################

unsigned int* engine_shm_attach(bool write)
{
    const int MAXBUF      = BUFFER_OFF + BUFFER_SIZE;
    const int MAXBUF_char = MAXBUF*sizeof(shared_mem[0]);

    // attach shared data buffer segment to process
    const int shmid_buffer = shmget(ENGINE_SHM_KEY, MAXBUF_char,
                                    write ? (IPC_CREAT | PERMS_WRITE) : PERMS_READ);
    if( shmid_buffer == -1)
      	return 0;

    shared_mem = (unsigned int*)shmat(shmid_buffer, 0, 0);
    if( shared_mem == (void*)-1 ) {
        shared_mem = 0;
      	return 0;
    }

    if( write ) {
        // clear data buffer segment
        memset( (char*)shared_mem, 0, MAXBUF_char );
        
        // store some info for sorting process
        shared_mem[ENGINE_TIME_US]      = 0;
        shared_mem[ENGINE_TIME_S]       = 0;
        shared_mem[ENGINE_DATA_START] = BUFFER_OFF; // TODO: take these from vme cpu
        shared_mem[ENGINE_DATA_SIZE]  = BUFFER_SIZE;
    }
    return shared_mem;
}

// ########################################################################

bool engine_shm_detach()
{
    if( shared_mem ) {
        // detach shared memory
        if( shmdt( shared_mem ) == -1 )
            return false;
        shared_mem = 0;
    }
    return true;
}
