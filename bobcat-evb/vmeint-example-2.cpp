// from http://svnweb.cern.ch/world/wsvn/fi/General/Projects/Logic_Analyser/CPP/ppc-x86-arch/LA_server/lynxos_make/vmeapi/vmeapi/vmeapi.c?rev=146&sc=1

/* vmeapi.c Simple functions to access the VME */
/* Rick Bourner CERN SL/BI 1998 */

/* #define DIAG 1 */


/*  Date           Changes                                    Author        */
/*  15-Dec-1998    Token names and interface                                */
/*                 variable names changed.                    Rick Bourner  */          
/*  16-Dec-1998    Error mode set to default before init      Rick Bourner  */
/*   9-Feb-1999    Error String VME_ErrStr added to provide                 */
/*                 access to the error description without                  */
/*                 the need for curses windows                Rick Bourner  */
/*   9-Feb-1999    Added warning if VME_Init() has not been                 */
/*                 called.                                    Rick Bourner  */
/*   9-Feb-1999    Fixed bug that I introduced today.         Gordon Spamsy */
/*   1-Mar-1999    Removed menu functions placed in vmemenu.c Rick Bourner  */
/*   8-Mar-1999    Extended vmeapi interface to level of RVME Rick Bourner  */
/*  23-Mar-1999    Moved error state variable to header       Rick.         */
/*  12-May-2000    add re-catching SIGBUS in sig_catch handler A. Guerrero  */

#include "../global/vmeapi.h"
#include "../global/vmetypes.h"
#include <stdio.h>
#include <stdlib.h>
#include <sys/file.h>
#include <string.h>
#include <ctype.h>
#include <ces/vmelib.h>
#include <ces/vmecmd.h>
#include <signal.h>
#include <stdarg.h>
#include <sys/ioctl.h>
#include <vmelib.h>			/* compiler -I/usr/include/headers_ppc/ces */

extern int find_controller(int a, int b, int c, int d, int e, struct pdparam_master *);
extern int return_controller(volatile void *a, int b);

#ifdef __cplusplus
extern "C" int find_controller(int a, int b, int c, int d, int e, struct pdparam_master *);
extern "C" int return_controller(volatile void *a, int b);
#endif

enum bool {FALSE, TRUE };

static struct pdparam_master param = {
	 1,	/* no vme iack */
	 0,	/* No VME read prefetch */
	 0,	/* No VME write posting */
	 1,	/* autoswap */
	 0,	/* page descriptor number */
	 0,	
	 0,	
	 0 
};

/* Driver and interrupt definitions / includes */
static struct sigaction act, oact;
static struct vme_conf vme_conf;
static vme_int_cmd_t int_cmd;
static char use_int;            /* TRUE = ok to use interrupts */
#define DEFAULT_TIME_OUT 30     /* 30 secs for to */
#define pathtoVME "/dev/vme"    /* Driver path */    
static int vme_fd;              /* Driver file descriptor */


static int InitCalled = FALSE;          /* Used to display error if VME_Init has not been called */
const  int VME24 = VME24BIT;            /* Address modifier for 16 bit addressing */
const  int VME16 = VME16BIT;            /* Address modifier for 24 bit addressing */
static volatile void *VME_24_PTR;       /* Holds pointer to base of 24 bit VME memory */
static volatile void *VME_16_PTR;       /* Holds pointer to base of 16 bit VME memory */
const  int BYTESZ = BYTESIZE;           /* Size of a byte in bytes */
const  int WORDSZ = WORDSIZE;           /* Size of a word in bytes */
const  int LONGSZ = LONGSIZE;           /* Size of a long in bytes */
int    error_mode = DEFAULT_ERROR_MODE; /* Error mode variable set to QUIET / LOUD as defined in vmetypes.h */
static char tmp_errstr[256];            /* Temporary error string used to format data before being copied to VMEERRSTR */

/* Variables accessable to the calling program as 
   declared in VMEAPI.h
*/
unsigned long VME_ErrNo;               /* Holds the error code decleared in VMEAPI.h */
char *VME_ErrStr = NULL;               /* Holds current error condition declard in VMEAPI.h */
LONG VME_ByteBlockLen = 0;
LONG VME_WordBlockLen = 0;
LONG VME_LongBlockLen = 0;
LONG VME_FloatBlockLen = 0;
LONG VME_DoubleBlockLen = 0;
BYTE *VME_ByteBlock = NULL;
WORD *VME_WordBlock = NULL;
LONG *VME_LongBlock = NULL;
float *VME_FloatBlock = NULL;
double *VME_DoubleBlock = NULL;

