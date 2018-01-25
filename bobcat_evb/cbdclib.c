/******************************************************************
* file: cbdclib.c
* 931028 MAWE
* 950914 MAWE adapted to RTPC
* 950914 MAWE ctci,ctcd,ctgl,cssa added
* 950915 MAWE csubr,cfubr,csubc,cfubc added
* 950915 MAWE last update
*
* small ESONE library for CBD 8210
*
* routines that access CAMAC return -1 if X=0, otherwise they
* return the state of Q
******************************************************************/
#include <stdio.h>

#define F(e,f) \
  {(e) &= ~0x7C;(e) |= ((f)<<2)|2;}
#define NAF(e,n,a,f) \
  {(e) &= ~0xFFFF;(e) |= ((((((n)<<4)|(a))<<5)|(f))<<2)|2;}

/*The old verison to be used if "cbd" is aligned to 4 MBytes (0x00400000)*/
#define BCNAF(e,b,c,n,a,f) \
  {(e) &= ~0x3FFFFF;(e) |= ((((((((((b)<<3)|(c))<<5)|(n))<<4)|(a))<<5)|(f))<<2)|2;}
#define CSR(e) \
  (*(ushort *)(((e)&~0x7FFFF)|0xE802))

/*Here is one alternative new version aligned to to 64 Kbytes (0x10000):

#define BCNAF(e,b,c,n,a,f) \
{int e1=0; e1=((((((((((b)<<3)|(c))<<5)|(n))<<4)|(a))<<5)|(f))<<2)|2;(e) += e1;}
#define CSR(e) \
(*(ushort *)((cbd+((e)-cbd)&~0x7FFFF)|0xE802))
*/

#define STATUS(s,q) \
  ((q)=(s)>>15,((s)&0x4000)?(q):-1)
#define Q(s) \
  ((s)&0x8000)
 
/*
#define WAIT \
  {int i; for(i=100;i;i--);}   this is 100*0.03 =3 us */ 

    
#define WAIT  
  

