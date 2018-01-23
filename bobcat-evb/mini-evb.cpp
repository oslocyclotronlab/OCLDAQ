
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

#include "evb_caen_atdc.h"
#include "evb_lecroy_1151n.h"
#include "evb_mesytec_atdc.h"
#include "evb_tpu.h"
#include "evb_buffer.h"
#include "evb_transfer.h"
#include "evb_readconfig.h"
#include "mini-evb.h"

#if 1
#include "evb_tiger_sbs.h"
inline loopstate_t tiger_any_wait() { return tiger_sbs_wait(); }
inline loopstate_t tiger_any_flush() { return tiger_sbs_flush(); }
inline void tiger_any_acq_running(int running) { tiger_sbs_acq_running(running); }
inline void tiger_any_open() { tiger_sbs_open(); }
inline void tiger_any_close() { tiger_sbs_close(); }
inline bool tiger_any_is_listening() { return tiger_sbs_is_listening(); }
#else
#include "evb_tiger_net.h"
inline loopstate_t tiger_any_wait() { return tiger_net_wait(); }
inline loopstate_t tiger_any_flush() { return tiger_net_flush(); }
inline void tiger_any_acq_running(int running) { tiger_net_acq_running(running); }
inline void tiger_any_open() { tiger_net_open(); }
inline void tiger_any_close() { tiger_net_close(); }
//inline bool tiger_any_is_listening() { return true; /* TODO: implement this! */ }
#endif

static const int WALLCLOCK_BOXID = 0x01;
static const int WALLCLOCK_CH    = 16;

static const int SAFE = 256; // only fill buffers until this many words remain free

static UiO_TPU_t   tpu;
#ifdef VME_SCALER
    static LECROY_1151N_t vme_scaler;
#endif
static CAEN_ATDC_t    caen_adc_e, caen_adc_de2, caen_tdc_1, caen_tdc_2;
static MESYTEC_ATDC_t mesytec_adc_nai, mesytec_adc_de1, mesytec_adc_ge;

// number of microseconds/loop in WAITupdate(...)
const float usLOOP = 0.0362;