int VME_IsErr() { return !no_err; }
int VME_NoErr() { return no_err; }

void check_init()
{
	 /* Check that VME_Init() has been called */
	 if (!InitCalled){
		  printf("vmeapi: VMEAPI access is not initalized, call VME_Init()\n");
		  exit(ERROR);
	 }
}

void set_error(int err, char *error_string)
{
 	 /* set the error string and the number */
	 VME_ErrNo = err;
	 strcpy(VME_ErrStr, error_string);
	 if (error_mode)
		  VME_DispErr();
}

void VME_ErrQuiet() { error_mode = QUIET; }
void VME_ErrLoud()  { error_mode = LOUD;  }

static void show_err_txt()
{
	 /* This function is to be called when no windows are set */
	 fprintf(stderr, "vmeapi: %s", VME_ErrStr);
}

void VME_DispErr()
{
	 /* If there is an error then display it */
	 /* As either text or a window */
	 if (VME_NoErr())
		  return;
	 if (DISP_MODE == TXT)
		  show_err_txt();
	 else
		  show_err_win();
}


static void sig_catch(int sig)
{
	 /* Exception handler */
	 ADMOD bus_admod = 0;                                 
	 ADDRESS bus_offset = 0;

	 void VME_Release();
	 switch(sig){
	 case SIGBUS  : 
		  sprintf(tmp_errstr, "VME Bus error admod: 0x%02x offset 0x%08x.\n", bus_admod & 0xff, bus_offset); 
		  set_error(BUS, tmp_errstr); 
		  /* Install Bus Error signal handler */
		  if (signal(SIGBUS, &sig_catch) == (void *) -1)
			   set_error(NOSIGBUS, "Unable to install bus signal handler.\n" );
		  break;
	 case SIGINT  :
	 case SIGTERM :
		  printf("\nVME safe exit.\n");
		  VME_Release(); 
		  if (VME_IsErr()) {
			   printf("Errors at VME_Release():\n");
			   VME_DispErr();
		  }
		  exit(0); 
		  break;
	 default :  break;
	 }
}
static void timeout_handler() {/* do nothing */}


static void chk_ptr(int ad_mod, long offset)
{
	 /* Check that various aspects of the pointer and vme access are okay
        before allowing access to the pointers memory location */

	 ADMOD bus_admod = 0;                                 
	 ADDRESS bus_offset = 0;

	 clr_err; /* ensure that no current errors are set */
	 bus_admod = ad_mod;
	 bus_offset = offset;
	 
	 /* no negative offsets */
	 if (offset < 0) {
		  sprintf(tmp_errstr, "Bad offset: Accessing 0x%02x at 0x%08x\n", ad_mod, offset); 
		  set_error(BOFF, tmp_errstr);
	 }
	 /* Check to see if vme has been mapped */
	 if (ad_mod == VME24BIT && VME_24_PTR == NULL){
		  sprintf(tmp_errstr, "VME24 not mapped: Accessing 0x%02x at 0x%08x\n", ad_mod, offset); 
		  set_error(NVME24, tmp_errstr);
	 }
	 if (ad_mod == VME16BIT && VME_16_PTR == NULL){
		  sprintf(tmp_errstr, "VME16 not mapped: Accessing 0x%02x at 0x%08x\n", ad_mod, offset); 
		  set_error(NVME16, tmp_errstr);
	 }
	 /* Check if the address modifier is known */
	 if ((ad_mod != VME16BIT) && (ad_mod != VME24BIT)){
		  sprintf(tmp_errstr, "Bad address modifier: Accessing 0x%02x at 0x%08x\n", ad_mod, offset); 
		  set_error(BADMOD, tmp_errstr);
	 }
	 /* If access is within bounds then continue */
	 if (((ad_mod == VME24BIT)&&(offset >= VME24WSIZE)) || ((ad_mod == VME16BIT) && (offset >= VME16WSIZE))){
		  sprintf(tmp_errstr, "Pointer out of range: Accessing 0x%02x at 0x%08x\n", ad_mod, offset); 
		  set_error(POINTER, tmp_errstr);
	 }
	 /* Automatic error reporting if required */
	 if (error_mode && VME_IsErr())
		  VME_DispErr();
}


