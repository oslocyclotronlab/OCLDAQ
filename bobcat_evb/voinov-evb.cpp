
#include <ctype.h>
#include <signal.h>
#include <sched.h>
#include <smem.h>
#include <stdio.h>
#include <time.h>
#include <unistd.h>

#include <ces/uiocmd.h>
#include <ces/vmelib.h>

#define WRITE_CLOCK 1
#define NO_PILEUP 1
#define VME_SCALER 1
#define MESYTEC_MADC32 1
//#define CAEN_NAI_ADC

#include "evb_caen_atdc.h"
#include "evb_lecroy_1151n.h"
#include "evb_mesytec_madc32.h"
#include "evb_tpu.h"
#include "evb_buffer.h"
#include "evb_transfer.h"
#include "mini-evb.h"

#if 1
#include "evb_tiger_sbs.h"
inline loopstate_t tiger_any_wait() { return tiger_sbs_wait(); }
inline loopstate_t tiger_any_flush() { return tiger_sbs_flush(); }
inline void tiger_any_acq_running(int running) { tiger_sbs_acq_running(running); }
inline void tiger_any_open() { tiger_sbs_open(); }
inline void tiger_any_close() { tiger_sbs_close(); }
#else
#include "evb_tiger_net.h"
inline loopstate_t tiger_any_wait() { return tiger_net_wait(); }
inline loopstate_t tiger_any_flush() { return tiger_net_flush(); }
inline void tiger_any_acq_running(int running) { tiger_net_acq_running(running); }
inline void tiger_any_open() { tiger_net_open(); }
inline void tiger_any_close() { tiger_net_close(); }
#endif

static const int WALLCLOCK_BOXID = 0x01;
static const int WALLCLOCK_CH    = 16;

static const int SAFE = 256; // only fill buffers until this many words remain free

static CAEN_ATDC_t vme_tdc_1;
static UiO_TPU_t   tpu;
#ifdef VME_SCALER
static LECROY_1151N_t vme_scaler;
#endif
#ifdef MESYTEC_MADC32
static MESYTEC_MADC32_t madc32;
#endif

// number of microseconds/loop in WAITupdate(...)
const float usLOOP = 0.0362;

// times required for the different modules to be ready
const int loopPUR   = (int)((/*uPUR=*/ 8.0 / usLOOP) + 0.5);
const int loopVME   = (int)((/*uVME=*/ 6.0 / usLOOP) + 0.5);
const int loop500ns = (int)((          0.5 / usLOOP) + 0.5);
const int loop1us   = (int)((          1.0 / usLOOP) + 0.5);

// counter for the number of events
static unsigned int event_count = 0;

// ########################################################################

// set to 'y' in interrupt handler (i.e. when Ctrl-C is pressed)
char leaveloop = 'n';

// ########################################################################
//! Keyboard interrupt handler.
static void keyb_int(int sig_num)
{
        if (sig_num == SIGINT) {
                printf("\n\nLeaving event-loop...\n");
                printf("Type r if you want to restart VME-acquisition\n");
                leaveloop = 'y';
        }
}

// ########################################################################
// ########################################################################

inline void WAIT(long x)
{
        volatile long ii;
        for(ii=x; ii; ii--) {
                /*1 loop =66/300*0.1647us*/
        }
}

// ########################################################################

inline void WAITupdate(long a, long &b)
{
        if(a>b){
                WAIT(a-b);
                b=a+2;
        }
}