// times required for the different modules to be ready
const int loopPUR   = (int)((/*uPUR=*/ 8.0 / usLOOP) + 0.5);
//const int loopVME   = (int)((/*uVME=*/ 6.0 / usLOOP) + 0.5); // Old loop length
const int loopVME   = (int)((/*uVME=*/ 10.0 / usLOOP) + 0.5);   // WIth Ge ADC
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
//! Returns LEAVELOOP, WAIT4ENGINE or NEXTEVENT.
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

                CAEN_ATDC_clear(&caen_adc_e);
                CAEN_ATDC_clear(&caen_adc_de2);
                CAEN_ATDC_clear(&caen_tdc_1);
                CAEN_ATDC_clear(&caen_tdc_2);
                MESYTEC_ATDC_clear(&mesytec_adc_de1);
                MESYTEC_ATDC_clear(&mesytec_adc_nai);
                MESYTEC_ATDC_clear(&mesytec_adc_ge);

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
#endif
                        if( (waitloopcount&0xffff) == 0 ) {
                            printf("?");
                            fflush(stdout);
                            if( !tiger_any_is_listening() )
                                return WAIT4ENGINE;
                        }
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
        WAITupdate(15*loop1us, loopTOT);

        // write trigger pattern from TPU 
        UiO_TPU_data2buffer(&tpu);
         
        // read out VME ADCs and TDCs
        WAITupdate(loopVME, loopTOT);


        CAEN_ATDC_data2buffer(&caen_adc_e);
        CAEN_ATDC_data2buffer(&caen_adc_de2);
        CAEN_ATDC_data2buffer(&caen_tdc_1);
        CAEN_ATDC_data2buffer(&caen_tdc_2);
        MESYTEC_ATDC_data2buffer(&mesytec_adc_nai);
        MESYTEC_ATDC_data2buffer(&mesytec_adc_de1);
        MESYTEC_ATDC_data2buffer(&mesytec_adc_ge);
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

        while( buffer_capacity() >= SAFE && !check_interrupt() && state == NEXTEVENT ) {
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
                        buffer_reset();
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
               "|                      Eventbuilder 2.2                      |\r\n"
               "|                                                            |\r\n"
               "|          A VME-based data acquisition system for           |\r\n"
               "|            the CACTUS/SiRi multidetector system            |\r\n"
               "|      Written for the CES RIO2 single board processor       |\r\n"
               "|      with a PowerPC 604r @ 300 MHz CPU running LynxOS      |\r\n"
               "| E-mail  : magne.guttormsen@fys.uio.no                      |\r\n"
               "| E-mail  : alexander.buerger@fys.uio.no                     |\r\n"
               "| E-mail  : a.c.larsen@fys.uio.no                            |\r\n"
               "| Created :  3 Jan 2008                                      |\r\n"
               "| Modified:  8 Jan 2014                                      |\r\n"
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
        unsigned short int tdc_timerange_nai[1];
        if( !read_config_values("timerange_nai.txt", tdc_timerange_nai, 1) )
                exit(-1);
        CAEN_ATDC_open(&caen_tdc_1, 0xAAA00000, /*box*/ 0x10, 1);
        CAEN_ATDC_config(&caen_tdc_1);
        CAEN_ATDC_config_timerange(&caen_tdc_1, tdc_timerange_nai[0]);      // TDC (Si - NaI)

        unsigned short int tdc_timerange_ge[1];
        if( !read_config_values("timerange_ge.txt", tdc_timerange_ge, 1) )
            exit(-1);
        CAEN_ATDC_open(&caen_tdc_2, 0xAAA10000, /*box*/ 0x11, 1);
        CAEN_ATDC_config(&caen_tdc_2);
        CAEN_ATDC_config_timerange(&caen_tdc_2, tdc_timerange_ge[0]);      // TDC (Si - Ge)
    
        unsigned short int adc_thr_e[32];
        if( !read_config_values("thresholds_e.txt", adc_thr_e, 32) )
                exit(-1);
        CAEN_ATDC_open(&caen_adc_e, 0xAAB00000, /*box*/ 0x21, 0);       // E (Si)
        CAEN_ATDC_config(&caen_adc_e, adc_thr_e);
    
        unsigned short int adc_thr_de1[32];
        if( !read_config_values("thresholds_de1.txt",adc_thr_de1, 32) )
                exit(-1);
        MESYTEC_ATDC_open(&mesytec_adc_de1, 0xAAB10000, /*box*/ 0x22);  // DE1 ch 0-31 (Si)
        MESYTEC_ATDC_config(&mesytec_adc_de1, adc_thr_de1);
    
        unsigned short int adc_thr_de2[32];
        if( !read_config_values("thresholds_de2.txt", adc_thr_de2, 32) )
                exit(-1);
        CAEN_ATDC_open(&caen_adc_de2, 0xAAB20000, /*box*/ 0x23, 0);     // DE2 ch 32-63 (Si)
        CAEN_ATDC_config(&caen_adc_de2, adc_thr_de2);
    
        unsigned short int adc_thr_nai[32];
        if( !read_config_values("thresholds_nai.txt", adc_thr_nai, 32) )
                exit(-1);
        MESYTEC_ATDC_open(&mesytec_adc_nai, 0xAAD00000, /*box*/ 0x24);  // E (NaI)
        MESYTEC_ATDC_config(&mesytec_adc_nai, adc_thr_nai);
        
        unsigned short int adc_thr_ge[32];
        if( !read_config_values("thresholds_ge.txt", adc_thr_ge, 32) )
            exit(-1);
        MESYTEC_ATDC_open(&mesytec_adc_ge, 0xAAD10000, /*box*/ 0x25);  // E (Ge)
        MESYTEC_ATDC_config_8k(&mesytec_adc_ge, adc_thr_ge);

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

        MESYTEC_ATDC_close(&mesytec_adc_nai);
        MESYTEC_ATDC_close(&mesytec_adc_de1);
        MESYTEC_ATDC_close(&mesytec_adc_ge);
#ifdef VME_SCALER
        LECROY_1151N_close(&vme_scaler);
#endif
        UiO_TPU_close(&tpu);
        CAEN_ATDC_close(&caen_tdc_1);
        CAEN_ATDC_close(&caen_tdc_2);
        CAEN_ATDC_close(&caen_adc_de2);
        CAEN_ATDC_close(&caen_adc_e);

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