void VME_Init()
{  
	 /* initlise access to VME */
	 clr_err;
	 /* allocate memory for errors */
	 if ( (VME_ErrStr = malloc(1024)) == NULL){
		  fprintf(stderr, "Fatal error allocatin memeory for error string\n");
		  exit(ERROR);
	 }
	 
	 /* First install Bus Error signal handler */
	 if (signal(SIGBUS, &sig_catch) == (void *) -1)
		  set_error(NOSIGBUS, "Unable to install bus signal handler.\n" );
	 /* install <CTRL>C signal handler */
	 if (signal(SIGINT, &sig_catch) == (void *) -1)
		  set_error(NOSIGCC, "Unable to install ^C signal handler.\n" );
	 /* install sigterm safe exit */
	 if (signal(SIGTERM, &sig_catch) == (void *) -1)
		  set_error(NOSIGT, "Unable to install safe term signal handler.\n");
	 /* First try to map the 24 bit addressed VME on failure return -1  */
	 VME_16_PTR = VME_24_PTR = NULL;
	 VME_24_PTR = (void *) find_controller(0x0,  VME24WSIZE, VME24BIT, 0, 0, &param); 
	 if (VME_24_PTR == (void *)  -1){
		  VME_24_PTR = NULL;
		  set_error(A24INIT, "Error mapping VME24 addressing.\n");
	 }
	 /* Now try to map the 16 bit addressed VME on failure return -1 */
	 VME_16_PTR = (void *) find_controller(0x0, VME16WSIZE, VME16BIT, 0, 0, &param);
	 if (VME_16_PTR == (void *) -1){
		  VME_16_PTR = NULL;
		  set_error(A16INIT, "Error mapping VME16 addressing.\n");		  
	 }
	 /* Connect to VME driver */
	 if ((vme_fd = open(pathtoVME, O_RDWR)) == -1){
		  sprintf(tmp_errstr, "Can't connect to vme driver: %s\n", pathtoVME); 
		  set_error(DRVINIT, tmp_errstr);
		  vme_fd = NULL;
	 }	
	 use_int = TRUE; /* OK to use interrupts */
	 /* install timeout signal handler */
	 act.sa_handler = (void (*)())timeout_handler;
	 if (sigemptyset(&act.sa_mask) == -1) {
		  set_error(TOHAND, "Can't install interrupt timeout handler.\n");
		  use_int = FALSE;
	 }
	 act.sa_flags = (int)0;
	 if (sigaction(SIGALRM,&act,&oact) == -1) {
		  set_error(TOHAND, "Can't install interrupt timeout handler [2].\n");
		  use_int = FALSE;
	 }
	 /* Call init of curses menus */
	 menu_init();
	 /* Automatic error reporting if required */
	 if (error_mode && VME_IsErr())
		  VME_DispErr();
	 InitCalled = TRUE;
}

void VME_Release()
{
	 /* Let go of resources needed by VMEAPI */
	 clr_err;
	 /* Release the VME mapping for both 16 and 24 bit addressing modes */
	 if (return_controller(VME_16_PTR, VME16WSIZE) == -1)
		  set_error(A16REL,"Error releasing VME16 addressing.\n"); 
	 if (return_controller(VME_24_PTR, VME24WSIZE) == -1)
		  set_error(A24REL, "Error releasing VME24 addressing.\n");
	 /* Set pointers to null */
	 VME_16_PTR = VME_24_PTR = NULL;
	 /* disconnect from vme driver */
	 if ( (vme_fd != NULL) && (close(vme_fd) == -1)) {
		  sprintf(tmp_errstr, "Can't release vme driver: %s\n", pathtoVME);
		  set_error(DRVREL, tmp_errstr);		 
	 }	
	 /* Tidy up menu */
	 menu_release();
	 /* Automatic error reporting if required */
	 if (error_mode && VME_IsErr())
		  VME_DispErr();
	 /* Restore memory for block access */
	 if (VME_ByteBlock)
		  free(VME_ByteBlock);
	 if (VME_WordBlock)
		  free(VME_WordBlock);
	 if (VME_LongBlock)
		  free(VME_LongBlock);
	 if (VME_FloatBlock)
		  free(VME_FloatBlock);
	 if (VME_DoubleBlock)
		  free(VME_DoubleBlock);  
	 if (VME_LongBlock)
		  free(VME_LongBlock); 
	 VME_ByteBlock = NULL;   VME_WordBlock = NULL;
	 VME_LongBlock = NULL;   VME_FloatBlock = NULL;
	 VME_DoubleBlock = NULL; VME_LongBlock =NULL;
	 VME_ByteBlockLen = 0;   VME_WordBlockLen = 0;
	 VME_LongBlockLen = 0;   VME_FloatBlockLen = 0;
	 VME_DoubleBlockLen = 0;
	 InitCalled = FALSE;
}
	