// ########################################################################
//! Wait for the next event and copy it to the buffer.
//! Returns LEAVELOOP or NEXTEVENT.
static loopstate_t next_event()
{
        // time counter (used to wait for modules) starts at 0
        long loopTOT;

        // counter for the number of wait loop iterations
        unsigned int waitloopcount = 0;

#ifdef NO_PILEUP
        do {
#endif
                // clear is necessary in case of pileup - then, the
                // conversion has run, but we did not fetch the event
                // (we could also move these two calls further down)
                CAEN_ATDC_clear(&vme_tdc_1);
#ifdef MESYTEC_MADC32
                MESYTEC_MADC32_clear(&madc32);
#endif

                // tell TPU module that it should wait for the next
                // trigger by writing to the nextreg address
                UiO_TPU_nextev(&tpu);

                // without waiting, there is almost always an empty
                // event with trigger pattern 0 after a real event;
                // did not try to find out shortest possible waiting
                // time
                WAIT(loop1us);

                // wait for tpu bit 7 in numreg - bit 7==1 indicates
                // that an event was recorded
                while( !bit(UiO_TPU_numreg(&tpu), 7) ) {
                        waitloopcount += 1;
                        
                        // check if the interrupt handler has set a flag
                        if( check_interrupt() )
                                return LEAVELOOP;

#ifdef USLEEP_SLEEPS_ONLY_USECS
                        // sleep 1 us, should not effect the possible
                        // event rate very much
                        usleep(1);
                        // unfortunately, usleep does affect the
                        // program VERY much, slowing it down by a
                        // factor of about 800; I do not understand
                        // the reason for that
#else
                        WAIT(5*loop1us);
                        if( (waitloopcount&0xffff) == 0 ) {
                            printf("?");
                            fflush(stdout);
                        }
#endif
                }

                // now we know that a trigger was seen by the TPU
                
                loopTOT = 0;
                //yield();
                
#ifdef NO_PILEUP
                // check for pileup: first wait a little...
                WAITupdate(loopPUR, loopTOT);
                // check pile-up bit -- if it is not set, read this event
        } while( bit(UiO_TPU_numreg(&tpu), 6) );
#endif
        
#if 1
        WAITupdate(2*loop1us, loopTOT);

        // write trigger pattern from TPU 
        UiO_TPU_data2buffer(&tpu);
         
        // read out VME ADCs and TDCs
        WAITupdate(loopVME, loopTOT);

        CAEN_ATDC_data2buffer(&vme_tdc_1);
#ifdef MESYTEC_MADC32
        MESYTEC_MADC32_data2buffer(&madc32);
#endif
#endif

        // check if the event has data
        if(buffer_eventlength() > 0) {
#if 1 && defined(WRITE_CLOCK)
                // write the wall clock time every few events
                if( !(event_count & 0x3ff) && buffer_capacity()>2) {
                        time_t now = time(0);
                        buffer_put(WALLCLOCK_BOXID<<23,  WALLCLOCK_CH   <<16, (now>>16) & 0xFFFF);
                        buffer_put(WALLCLOCK_BOXID<<23, (WALLCLOCK_CH+1)<<16,  now      & 0xFFFF);
                }
#endif

#ifdef VME_SCALER
                // write the scaler counters now and then; no reset,
                // just let the counters overflow
                if( !(event_count & 0xfff) && buffer_capacity()>16)
                        LECROY_1151N_data2buffer(&vme_scaler);
#endif

                // write event header with event length
                buffer_accept();
                event_count += 1;
        }
        return NEXTEVENT;
}

// ########################################################################
//! Run the event loop once without sending the buffer.
static void loop_1()
{
        // reset event counter and buffer pointer
        event_count = 0;
        // counter for the number of wait loop iterations
        unsigned int waitloopcount = 0;

        UiO_TPU_clear(&tpu);
#ifdef VME_SCALER
        LECROY_1151N_clear(&vme_scaler);
#endif
        buffer_reset();

        // state can be either WAIT4ENGINE, NEXTEVENT, or LEAVELOOP
        loopstate_t state = NEXTEVENT;

        while( buffer_capacity() >= SAFE && !check_interrupt() ) {
                state = next_event();
                if( (event_count&0x3ff) == 0 && event_count>0 ) {
                        printf("\r%u %f", event_count, float(waitloopcount)/event_count);
                        fflush(stdout);
                }
        }
}

// ########################################################################
//! Run the event loop more than once, sending buffers as thez are filled.
//! onerun determines how often:  0 => until Ctrl-C
//!                              >0 => finite number of buffers
static void loop_n(int onerun)
{
        // reset event counter and buffer pointer
        event_count = 0;

        UiO_TPU_clear(&tpu);
#ifdef VME_SCALER
        LECROY_1151N_clear(&vme_scaler);
#endif
        buffer_reset();

        // state can be either WAIT4ENGINE, NEXTEVENT, or LEAVELOOP
        loopstate_t state = WAIT4ENGINE;

        // main readout loop
        do {
                switch(state) {
                case WAIT4ENGINE:
                        // set bit so that tiger can see that the acq is running
#ifdef USLEEP_SLEEPS_ONLY_USECS
                        usleep(1);
#endif
                        tiger_any_acq_running(1);
                        
                        state = tiger_any_wait();
                        break;
                case NEXTEVENT:
                        if( buffer_capacity() < SAFE ) {
                                // not enough space in buffer
                                
                                // add end of buffer marker (but only
                                // if there is room) and zeroes
                                buffer_fill0();

                                // send it buffer to engine
                                state = tiger_any_flush();

                                // reset pointer to beginning of
                                // buffer; XXX no "if" here -
                                // otherwise the first buffer after
                                // re-connect would old
                                buffer_reset();

                                // decrement buffer counter if necessary
                                if( onerun>0 && state==NEXTEVENT ) {
                                        onerun -= 1;
                                        if( onerun==0 )
                                                state = LEAVELOOP;
                                }
                        } else {
                                state = next_event();

                                // print some alive message nwo and then
                                if( (event_count&0x3fff) == 0 ) {
                                        printf("\r%u", event_count);
                                        fflush(stdout);
                                }
                        }
                case LEAVELOOP:
                        break;
                }
        } while( state != LEAVELOOP && !check_interrupt() );

        // notify engine that the VME acq has stopped
        tiger_any_acq_running(0);
}

