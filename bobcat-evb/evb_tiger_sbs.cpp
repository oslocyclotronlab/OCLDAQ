
#include "evb_tiger_sbs.h"

#include "mini-evb.h"
#include "evb_buffer.h"

#include <smem.h>
#include <stdio.h>
#include <unistd.h>
#include <ces/uiocmd.h>

// phys. addr. for SRAM -> A24 slave
static const unsigned long CPUADDR = 0xff010000;

// absolute VME A24 slave address with hex switch = 5 on CPU card
static const unsigned long SLV24ADDR = 0x00850000;

static unsigned long *pBUFFER_ADDRESS, *pBUFFER_LENGTH, *pSEMA[4];
static unsigned long *pVMESTATUS, *pTigerSTATUS;

static int  mem24;
static unsigned long *pmes, *pmove;
static const long messagebytes = 10*4;
static const long movebytes = 0x8000;
static const long slavebytes = messagebytes + movebytes;

static long vad;

// ########################################################################

//! Wait for tiger (i.e. sirius start button).
//! Returns LEAVELOOP or NEXTEVENT.
loopstate_t tiger_sbs_wait()
{
    // wait 10ms
    usleep(10000);

    // all transfer buffers are now empty by definition
    for(int i=0; i<4; ++i)
        *pSEMA[i] = 0;

    // if tiger is already waiting, ask for a restart
    if(*pTigerSTATUS == 1) {
        printf( "SIRIUS/engine already running: please stop and restart\n");
        return LEAVELOOP;
    }
    
    // now actually wait for tiger
    printf( "Push the START button for SIRIUS!\n");
    do {
        usleep(10000);
        if(leaveloop == 'y')
            return LEAVELOOP;
    } while(*pTigerSTATUS == 0);
    
    printf( "tiger seems to listen now\n");
    // tiger is now running, we can start collecting data
    return NEXTEVENT;
}

// ########################################################################

bool tiger_sbs_is_listening()
{
    return (*pTigerSTATUS != 0);
}

// ########################################################################
//! Transfer (passively) the buffers to tiger.
//! Returns LEAVELOOP or NEXTEVENT or WAIT4ENGINE.
loopstate_t tiger_sbs_flush()
{
        // move 4 buffers using a loop
        for(int i=0; i<4; ++i) {
                // copy a part of the event buffer
                buffer_copy(i*0x2000, 0x2000, pmove);

                // set buffer i semaphore to FULL (!=0); tiger should
                // remark this and fetch the buffer and set the
                // semaphore to EMPTY (0) afterwards
                *pSEMA[i] = 1;

                // wait until buffer i has been fetched
                do {
#ifdef USLEEP_SLEEPS_ONLY_USECS
                        // link speed is 30MB/s, data size 32kB => min time ~1ms
                        usleep(250);
                        // removed as usleep seems not to work, see
                        // comment in next_event
#endif
                        // if engine(tiger) is not running, drop the buffer
                        if(*pTigerSTATUS == 0)
                                return WAIT4ENGINE;
                        // if interrupt handler has run, drop the buffer
                        if(leaveloop == 'y')
                                return LEAVELOOP;
                } while( *pSEMA[i] != 0 );
        }
        
        return NEXTEVENT;
}

/* ######################################################################## */

static void SLAVEset(unsigned long physical, unsigned long value, const char* name)
{
    const unsigned long window = (1<<12)-1; // set bits 0..12

    const unsigned long logical_base = (unsigned long)
        smem_create("MEMsl", (char*)(physical&(~window)), 4, SM_WRITE);
    if( !logical_base ) {
        fprintf(stderr,"Unable to allocate MEMsl window for %s\n", name);
        exit(-1);
    }

    unsigned long* logical = (unsigned long*)(logical_base|(physical&window));
    *logical = value;
    printf("%s at phys/log address 0x%08lx/0x%08lx set to 0x%08lx\n",
           name, physical, (unsigned long)logical, value);
    
    smem_create("MEMsl", (char*)logical_base, 4, SM_DETACH);
    smem_remove("MEMsl");
}