BYTE VME_ReadByte(ADMOD ad_mod, ADDRESS offset)
{
	 /* Read a byte from VME */
	 vu_char *act_addr;
	 
	 clr_err;
	 act_addr = (unsigned char *) ((ad_mod == VME16BIT ? VME_16_PTR : VME_24_PTR) + offset);
	 chk_ptr(ad_mod, offset);
	 return VME_NoErr() ? (*act_addr & 0xff) : ERROR;
}


WORD VME_ReadWord(ADMOD ad_mod, ADDRESS offset)
{ 
	 /* Read a word from VME */
	 vu_short *act_addr;
	 
	 clr_err;
	 act_addr = (unsigned short *) ((ad_mod == VME16BIT ? VME_16_PTR : VME_24_PTR) + offset);
	 chk_ptr(ad_mod, offset);
	 return VME_NoErr() ? (*act_addr & 0xffff) : ERROR;
}


LONG VME_ReadLong(ADMOD ad_mod, ADDRESS offset)
{
	 /* Read a long from VME */
	 vu_long *act_addr;
	 
	 clr_err;
	 act_addr = (unsigned long *) ((ad_mod == VME16BIT ? VME_16_PTR : VME_24_PTR) + offset);
	 chk_ptr(ad_mod, offset);
	 return VME_NoErr() ? (*act_addr & 0xffffffff) : ERROR;
	 
}


float VME_ReadFloat(ADMOD ad_mod, ADDRESS offset)
{
	 /* Read a floating point number from VME */
	 float *act_addr;
		 
	 clr_err;
	 act_addr = (float *) ((ad_mod == VME16BIT ? VME_16_PTR : VME_24_PTR) + offset);
	 chk_ptr(ad_mod, offset); 		
	 return VME_NoErr() ? *act_addr : ERROR;

}


double VME_ReadDouble(ADMOD ad_mod, ADDRESS offset)
{
	 /* read a double from VME */
	 double *act_addr;
		 
	 clr_err;
	 act_addr = (double *) ((ad_mod == VME16BIT ? VME_16_PTR : VME_24_PTR) + offset);
	 chk_ptr(ad_mod, offset); 		
	 return VME_NoErr() ? *act_addr : ERROR;

}

LONG VME_Read(ADMOD ad_mod, ADDRESS offset, int size)
{	 
	 /* Read from VME the datasize specified */
	 clr_err;	 
	 switch(size){
	 case BYTESIZE : return (long) (VME_ReadByte(ad_mod, offset) & 0xff); break;
	 case WORDSIZE : return (long) (VME_ReadWord(ad_mod, offset) & 0xffff); break;
	 case LONGSIZE : return VME_ReadLong(ad_mod, offset); break;
	 default :
		  set_error(UNKDSZ, "vmeapi: Unknown data size\n");
		  return ERROR;
		  break;
	 }
}


void VME_ReadBlockByte(ADMOD ad_mod, ADDRESS offset, LONG winsz)
{
	 /* read a block of Bytes */
	 BYTE *target, *target_tmp;
	 
	 clr_err; /* Clear errors */
	 /* If Byte block already allocated free the memory*/
	 if (VME_ByteBlock)
		  free(VME_ByteBlock);
	 
	 VME_ByteBlockLen = winsz;
	 if ( (target_tmp = target = malloc(sizeof(BYTE)*winsz)) == NULL) {
		  set_error(MEMALC, "vmeapi: Error allocating memory in read block byte\n");
		  return;
	 }
	 /* Read the block NB automatic error reporting handled by readbyte() */
	 while(winsz-- && no_err) {
		  *target_tmp++ = VME_ReadByte(ad_mod, offset);
		  offset++;
	 }
	 VME_ByteBlock = target;
	 return;
}

