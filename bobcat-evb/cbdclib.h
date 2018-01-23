
#ifndef ces_cbd_cbdclib_h
#define ces_cbd_cbdclib_h 1

/* extracted from /usr/src/ces/examples/cbd/cbdclib.c on bobcat (Lynx OS) */

#ifdef __cplusplus
extern "C" {
#if 0
} /* to make emacs indentation work nicely */
#endif
#endif

typedef unsigned short ushort;

extern long copen();
extern long cclose();
extern long cdreg(long *ext, long b, long c, long n, long a);
extern long cdlam(long *lam, long b, long c, long n, long a, long *inta);

extern long cccc(register long ext);
extern long ccci(register long ext, register long l);
extern long cccd(register long ext, register long l);
extern long cccz(register long ext);
extern long cclc(register long lam);
extern long cclm(register long lam, register long l);
extern long ctci(register long ext, register long *l);
extern long ctcd(register long ext, register long *l);
extern long ctgl(register long ext, register long *l);
extern long ctlm(register long lam, register long *l);
extern long cssa(register long f, register long ext, register ushort *d, long *q);
extern long cfsa(register long f, register long ext, register ushort *d, long *q);
extern long csubr(register long f, register long ext, register ushort *d, long *cb);
extern long cfubr(register long f, register long ext, register ushort *d, long *cb);
extern long csubc(register long f, register long ext, register ushort *d, long *cb);
extern long cfubc(register long f, register long ext, register ushort *d, long *cb);

#ifdef __cplusplus
#if 0
{ /* to make emacs indentation work nicely */
#endif
}
#endif

#endif /* ces_cbd_cbdclib_h */