// ########################################################################

void Menu()
{
        printf("\n"
               "      R : Run infinite event-loop               \n"
               "      O : One-buffer run                        \n"
               "      2 : 2-buffer run                          \n"
               "      9 : 9-buffer run                          \n"
               "      D : Dump data buffer (buffer one)         \n"
               "      H : Help, listing of this menu            \n"
               " Ctrl-C : Jump out of infinite event-loop       \n"
               "      * : Exit                                  \n"
               "\n");
}

// ########################################################################

int readchar()
{
        int c1;
        int c2 = '\n';
        while((c1 = getchar()) != '\n')
                c2 = tolower(c1);
        return c2;
}

// ########################################################################

int main(int argc, char* argv[])
{
        // set up interrupt handler (Ctrl-C)
        if(signal(SIGINT, SIG_IGN) != SIG_IGN) {
                signal(SIGINT, keyb_int);
        }

        printf(" ____________________________________________________________ \r\n"
               "|                                                            |\r\n"
               "|                      Eventbuilder 2.0test                  |\r\n"
               "|                                                            |\r\n"
               "|          A VME-based data acquisition system for           |\r\n"
               "|            the CACTUS/SiRi multidetector system            |\r\n"
               "|      Written for the CES RIO2 single board processor       |\r\n"
               "|      with a PowerPC 604r @ 300 MHz CPU running LynxOS      |\r\n"
               "| E-mail  : magne.guttormsen@fys.uio.no                      |\r\n"
               "| E-mail  : alexander.buerger@fys.uio.no                     |\r\n"
               "| Created :  3 Jan 2008                                      |\r\n"
               "| Modified: 22 Jan 2009                                      |\r\n"
               "|____________________________________________________________|\r\n"
               "                                                              \r\n");

        buffer_open();
        
        tiger_any_open();
        
        UiO_TPU_open(&tpu, 0xf0ffff00, /*box*/ 0x00);
        UiO_TPU_clear(&tpu);

        // wall clock time is box 0x01 (WALLCLOCK_BOXID)

#ifdef VME_SCALER
        LECROY_1151N_open(&vme_scaler, 0x444400, /*box*/ 0x02, 0xf);
#endif

        CAEN_ATDC_open(&vme_tdc_1, 0xAAA00000, /*box*/ 0x10, 1);
        CAEN_ATDC_config(&vme_tdc_1);

#ifdef MESYTEC_MADC32
        MESYTEC_MADC32_open(&madc32, 0xAAD00000, /*box*/ 0x24); // mesytec MADC-32
        MESYTEC_MADC32_config(&madc32);
#endif

        Menu();
        
        int cmd;
        do {
                leaveloop ='n';
                printf("mini-evb>");
                if( argc>1 && *argv[1] != 0 ) {
                        cmd = *argv[1];
                        printf("%c\n", cmd);
                        argv[1] += 1;
                } else {
                        cmd = readchar();
                }
                switch (cmd) {
                        case 'o': loop_1();         break;
                        case 'r': loop_n(0);        break;
                        case '2': loop_n(2);        break;
                        case '9': loop_n(9);        break;
                        case 'd':
                                buffer_fill0();
                                buffer_dump("buffer.dmp");
                                break;
                        case 'h': Menu();           break;
                        case '\n':                  break;
                        case '*':                   break;
                        case 'q':                   break;
                        default:
                                printf(" Illegal command, try again! (h for help)\n");
                }
        } while(cmd != '*' && cmd != 'q');

#ifdef MESYTEC_MADC32
        MESYTEC_MADC32_close(&madc32);
#endif
#ifdef VME_SCALER
        LECROY_1151N_close(&vme_scaler);
#endif
        UiO_TPU_close(&tpu);
        CAEN_ATDC_close(&vme_tdc_1);

        tiger_any_close();

        buffer_close();

        return 0;
}

// ########################################################################
// ########################################################################

/* for emacs */
/*** Local Variables: ***/
/*** indent-tabs-mode: nil ***/
/*** End: ***/
