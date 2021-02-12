
#include "sbs_usb_comm.h"

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/file.h>
#include <sys/types.h>
#include <fcntl.h>
#include <ctype.h>
#include <unistd.h>

#ifdef SUPPORT_SBS_CARD
#include "btapi.h"
#endif
#ifdef SUPPORT_USB_CARD
#include "usb_vme.h"
#endif
#include "utilities.h"

// ########################################################################
// ########################################################################
// ########################################################################
// ########################################################################
// ########################################################################

#if defined(SUPPORT_USB_CARD) || defined(SUPPORT_SBS_CARD)

/// from buffer_defs.h
const unsigned int MESSAGE_BOX = 0x00850000;   // VMEbus Message_Box address
const unsigned int BUFFER_ADDRESS = MESSAGE_BOX + 0x00;
const unsigned int BUFFER_LENGTH  = MESSAGE_BOX + 0x04;
const unsigned int SEM[4] = { MESSAGE_BOX + 0x08, // movebuffer semaphores
			      MESSAGE_BOX + 0x0c,
			      MESSAGE_BOX + 0x10,
			      MESSAGE_BOX + 0x14 };
const unsigned int VMESTATUS = MESSAGE_BOX + 0x18; // status flag set by VME system
const unsigned int TIGERSTATUS = MESSAGE_BOX + 0x1c; // status flag set by this program
/// end of buffer_defs.h

// ########################################################################
// ########################################################################

#ifdef SUPPORT_SBS_CARD
// Unit descriptor for SBS library
static bt_desc_t btd;
static bool btd_closed = true;

static void sbs_vmepci1003_close()
{
    if( btd_closed )
        return;

    // check for status errors
    bt_error_t status = bt_chkerr(btd);
    if( status != BT_SUCCESS )
        bt_perror(btd, status, "engine ERROR: Status error from SBS VME-PCI");

    // close SBS VME-PCI
    status = bt_close(btd);
    if( status != BT_SUCCESS )
        bt_perror(btd, status, "engine ERROR: Could not close SBS VME-PCI");

    btd_closed = true;

    printf("SBS VME-PCI closed\n");
}

static void sbs_vmepci1003_open()
{
    if( !btd_closed )
        return;

    // open SBS VME-PCI
    const int unit = 0;
    const bt_dev_t type = BT_DEV_A24;
    char devname[BT_MAX_DEV_NAME]; // Device to open
    bt_error_t status = bt_open(&btd, bt_gen_name(unit, type, devname, BT_MAX_DEV_NAME), BT_RDWR);
    if( status != BT_SUCCESS ) {
        bt_perror(btd, status, "engine ERROR: Could not open SBS VME-PCI");
        exit(EXIT_FAILURE);
    }
    btd_closed = false;

    static bool exit_handler = false;
    if( !exit_handler ) {
        atexit(sbs_vmepci1003_close);
        exit_handler = true;
    }
        
    // clear any outstanding errors
    status = bt_clrerr(btd);
    if( status != BT_SUCCESS ) {
        bt_perror(btd, status, "engine ERROR: Could not clear errors from SBS VME-PCI");
        exit(EXIT_FAILURE);
    }
}

// ########################################################################

void sbs_vmepci1003_read_any(unsigned int address, unsigned int* dest, unsigned int count, const char* what=0)
{
    size_t n_read = 0;
    bt_error_t status = bt_read(btd, dest, address, count, &n_read);

    if( count > 4000000 )
	fprintf(stderr, "engine INFO: %d bytes read from VME a=%08x\n", n_read, address);

    if( status != BT_SUCCESS ) {
        if( n_read != count )
            fprintf(stderr, "engine ERROR: only %d bytes read from VME\n", n_read);

        bt_perror(btd, status, "engine ERROR: could not read %s to VME");
	if( what )
	    fprintf(stderr, "engine ERROR: error hint is '%s'\n", what);
        exit(EXIT_FAILURE);
    }
}

// ########################################################################

unsigned int sbs_vmepci1003_read_4(unsigned int address, const char* what=0)
{
    unsigned int out;
    sbs_vmepci1003_read_any(address, &out, 4, what);
    return out;
}