void VME_ReadBlockWord(ADMOD ad_mod, ADDRESS offset, LONG winsz)
{
	 /* read a block of words from vme */
	 WORD *target, *target_tmp;
	 
	 clr_err;	/* Clear errors */
	 if (VME_WordBlock)
		  free(VME_WordBlock);
	 VME_WordBlockLen = winsz;
	 if ( (target_tmp = target = malloc(sizeof(WORD)*winsz)) == NULL) {
		  set_error(MEMALC, "vmeapi: error allocating memry in read block word\n");
		  VME_WordBlockLen = 0;
		  return;
	 }

	 /* Read the block, note automatic error reporting handled by readword() */
	 while(winsz-- && no_err) {
		  *target_tmp++ = VME_ReadWord(ad_mod, offset);
		  offset+=WORDSZ;
	 }
	 VME_WordBlock = target;
	 return;
}

void VME_ReadBlockLong(ADMOD ad_mod, ADDRESS offset, LONG winsz)
{
	 /* read a block of longs */
	 LONG *target, *target_tmp;
	 
	 clr_err;	 /* Clear errors */
	 if (VME_LongBlock) /* Reset the memory block */
		  free(VME_LongBlock); 

	 VME_LongBlockLen = winsz;
	 if ( (target_tmp = target = malloc(sizeof(LONG)*winsz)) == NULL) {
		  set_error(MEMALC, "vmeapi: error allocating memory in read block long\n");
		  return;
	 }
	 /* Read the block, note automatic error reporting handled by readlong() */
	 while(winsz-- && no_err) {
		  *target_tmp++ = VME_ReadLong(ad_mod, offset);
		  offset+=LONGSZ;
	 }
	 VME_LongBlock = target;
	 return;
}

void VME_ReadBlockFloat(ADMOD ad_mod, ADDRESS offset, LONG winsz)
{	 
	 /* read a block of floats */
	 float *target, *target_tmp;
	 
	 clr_err;	 /* Clear errors */
	 if (VME_FloatBlock)          /* Reset the memory block */
		  free(VME_FloatBlock);

	 VME_FloatBlockLen = winsz;
	 if ( (target_tmp = target = malloc(sizeof(float)*winsz)) == NULL) {
		  set_error(MEMALC, "vmeapi: error allocating memory in read block float\n");
		  return;
	 }

	 /* Read the block, note automatic error reporting handled by readfloat() */
	 while(winsz-- && no_err) {
		  *target_tmp++ = VME_ReadFloat(ad_mod, offset);
		  offset+=sizeof(float);
	 }	 
	 VME_FloatBlock = target;
	 return;
}

void VME_ReadBlockDouble(ADMOD ad_mod, ADDRESS offset, LONG winsz)
{	 
	 /* read a block of doubles */
	 double *target, *target_tmp;
	 
	 clr_err;	 /* Clear errors */
	 if (VME_DoubleBlock)         /* Reset the memory block */
		  free(VME_DoubleBlock); 

	 VME_DoubleBlockLen = winsz;
	 if ( (target_tmp = target = malloc(sizeof(double)*winsz)) == NULL) {
		  set_error(MEMALC, "vmeapi; error allocating memory in read block double\n");
		  return;
	 }
	
	 /* Read the block, note automatic error reporting handled by readdouble() */
	 while(winsz-- && no_err) {
		  *target_tmp++ = VME_ReadDouble(ad_mod, offset);
		  offset+=sizeof(double);
	 }	
	 VME_DoubleBlock = target;
	 return;
}


void VME_WriteByte(ADMOD ad_mod, ADDRESS offset, BYTE write_value)
{
	 /* Write a byte to VME */
	 vu_char *act_addr;
	 
	 clr_err; /* clear errors */
	 chk_ptr(ad_mod, offset);      /* Check pointer details */
	 if (VME_IsErr()) return;      /* Return when in error  */
	 act_addr = (unsigned char *) ((ad_mod == VME16BIT ? VME_16_PTR : VME_24_PTR) + offset);
	 *act_addr = write_value;
}


void VME_WriteWord(ADMOD ad_mod, ADDRESS offset, WORD write_value)
{	 
	 /* Write a word to VME */
	 short *act_addr;
	 
	 clr_err;     /* clear errors */
	 chk_ptr(ad_mod, offset);    /* Check pointer details */
	 if (VME_IsErr()) return;    /* Return when in error  */
	 act_addr = (unsigned short *) ((ad_mod == VME16BIT ? VME_16_PTR : VME_24_PTR) + offset);
	 *act_addr = write_value;
}