/* ######################################################################## */

static void SLAVEinitiate()
{
        const unsigned long PCI_CTL2 = 0xa0f50008;
        const unsigned long PCI_CTL3 = 0xa0f5000c;
        
        printf("Enable slave A24 memory (writting to PCI_CTL registers)...");

        // the values seem quite byteswapped :-) according to the RIO2
        // manual (pages 37 and 48), these should mean mapping 64kB
        // VME CSR at the addresses PDH(@4001)=005f and
        // PDL(@4000)=8000 as required by page 48, i.e. it looks like
        // the bytes are in reverse significance order
        SLAVEset(PCI_CTL2, 0x01400000, "PCI_CTL2 a");
        SLAVEset(PCI_CTL3, 0x5f000000, "PCI_CTL3 a");
        SLAVEset(PCI_CTL2, 0x00400000, "PCI_CTL2 b");
        SLAVEset(PCI_CTL3, 0x00800000, "PCI_CTL3 b");

        // RIO2 manual page 50 says that the 64kB CSR contain the SRAM
        // from 0x0000 to 0xAFFF, i.e. only 44 kB; therefore, the
        // 128kB buffer has to be transferred in 4 block of 32kB each

        printf("DONE \n");
}

// ########################################################################

static void A24open()
{
        SLAVEinitiate();
        printf("Allocating SRAM A24 slave memory \n"
               "for messagebox (0x%lx bytes) and buffer (0x%lx bytes)...",
               messagebytes, movebytes);
        if (!(mem24=(int)smem_create("MEM24",(char *)(CPUADDR&~0xFFF), slavebytes, SM_WRITE | SM_READ))) {
                fprintf(stderr,"Unable to allocate MEM window for slave A24 memory");
                exit(0);
        }
        pmes = (unsigned long *)(mem24 | (CPUADDR&0xFFF)); // first 10 message-words
        pBUFFER_ADDRESS = pmes + 0;
        pBUFFER_LENGTH  = pmes + 1;
        for(int i=0; i<4; ++i)
                pSEMA[i] = pmes + 2 + i;
        pVMESTATUS      = pmes + 6;
        pTigerSTATUS    = pmes + 7;
        pmove = pmes + (messagebytes/4);          // start of eventbuffer for program
        vad   = SLV24ADDR + messagebytes;         // start of eventbuffer for other CPUs
        printf("DONE \n");
}

// ########################################################################

static void A24close()
{
        *pVMESTATUS = 0;
        printf("Closing A24 slave memory...");
        smem_create("MEM24", (char*)mem24, slavebytes, SM_DETACH);
        smem_remove("MEM24");
        printf("DONE \n");
}

// ########################################################################

static void MESSAGEBOXconfig()
{
        printf("Writing status, moveaddress and bufferlength to messagebox...");
        *pBUFFER_ADDRESS = vad;                   // Start address of eventbuffer
        *pBUFFER_LENGTH  = MAXBUF;                // Store buffer length
        *pVMESTATUS      = 0;                     // eventbuilder is not yet running
        *pTigerSTATUS    = 0;                     // sirius has to be restarted to run
        pmes[8] = 0;                              // zeroing unused locations
        pmes[9] = 0;
        printf("DONE \n");
}

// ########################################################################

void tiger_sbs_open()
{
    A24open();
    MESSAGEBOXconfig();
}

// ########################################################################

void tiger_sbs_close()
{
    A24close();
}

// ########################################################################

void tiger_sbs_acq_running(int running)
{
    // notify tiger about the VME acq status
    *pVMESTATUS = running;
}

// ######################################################################## 

/* for emacs */
/*** Local Variables: ***/
/*** indent-tabs-mode: nil ***/
/*** End: ***/