// ########################################################################

void sbs_vmepci1003_write_4(unsigned int address, unsigned int value, const char* what=0)
{
    size_t written = 0; // number of bytes written to device
    bt_error_t status = bt_write(btd, &value, address, 4, &written);
    if( status != BT_SUCCESS ) {
        if( written != 4 )
            fprintf(stderr, "engine ERROR: only %d bytes written to VME\n", written);

        bt_perror(btd, status, "engine ERROR: could not write %s to VME");
	if( what )
	    fprintf(stderr, "engine ERROR: error hint is '%s'\n", what);
        exit(EXIT_FAILURE);
    }
}
#endif /* SUPPORT_SBS_CARD */

// ########################################################################
// ########################################################################

#ifdef SUPPORT_USB_CARD
static bool caen_closed = true;

static void caen_v1718_close()
{
    if( caen_closed )
        return;

    CAEN_V1718_close();
    caen_closed = true;

    printf("CAEN v1718 USB-VME closed\n");
}

static void caen_v1718_open()
{
    if( !caen_closed )
        return;

    CAEN_V1718_open();
    caen_closed = false;

    static bool exit_handler = false;
    if( !exit_handler ) {
        atexit(caen_v1718_close);
        exit_handler = true;
    }

    printf("CAEN v1718 USB-VME opened\n");
}

// ########################################################################

void caen_v1718_read_any(unsigned int address, unsigned int* dest, unsigned int count, const char* what=0)
{
    //printf("read_M32 @ %08x size %d\n", address, count);
    const unsigned int n_read = CAEN_V1718_readM32
	(address, (unsigned char*)dest, count, what?what:"any");

    if( n_read != count )
        fprintf(stderr, "engine ERROR: only %d bytes read from VME\n", n_read);
}

// ########################################################################

unsigned int caen_v1718_read_4(unsigned int address, const char* what=0)
{
    const address32 a(address, false);
    unsigned int v = CAEN_V1718_read(a, 0, what?what:"read_4");
    swap(v);
    //printf("read %08x from %08x\n", v, address);
    return v;
}

// ########################################################################

void caen_v1718_write_4(unsigned int address, unsigned int value, const char* what=0)
{
    const address32 a(address, false);
    swap(value);
    //printf("write %08x to %08x\n", value, address);
    CAEN_V1718_write(value, a, 0, what?what:"write_4");
}
#endif /* SUPPORT_USB_CARD */

// ########################################################################
// ########################################################################

static void (*vme_open)() = 0;
static void (*vme_read_any)(unsigned int, unsigned int*, unsigned int, const char*) = 0;
static unsigned int (*vme_read_4)(unsigned int, const char*) = 0;
static void (*vme_write_4)(unsigned int, unsigned int, const char*) = 0;

void sbs_usb_select(bool use_usb)
{
    if( use_usb ) {
#ifdef SUPPORT_USB_CARD
        vme_open     = caen_v1718_open;
        vme_read_any = caen_v1718_read_any;
        vme_read_4   = caen_v1718_read_4;
        vme_write_4  = caen_v1718_write_4;
#else
#warning "USB support not compiled."
        fprintf(stderr, "Support for USB was not compiled.\n");
	exit(EXIT_FAILURE);
#endif /* !SUPPORT_USB_CARD */
    } else {
#ifdef SUPPORT_SBS_CARD
        vme_open     = sbs_vmepci1003_open;
        vme_read_any = sbs_vmepci1003_read_any;
        vme_read_4   = sbs_vmepci1003_read_4;
        vme_write_4  = sbs_vmepci1003_write_4;
#else
#warning "SBS support not compiled."
        fprintf(stderr, "Support for SBS was not compiled.\n");
	exit(EXIT_FAILURE);
#endif /* !SUPPORT_SBS_CARD */
    }
}

// ########################################################################

bool sbs_usb_status()
{
    return (vme_read_4(VMESTATUS, "vme status") != 0);
}

// ########################################################################

static unsigned int moveaddress = 0, movebytes = 0;

