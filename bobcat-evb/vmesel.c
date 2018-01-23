/*
***************************************************************
*
*      vmesel: CES RTPC 8067 / RIO2 806x VME interrupt examples
*
* vmesel.c: wait for VME interrupts using the 'select' mechanism
*
***************************************************************
*                                                             
*      Copyright 1997, CES Creative Electronic System SA      
*        70, route du Pont-Butin, CH-1213 Petit-Lancy         
*                   All Rights Reserved                       
*                                                             
***************************************************************
*
* $Log: vmesel.c,v $
* Revision 1.1  2001/03/21 12:22:03  martin
* VME examples added to lynx 3.1
*
* Revision 1.1  1999/04/09 15:59:56  martin
* VME interrupt examples added. [MAWE]
*
* Revision 1.1  1998/02/01 19:29:23  ces
* re-constructing Lynx/PPC: CES add-ons [MAWE]
*
* Revision 1.1  1997/10/31 00:23:00  lynx
* added to Lynx v2.5. MAWE
*
* Revision 1.1  1997/08/07 09:40:16  lynx
* VME interrupt examples added to CVS.
* add ioctl(..VME_GET_CONG..) before enabling the interrupts. MAWE.
*
*
***************************************************************
* pre-CVS update history:
*
* 960125 MAWE 
* 961009 MAWE new VME driver: vme_conf.ilev is bitmask 
* 961009 MAWE last update
*
***************************************************************
*/
#ifndef lint
static char *rcsid = "$Header: /sources/ces/lynx3.1/usr/src/ces/examples/vme/vmesel.c,v 1.1 2001/03/21 12:22:03 martin Exp $";
#endif

#include        <kernel.h>
#include        <errno.h>
#include        <sys/ioctl.h>
#include        <st.h>
#include	<sys/file.h>
#include	<sys/types.h>
#include	<time.h>
#include	<signal.h>
#include	<stdio.h>

#include	<ces/vmelib.h>
#include	<ces/vmecmd.h>

#define		EXIT_SUCCESS	0
#define		EXIT_FAILURE	1

#define		AD	(0x02700000)
#define		AM	(0x09)
#define		VC	(0xEF)
#define		LV	(0x03)
/*
*===========================================================================
* globals
*---------------------------------------------------------------------------
*/
char *path_vme = "/dev/vme";
long fd_vme;
long it_cnt;
/*
*===========================================================================
* do it
*---------------------------------------------------------------------------
*/
main(argc,argv)
int argc;
char *argv[];
{
	struct timeval timeout;
	fd_set fdset,fdsav,*ready;
	struct sigaction act, oact;
	struct vme_conf vme_conf;
	vme_int_cmd_t int_cmd;

	long i,ad,am,vc,lv,ct,dt;
	char cdum;
	
	if ((argc==6)&&(sscanf(argv[5],"%x",&ct)==1)) {argc--;} else {ct=1;}
	if ((argc==5)&&(sscanf(argv[4],"%x",&am)==1)) {argc--;} else {am=AM;}
	if ((argc==4)&&(sscanf(argv[3],"%x",&ad)==1)) {argc--;} else {ad=AD;}
	if ((argc==3)&&(sscanf(argv[2],"%x",&lv)==1)) {argc--;} else {lv=LV;}
	if ((argc==2)&&(sscanf(argv[1],"%x",&vc)==1)) {argc--;} else {vc=VC;}
	if (argc != 1) {
		printf("use: %s [<vector>[<level>[<AD>[<AM>[<cnt>]]]]]\n",argv[0]);
		printf("     wait for VME interrupt with <vector>\n");
		printf("     then read address <AD> using address modifier code<AM>\n");
		printf("     <cnt> times, display result\n");
		exit(0);
	}
	/*
	* connect to VME driver
	*/
	if ((fd_vme=open(path_vme, O_RDWR)) == -1)
	{
		perror("open");
		exit(EXIT_FAILURE);
	}

	/*
	* enable level
	*/
	if (ioctl(fd_vme, VME_GET_CONF, &vme_conf) == -1)
	{
		perror("ioctl(..VME_GET_CONF..)");
		exit(EXIT_FAILURE);
	}
	vme_conf.ilev=(1<<lv); 
	vme_conf.ivec=vc; 
	vme_conf.acfail=0; 
	vme_conf.sysfail=0; 
	if (ioctl(fd_vme, VME_SET_CONF, &vme_conf) == -1)
	{
		perror("ioctl(..VME_SET_CONF..)");
		exit(EXIT_FAILURE);
	}
	/*
	* link to interrupt
	*/
	int_cmd.vector = vc;
	if (ioctl(fd_vme, VME_INT_LINK, &int_cmd) == -1)
	{
		perror("ioctl");
		printf("VMESEL> VME_INT_LINK returns status %d\n",int_cmd.status);
		close(fd_vme);
		exit(EXIT_FAILURE);
	}
	/*
	* wait for interrupt
	*/
	printf("VMESEL> waiting for VME interrupt: level=%d, vector=0x%02x or <key-pressed>\n",
        lv,vc);
	it_cnt=0;
	ready = &fdset;
	FD_ZERO(ready);
	FD_SET(fd_vme,ready);
	FD_SET(STDIN,ready);
	fdsav = *ready;
	for(i=ct;i;i--)
	{
		*ready = fdsav;
		if (select ((fd_vme+1),ready,(fd_set*)0,(fd_set*)0,0)!=-1)
		{
   			/* 
			* VME it received -> ready to read 
			*/
			if (FD_ISSET(fd_vme,ready)) 
			{  
				if (ioctl(fd_vme, VME_INT_WAIT, &int_cmd) == -1)
				{
					perror("ioctl");
					printf("VMESEL> VME_INT_WAIT returns status %d\n",int_cmd.status);
					break;
				}
				printf("VMESEL> VME interrupt with vector 0x%02x received\n",
				int_cmd.vector);
				it_cnt++;
			}
   			/* 
			* key pressed
			*/
			if (FD_ISSET(STDIN,ready))
			{
				read(STDIN,&cdum,1);
				printf("VMESEL> VME key pressed - aborting\n");
				break;
			}
		}
		else
		{
			perror("select");
			close(fd_vme);
			exit(EXIT_FAILURE);
		}
	}
	printf("VMESEL> %d VME interrupts with vector 0x%02x received\n",it_cnt,int_cmd.vector);
	/*
	* unlink from interrupt
	*/
	if (ioctl(fd_vme, VME_INT_ULNK, &int_cmd) == -1)
	{
		perror("ioctl");
		printf("VMESEL> VME_INT_ULNK returns status %d\n",int_cmd.status);
		close(fd_vme);
		exit(EXIT_FAILURE);
	}
	/*
	* disconnect from VME driver
	*/
	if (close(fd_vme) == -1) 
	{
		perror("close");
		exit(EXIT_FAILURE);
	}
}
