/*
***************************************************************
*
*      vmesel2: CES RTPC 8067 / RIO2 806x VME interrupt examples
*
* vmesel2.c: wait for 2 different VME interrupts using 
*            the 'select' mechanism
*
***************************************************************
*                                                             
*      Copyright 1997, CES Creative Electronic System SA      
*        70, route du Pont-Butin, CH-1213 Petit-Lancy         
*                   All Rights Reserved                       
*                                                             
***************************************************************
*
* $Log: vmesel2.c,v $
* Revision 1.1  2001/03/21 12:22:04  martin
* VME examples added to lynx 3.1
*
* Revision 1.1  1999/04/09 15:59:56  martin
* VME interrupt examples added. [MAWE]
*
* Revision 1.1  1998/02/01 19:29:27  ces
* re-constructing Lynx/PPC: CES add-ons [MAWE]
*
* Revision 1.1  1997/10/31 00:23:01  lynx
* added to Lynx v2.5. MAWE
*
* Revision 1.1  1997/08/07 09:40:18  lynx
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
static char *rcsid = "$Header: /sources/ces/lynx3.1/usr/src/ces/examples/vme/vmesel2.c,v 1.1 2001/03/21 12:22:04 martin Exp $";
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

#define		VC1	(0xaa)
#define		VC2	(0x55)
#define		LV1	(0x03)
#define		LV2	(0x02)
/*
*===========================================================================
* globals
*---------------------------------------------------------------------------
*/
char *path_vme = "/dev/vme";
long fd_vme1;
long fd_vme2;
long it_cnt1,it_cnt2;
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
	vme_int_cmd_t int_cmd1,int_cmd2;

	long ad,am,vc1,vc2,lv1,lv2,dt;
	char cdum;
	
	if ((argc==5)&&(sscanf(argv[4],"%x",&lv2)==1)) {argc--;} else {lv2=LV2;}
	if ((argc==4)&&(sscanf(argv[3],"%x",&lv1)==1)) {argc--;} else {lv1=LV1;}
	if ((argc==3)&&(sscanf(argv[2],"%x",&vc2)==1)) {argc--;} else {vc2=VC2;}
	if ((argc==2)&&(sscanf(argv[1],"%x",&vc1)==1)) {argc--;} else {vc1=VC1;}
	if (argc != 1) {
		printf("use: %s [<vector1>[<vector2>[<level1>[<level2>]]]]\n",argv[0]);
		printf("     wait in parallel for VME interrupt with <vector1>/<level1>\n");
		printf("     or <vector2><level2>, <CR> terminates\n");
		exit(0);
	}
	/*
	* connect to VME driver
	*/
	if ((fd_vme1=open(path_vme, O_RDWR)) == -1)
	{
		perror("open_1");
		exit(EXIT_FAILURE);
	}
	if ((fd_vme2=open(path_vme, O_RDWR)) == -1)
	{
		perror("open_2");
		exit(EXIT_FAILURE);
	}
	/*
	* enable level
	*/
	if (ioctl(fd_vme1, VME_GET_CONF, &vme_conf) == -1)
	{
		perror("ioctl(..VME_GET_CONF..)");
		exit(EXIT_FAILURE);
	}
	vme_conf.ilev=(1<<lv1); 
	vme_conf.ivec=vc1; 
	vme_conf.acfail=0; 
	vme_conf.sysfail=0; 
	if (ioctl(fd_vme1, VME_SET_CONF, &vme_conf) == -1)
	{
		perror("ioctl_1(..VME_SET_CONF..)");
		exit(EXIT_FAILURE);
	}
	vme_conf.ilev=(1<<lv2); 
	vme_conf.ivec=vc2; 
	if (ioctl(fd_vme2, VME_SET_CONF, &vme_conf) == -1)
	{
		perror("ioctl_2(..VME_SET_CONF..)");
		exit(EXIT_FAILURE);
	}
	/*
	* link to interrupt
	*/
	int_cmd1.vector = vc1;
	if (ioctl(fd_vme1, VME_INT_LINK, &int_cmd1) == -1)
	{
		perror("ioctl_1");
		printf("VMESEL> VME_INT_LINK returns status %d\n",int_cmd1.status);
		exit(EXIT_FAILURE);
	}
	int_cmd2.vector = vc2;
	if (ioctl(fd_vme2, VME_INT_LINK, &int_cmd2) == -1)
	{
		perror("ioctl_2");
		printf("VMESEL> VME_INT_LINK returns status %d\n",int_cmd2.status);
		exit(EXIT_FAILURE);
	}
	/*
	* wait for interrupt
	*/
	printf("VMESEL> waiting for VME interrupt1: \n");
        printf("    level1=%d, vector1=0x%02x or\n",lv1,vc1);
 	printf("    level1=%d, vector1=0x%02x or\n",lv2,vc2);
        printf("    <key-pressed>\n");
	it_cnt1=0;
	it_cnt2=0;
	ready = &fdset;
	FD_ZERO(ready);
	FD_SET(fd_vme1,ready);
	FD_SET(fd_vme2,ready);
	FD_SET(STDIN,ready);
	fdsav = *ready;
	for(;;)
	{
		*ready = fdsav;
		if (select ((fd_vme2+1),ready,(fd_set*)0,(fd_set*)0,0)!=-1)
		{
   			/* 
			* VME it#1 received -> ready to read 
			*/
			if (FD_ISSET(fd_vme1,ready)) 
			{  
				if (ioctl(fd_vme1, VME_INT_WAIT, &int_cmd1) == -1)
				{
					perror("ioctl_1");
					printf("VMESEL> 1: VME_INT_WAIT returns status %d\n",
                                        int_cmd1.status);
					break;
				}
				printf("VMESEL> 1: VME interrupt with vector 0x%02x received\n",
				int_cmd1.vector);
				it_cnt1++;
			}
   			/* 
			* VME it#2 received -> ready to read 
			*/
			if (FD_ISSET(fd_vme2,ready)) 
			{  
				if (ioctl(fd_vme2, VME_INT_WAIT, &int_cmd2) == -1)
				{
					perror("ioctl");
					printf("VMESEL> 2: VME_INT_WAIT returns status %d\n",
                                        int_cmd2.status);
					break;
				}
				printf("VMESEL> 2: VME interrupt with vector 0x%02x received\n",
				int_cmd2.vector);
				it_cnt2++;
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
			exit(EXIT_FAILURE);
		}
	}
	printf("VMESEL> %d VME interrupts with vector 0x%02x received\n",it_cnt1,int_cmd1.vector);
	printf("VMESEL> %d VME interrupts with vector 0x%02x received\n",it_cnt2,int_cmd2.vector);
	/*
	* unlink from interrupt 1
	*/
	if (ioctl(fd_vme1, VME_INT_ULNK, &int_cmd1) == -1)
	{
		perror("ioctl_1");
		printf("VMESEL> VME_INT_ULNK returns status %d\n",int_cmd1.status);
	}
	/*
	* unlink from interrupt 2
	*/
	if (ioctl(fd_vme2, VME_INT_ULNK, &int_cmd2) == -1)
	{
		perror("ioctl_2");
		printf("VMESEL> VME_INT_ULNK returns status %d\n",int_cmd2.status);
	}
	/*
	* disconnect from VME driver
	*/
	if (close(fd_vme1) == -1) 
	{
		perror("close_1");
	}
	/*
	* disconnect from VME driver
	*/
	if (close(fd_vme2) == -1) 
	{
		perror("close_2");
	}
}
