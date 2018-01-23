
#ifndef ces_vme_vlib_h
#define ces_vme_vlib_h 1

/* extracted from /usr/src/ces/examples/vme/vlib.c on bobcat (Lynx OS) */

#ifdef __cplusplus
extern "C" {
#if 0
} /* to make emacs indentation work nicely */
#endif
#endif

const unsigned int AM24 = 0x39; // address modifier for 24 bits
const unsigned int AM32 = 0x09; // address modifier for 32 bits

typedef unsigned long uint;

extern uint vme_map(uint vaddr, uint wsiz, uint am);
extern uint vme_rel(uint laddr, uint wsiz);

#ifdef __cplusplus
#if 0
{ /* to make emacs indentation work nicely */
#endif
}
#endif

#endif /* ces_vme_vlib_h */