/*
typedef unsigned short ushort;
*/
/*
*===============================================================
* global pointer
*---------------------------------------------------------------
*/
static long cbd;
/*
*===============================================================
* initialize access
*---------------------------------------------------------------
*/
long copen()
{
	/*if (!(cbd = (long)vme_map(0x00800000,0x00400000,0x39))) {
		fprintf(stderr,"linktrap: unable to allocate VME window\n");
		exit(0);
	}*/


    cbd = 0xde800000; /* VME A24 mapped betwee 0xde000000..0xdeffffff */

      printf("The cbd in copen() has value (without wait):  0x%x \n",cbd);
	return(0);

}
/*
*===============================================================
* initialize access
*---------------------------------------------------------------
*/
long cclose()
{
/*	vme_rel(cbd,0x00400000);*/
	return(0);
}
/*
*===============================================================
* CDREG - declare CAMAC register
*---------------------------------------------------------------
*/
long cdreg(ext,b,c,n,a)
long *ext,b,c,n,a;
{
	register long e;
	
	e = cbd;
	BCNAF(e,b,c,n,a,0)
	*ext = e;
	return(0);
}
/*
*===============================================================
* CDLAM - declare LAM
*---------------------------------------------------------------
*/
long cdlam(lam,b,c,n,a,inta)
long *lcbam,b,c,n,a,*inta;
{
	register long e;
	
	e = cbd;
	BCNAF(e,b,c,n,a,0)
	*lam = e;
	return(0);
}
/*
*===============================================================
* CCCC - generate crate clear
*---------------------------------------------------------------
*/
long cccc(ext)
register long ext;
{
	register ushort s;
	register long q;

	NAF(ext,28,9,26)
	
	WAIT
	s = *(ushort *)ext;
	WAIT
	s = CSR(ext);
	return(STATUS(s,q));
}
/*
*===============================================================
* CCCI - set/clear dataway inhibit
*---------------------------------------------------------------
*/
long ccci(ext,l)
register long ext,l;
{
	register ushort s;
	register long q;

	if (l) {
		NAF(ext,30,9,26)
	}else{
		NAF(ext,30,9,24)
	}
	WAIT
	s = *(ushort *)ext;
	WAIT
	s = CSR(ext);
	return(STATUS(s,q));
}
/*
*===============================================================
* CCCD - enable/disable crate demand
*---------------------------------------------------------------
*/
long cccd(ext,l)
register long ext,l;
{
	register ushort s;
	register long q;

	if (l) {
		NAF(ext,30,10,26)
	}else{
		NAF(ext,30,10,24)
	}
	WAIT
	s = *(ushort *)ext;
	WAIT
	s = CSR(ext);
	return(STATUS(s,q));
}
/*
*===============================================================
* CCCZ - generate dataway initialise
*---------------------------------------------------------------
*/
long cccz(ext)
register long ext;
{
	register ushort s;
	register long q;
/*      printf("The ext in cccz (before NAF) has value:  0x%x \n",ext);*/
	NAF(ext,28,8,26)
	WAIT
/*      printf("The ext in cccz (after  NAF) has value:  0x%x \n",ext);
      printf("Hello, trying... \n");*/
	s = *(ushort *)ext;
/*      printf("Hello, you made *(ushort *)ext... \n");*/
	WAIT
	s = CSR(ext);
/*      printf("Hello, you made CSR... \n");*/
	return(STATUS(s,q));
/*      printf("Hello, you made STATUS... \n");*/

}
/*
*===============================================================
* CCLC - clear LAM
*---------------------------------------------------------------
*/
long cclc(lam)
register long lam;
{
	register ushort s;
	register long q;

	F(lam,10)
	WAIT
	s = *(ushort *)lam;
	WAIT
	s = CSR(lam);
	return(STATUS(s,q));
}
/*
*===============================================================
* CCLM - enable/disable LAM
*---------------------------------------------------------------
*/
long cclm(lam,l)
register long lam,l;
{
	register ushort s;
	register long q;

	if (l) {
		F(lam,26)
	}else{
		F(lam,24)
	}
	WAIT
	s = *(ushort *)lam;
	WAIT
	s = CSR(lam);
	return(STATUS(s,q));
}
/*
*===============================================================
* CTCI - test dataway inhibit
*---------------------------------------------------------------
*/
long ctci(ext,l)
register long ext,*l;
{
	register ushort s;
	register long q;

	NAF(ext,30,9,27)
	WAIT
	s = *(ushort *)ext;
	WAIT
	s = CSR(ext);
	return(STATUS(s,*l));
}
/*
*===============================================================
* CTCD - test crate demand enabled
*---------------------------------------------------------------
*/
long ctcd(ext,l)
register long ext,*l;
{
	register ushort s;
	register long q;

	NAF(ext,30,10,27)
	WAIT
	s = *(ushort *)ext;
	WAIT
	s = CSR(ext);
	return(STATUS(s,*l));
}
/*
*===============================================================
* CTGL - test crate demand present
*---------------------------------------------------------------
*/
long ctgl(ext,l)
register long ext,*l;
{
	register ushort s;
	register long q;

	NAF(ext,30,11,27)
	WAIT
	s = *(ushort *)ext;
	WAIT
	s = CSR(ext);
	return(STATUS(s,*l));
}
/*
*===============================================================
* CTLM - test LAM
*---------------------------------------------------------------
*/
long ctlm(lam,l)
register long lam,*l;
{
	register ushort s;

	F(lam,8)
	WAIT
	s = *(ushort *)lam;
	WAIT
	s = CSR(lam);
	return(STATUS(s,*l));
}
/*

/*
*===============================================================
* CSSA - execute CAMAC command (16 bit data)
*---------------------------------------------------------------
*/
long cssa(f,ext,d,q)
register long f,ext;
register ushort *d;
long *q;
{
	register ushort s;
	
	F(ext,f)
	if (f&8) {						/* control */
		WAIT
		s = *(ushort *)ext;
	}else if (f&0x10) {					/* write */
		WAIT
		*(ushort *)ext = *d;
	}else{							/* read */
		WAIT
		*d = *(ushort *)ext;
	}
	WAIT
	s = CSR(ext);
	return(STATUS(s,*q));
}
/*
*===============================================================
* CFSA - execute CAMAC command (24 bit data)
*---------------------------------------------------------------
*/
long cfsa(f,ext,d,q)
register long f,ext;
register ushort *d;
long *q;
{
	register ushort s;
	
	F(ext,f)
	if (f&8) {						/* control */
		WAIT
		s = *(ushort *)ext;
	}else if (f&0x10) {					/* write */
		WAIT
		((ushort *)ext)[-1] = *d++;
		WAIT
		*(ushort *)ext = *d;
	}else{							/* read */
		WAIT
		*d++ = ((ushort *)ext)[-1];
		WAIT
		*d = *(ushort *)ext;
	}
	WAIT
	s = CSR(ext);
	return(STATUS(s,*q));
}
/*
*===============================================================
* CSUBR - blocktransfer in Q-repeat mode (16 bit data)
*---------------------------------------------------------------
*/
long csubr(f,ext,d,cb)
register long f,ext;
register ushort *d;
long *cb;
{
	register ushort s;
	register int i,n;
	
	n=0;
	F(ext,f)
	if (f&8) {
		for(i=cb[0];i;i--,n++) {		/* control */
			do {
				WAIT
				s = *(ushort *)ext;
				WAIT
				s = CSR(ext);
			}while (Q(s)==0);
		}
	}else if (f&0x10) {				/* write */
		for(i=cb[0];i;i--,d++,n++) {
			do {
				WAIT
				*(ushort *)ext = *d;
				WAIT
				s = CSR(ext);
			}while (Q(s)==0);
		}
	}else{						/* read */
		for(i=cb[0];i;i--,d++,n++) {
			do {
				WAIT
				*d = *(ushort *)ext;
				WAIT
				s = CSR(ext);
			}while (Q(s)==0);
		}
	}
	cb[1] = n;
	return(n==cb[0]);
}
/*
*===============================================================
* CFUBR - blocktransfer in Q-repeat mode (24 bit data)
*---------------------------------------------------------------
*/
long cfubr(f,ext,d,cb)
register long f,ext;
register ushort *d;
long *cb;
{
	register ushort s;
	register int i,n;
	
	n=0;
	F(ext,f)
	if (f&8) {
		for(i=cb[0];i;i--,n++) {		/* control */
			do {
				WAIT
				s = *(ushort *)ext;
				WAIT
				s = CSR(ext);
			}while (Q(s)==0);
		}
	}else if (f&0x10) {				/* write */
		for(i=cb[0];i;i--,d+=2,n++) {
			do {
				WAIT
				((ushort *)ext)[-1] = *d;
				WAIT
				*(ushort *)ext = d[1];
				WAIT
				s = CSR(ext);
			}while (Q(s)==0);
		}
	}else{						/* read */
		for(i=cb[0];i;i--,d+=2,n++) {
			do {
				WAIT
				*d = ((ushort *)ext)[-1];
				WAIT
				d[1] = *(ushort *)ext;
				WAIT
				s = CSR(ext);
			}while (Q(s)==0);
		}
	}
	cb[1] = n;
	return(n==cb[0]);
}
/*
*===============================================================
* CSUBC - blocktransfer in Q-stop mode (16 bit data)
*---------------------------------------------------------------
*/
long csubc(f,ext,d,cb)
register long f,ext;
register ushort *d;
long *cb;
{
	register ushort s;
	register int i,n;
	
	n=0;
	F(ext,f)
	if (f&8) {
		for(i=cb[0];i;i--,n++) {		/* control */
			WAIT
			s = *(ushort *)ext;
			WAIT
			s = CSR(ext);
			if (Q(s)==0) break;
		}
	}else if (f&0x10) {				/* write */
		for(i=cb[0];i;i--,d++,n++) {
			WAIT
			*(ushort *)ext = *d;
			WAIT
			s = CSR(ext);
			if (Q(s)==0) break;
		}
	}else{						/* read */
		for(i=cb[0];i;i--,d++,n++) {
			WAIT
			*d = *(ushort *)ext;
			WAIT
			s = CSR(ext);
			if (Q(s)==0) break;
		}
	}
	cb[1] = n;
	return(n==cb[0]);
}
/*
*===============================================================
* CFUBC - blocktransfer in Q-stop mode (24 bit data)
*---------------------------------------------------------------
*/
long cfubc(f,ext,d,cb)
register long f,ext;
register ushort *d;
long *cb;
{
	register ushort s;
	register int i,n;
	
	n=0;
	F(ext,f)
	if (f&8) {
		for(i=cb[0];i;i--,n++) {		/* control */
			WAIT
			s = *(ushort *)ext;
			WAIT
			s = CSR(ext);
			if (Q(s)==0) break;
		}
	}else if (f&0x10) {				/* write */
		for(i=cb[0];i;i--,d+=2,n++) {
			WAIT
			((ushort *)ext)[-1] = *d;
			WAIT
			*(ushort *)ext = d[1];
			WAIT
			s = CSR(ext);
			if (Q(s)==0) break;
		}
	}else{						/* read */
		for(i=cb[0];i;i--,d+=2,n++) {
			WAIT
			*d = ((ushort *)ext)[-1];
			WAIT
			d[1] = *(ushort *)ext;
			WAIT
			s = CSR(ext);
			if (Q(s)==0) break;
		}
	}
	cb[1] = n;
	return(n==cb[0]);
}