void VME_WriteLong(ADMOD ad_mod, ADDRESS offset, LONG write_value)
{ 	
	 /* Write a long to VME */
	 vu_long *act_addr;
	 
	 clr_err;     /* clear errors */
	 chk_ptr(ad_mod, offset);    /* Check pointer details */
	 if (VME_IsErr()) return;    /* Return when in error  */
	 act_addr = (unsigned long *) ((ad_mod == VME16BIT ? VME_16_PTR : VME_24_PTR) + offset);
	 *act_addr = write_value;
}

void VME_WriteFloat(ADMOD ad_mod, ADDRESS offset, float write_value)
{	 
	 /* Write a float to VME */
	 float *act_addr;
		 
	 clr_err;     /* clear errors */
	 chk_ptr(ad_mod, offset);    /* Check pointer details */
	 if (VME_IsErr()) return;    /* Return when in error  */
	 act_addr = (float *) ((ad_mod == VME16BIT ? VME_16_PTR : VME_24_PTR) + offset);
	 *act_addr = write_value;
}



void VME_WriteDouble(ADMOD ad_mod, ADDRESS offset, double write_value)
{	 
	 /* Write a double to VME */
	 double *act_addr;
		 
	 clr_err;     /* clear errors */
	 chk_ptr(ad_mod, offset);    /* Check pointer details */
	 if (VME_IsErr()) return;    /* Return when in error  */
	 act_addr = (double *) ((ad_mod == VME16BIT ? VME_16_PTR : VME_24_PTR) + offset);
	 *act_addr = write_value;
}

void VME_Write(ADMOD ad_mod, ADDRESS offset, LONG write_value, int size)
{
	 /* Write bytes, words or longs to VME */
	 clr_err; /* Clear errors */
	 switch(size){ /* Call relevant function */
	 case BYTESIZE : VME_WriteByte(ad_mod, offset, (char) (write_value & 0xff)); break;
	 case WORDSIZE : VME_WriteWord(ad_mod, offset, (short) (write_value & 0xffff)); break;
	 case LONGSIZE : VME_WriteLong(ad_mod, offset, write_value); break;
	 default :  
		  set_error(UNKDSZ, "vmeapi: unknown data size\n");
		  break;
	 }
}

void VME_WriteBlockByte(ADMOD ad_mod, ADDRESS offset,  LONG winsz, BYTE write_value)
{    
	 /* Write a block of bytes to VME */
	 clr_err;
	 while (winsz-- && no_err) {
		  VME_WriteByte(ad_mod, offset, write_value);
		  offset++;
	 }
}

void VME_WriteBlockWord(ADMOD ad_mod, ADDRESS offset, LONG winsz, WORD write_value)
{
	 /* Write a block of words to vme */
	 clr_err;
	 while (winsz-- && no_err) {
		  VME_WriteWord(ad_mod, offset, write_value);
		  offset+=WORDSZ;
	 }
}


void VME_WriteBlockLong(ADMOD ad_mod, ADDRESS offset,  LONG winsz, LONG write_value)
{
	 /* Write a block of longs to VME */
	 clr_err;
	 while (winsz-- && no_err) {
		  VME_WriteLong(ad_mod, offset, write_value);
		  offset+=LONGSZ;
	 }
}

void VME_WriteBlockFloat(ADMOD ad_mod, ADDRESS offset, LONG winsz, float write_value)
{
	 /* Write a block f floats to vme */
	 clr_err;
	 while (winsz-- && no_err) {
		  VME_WriteFloat(ad_mod, offset, write_value);
		  offset++;
	 }
}

void VME_WriteBlockDouble(ADMOD ad_mod, ADDRESS offset, LONG winsz, double write_value)
{
	 /* Write a block of doubles to vme */
	 clr_err;
	 while (winsz-- && no_err) {
		  VME_WriteDouble(ad_mod, offset, write_value);
		  offset+=sizeof(double);
	 }
}

void VME_WriteBlock(ADMOD ad_mod, ADDRESS offset, LONG write_value, LONG winsz, int size)
{
	 /* Write a block of various sizes to vme */
	 clr_err;
	 while (winsz-- && no_err) {
		  VME_Write(ad_mod, offset, write_value, size);
		  offset+=size;
	 }
}