void sbs_usb_open()
{
    // open vme interface device
    vme_open();

    // read bobcat's buffer address and size
    moveaddress = vme_read_4(BUFFER_ADDRESS, "moveaddress");
    movebytes   = vme_read_4(BUFFER_LENGTH,  "movebytes");

    // tell bobcat that we are waiting
    vme_write_4(TIGERSTATUS, 1, "engine status = 1");
}

// ########################################################################

/**
 * Check if a buffer can be fetched from the VME CPU.
 * 
 * @return true, if data is available
 */
bool sbs_usb_check_buffer()
{
    return vme_read_4(SEM[0], "semaphore") != 0;
}

// ########################################################################

/**
 * Fetches a buffer from the VME CPU. It is assumed that a buffer is
 * available, i.e. sbs_usb_check_buffer returned true -- the function
 * will not check if the first part of the buffer is ready.
 * 
 * @return true, if the full buffer was transferred
 */
bool sbs_usb_fetch_buffer(unsigned int* buffer, unsigned bufsize)
{
    extern char leaveprog;

    if(bufsize != 0x8000)
        return false;

    for( int i=0; i<4; ++i ) {
        while(i>0) { // do not
            const int sem_flag = vme_read_4(SEM[i], "semaphore");
            if( sem_flag )
                break;
            if( leaveprog != 'n' )
                return false;

            usleep(1);
            if( vme_read_4(VMESTATUS, "vmestatus in loop") == 0 )
                return false;
        }

        unsigned int* dest = &buffer[i*0x2000];
        vme_read_any(moveaddress, dest, 0x8000, "movebuffer");

        vme_write_4(SEM[i], 0, "semaphore empty");
    }
    return true;
}

// ########################################################################

void sbs_usb_close()
{
    // tell bobcat that engine is not running
    vme_write_4(TIGERSTATUS, 0, "engine status = 0");
}

// ########################################################################
// ########################################################################
// ########################################################################
// ########################################################################
// ########################################################################

#else  /* !(defined(SUPPORT_USB_CARD) || defined(SUPPORT_SBS_CARD)) */

#include <fstream>
#include <sys/time.h>

static timeval dummy_last;
static std::ifstream dummy_file;

void sbs_usb_select(bool)
{
}

bool sbs_usb_status()
{
    return true;
}

void sbs_usb_open()
{
    dummy_file.open("dummy.data");
    if( dummy_file )
        fprintf(stderr, "engine: Opened dummy data file 'dummy.data'.\n");
    else
        fprintf(stderr, "engine: Could not open dummy data file 'dummy.data'.\n");

    gettimeofday(&dummy_last, 0);
}

void sbs_usb_close()
{
    dummy_file.close();
}

bool sbs_usb_check_buffer()
{
    if( !dummy_file )
        return false;

    timeval t;
    gettimeofday(&t, 0);
    if( t.tv_sec - dummy_last.tv_sec + 1e-6 * (t.tv_usec - dummy_last.tv_usec) > 1 ) {
        dummy_last = t;
        return true;
    } else {
        return false;
    }
}

bool sbs_usb_fetch_buffer(unsigned int* buffer, unsigned bufsize)
{
    bool have_data = false;
    if( dummy_file.read((char*)buffer, bufsize*sizeof(buffer[0])) ) {
        have_data = true;
    } else {
        fprintf(stderr, "engine: Rewind dummy data file.\n");
        dummy_file.clear();
        dummy_file.seekg(0, std::ios::beg);
        if( dummy_file.read((char*)buffer, bufsize*sizeof(buffer[0])) )
            have_data = true;
    }
    if( !have_data ) {
        memset(buffer, 0, bufsize*sizeof(buffer[0]));
        buffer[0] = 0x80; // swapped end-of-buffer
        fprintf(stderr, "engine: 'Fetched' empty dummy buffer.\n");
    } else {
        // swap data
        for( unsigned int i=0; i<bufsize; ++i )
            swap( buffer[i] );
        fprintf(stderr, "engine: Fetched dummy data buffer.\n");
    }
    return true;
}

#endif /* !(defined(SUPPORT_USB_CARD) || defined(SUPPORT_SBS_CARD)) */