void VME_WaitInt(int lev, int vec, int time_out)
{
	 /* Wait for an interrupt */
	 /* Check that vme access has been initalized */
	 clr_err; /* clear current errors */
	 check_init(); /* Check that vme access has been initalized */
	 if (!use_int) { /* Check that we can access the interrupts */
		  set_error(NODRV, "Attempt to wait for interrupt with no VME driver access.\n");
		  return;
	 }
	 if (time_out < 0) time_out = DEFAULT_TIME_OUT;
	 
	 /* enable ingterrupt level */
	 vme_conf.ilev = ( 1 << lev ); 
	 vme_conf.ivec = vec; 
	 vme_conf.acfail = 0; 
	 vme_conf.sysfail = 0; 
	 if (ioctl(vme_fd, VME_CONF, &vme_conf) == -1) {
		  sprintf(tmp_errstr, "Can't enable interrupt level %d.\n", (int) lev);
		  set_error(LEVENB, tmp_errstr);
		  return;
	 }
	 
	 /* link to interrupt */
	 int_cmd.vector = vec;
	 if (ioctl(vme_fd, VME_INT_LINK, &int_cmd) != 0) {
		   sprintf(tmp_errstr, "Unable to link to interrupt %d, level: %d.\n", (int) vec, (int) lev);
		   set_error(NOLNK, tmp_errstr);		  
		  return;
	 }
	 
	 /* set timeout-alarm */
	 alarm(time_out);
	 /* wait for interrupts */
	 if (ioctl(vme_fd, VME_INT_WAIT, &int_cmd) == -1) {
		  sprintf(tmp_errstr, "Error waiting for interrupt %d, level: %d.\n", (int) vec, (int) lev);
		  set_error(INTWAIT, tmp_errstr);
	 }
	 /*  unlink from interrupt */
	 if (ioctl(vme_fd, VME_INT_ULNK, &int_cmd) == -1) 
		  set_error(UNLNK, "Error unlinking from interrupt\n");
	
}

int VME_ExecuteCommand(char *command)
{
	 /* execute command on remote machine */
	 LONG exit_status = 0;
	 char tmp_str[512];
	 
	 /* execute a command issued fro client */
	 exit_status = system(command);
	 if (exit_status == 127){
		  set_error(CEXITSTAT, "vmeapi: failed shell fork for execute command");
		  return NULL;
	 }
	 if (exit_status != VFOK){
		  /* Note that the status set may not be an error code. */
		  sprintf(tmp_str, "vmeapi: shell command returned status %d.\n", exit_status);
		  set_error(CEXITSTAT, tmp_str);
	 }
	 return exit_status;
}


BYTE *VME_ReadFile(BYTE *filename, LONG *file_sz)
{
	 /* read a file form the remote server */
	 FILE *fd = NULL;
	 long f_size = 0;
	 BYTE *return_ptr = NULL;

	 /* open file */
	 if ( (fd = fopen(filename, "rb")) == NULL){
		  set_error(FOPEN, "vmeapi: can not open file for read");
		  return NULL;
	 }
	 /* get file size */
	 if (fseek(fd, 0, SEEK_END)){
		  set_error(FSEEK, "vmeapi: error seeking end of file");
		  fclose(fd);
		  return NULL;
	 }
	 if ( (f_size = ftell(fd)) == -1L){
		  set_error(FTELL, "vmeapi: error in ftell");
		  fclose(fd);
		  return NULL;
	 } 
	 if (fseek(fd, 0, SEEK_SET)){
		  set_error(FSEEK, "vmeapi: error seeking beginning of file");
		  fclose(fd);
		  return NULL;
	 }
	 /* allocate buffer */
	 if ( (return_ptr = malloc(f_size)) == NULL){
		  set_error(MEMALC, "vmepapi: Error allocating memory for return packet");
		  fclose(fd);
		  return NULL;
	 }
	 /* read file */
	 if ( fread(return_ptr, sizeof(BYTE), f_size, fd) != f_size){
		  set_error(READF, "vmeapi: error reading file");
		  fclose(fd);
		  free(return_ptr);
		  return NULL;
	 }
	 fclose(fd);
	 *file_sz = f_size;
	 return return_ptr;
}

/* to stay in line with rvmeapi functions this null function has been added */
void VME_SetRemoteHost(char *new_hostname){}

