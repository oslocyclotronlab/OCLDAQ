// -*- c++ -*-

#ifndef usb_vme_H
#define usb_vme_H 1

struct address32 {
    const unsigned long a;
    const bool am32;
    inline address32(unsigned long l, bool am_32=false) : a(l), am32(am_32) {}
    address32 operator[](int idx) const { return address32(a+4*idx, am32); }
    
    // unsigned long read(unsigned long base=0) const;
};
struct address16 {
    const unsigned long a;
    const bool am32;
    inline address16(unsigned long l, bool am_32=false) : a(l), am32(am_32) {}
    address16 operator[](int idx) const { return address16(a+2*idx, am32); }
    
    // unsigned short read(unsigned long base=0) const;
};
struct address8 {
    const unsigned long a;
    const bool am32;
    inline address8(unsigned long l, bool am_32=false) : a(l), am32(am_32) {}
    address8 operator[](int idx) const { return address8(a+1*idx, am32); }
    
    // unsigned char read(unsigned long base=0) const;
};

void CAEN_V1718_open();
extern void CAEN_V1718_close();

extern void CAEN_V1718_write(unsigned char  data, address8  a, unsigned long base=0, const char* errmsg = "write_8");
extern void CAEN_V1718_write(unsigned short data, address16 a, unsigned long base=0, const char* errmsg = "write_16");
extern void CAEN_V1718_write(unsigned long  data, address32 a, unsigned long base=0, const char* errmsg = "write_32");

extern unsigned char  CAEN_V1718_read(address8  a, unsigned long base=0, const char* errmsg = "read_8");
extern unsigned short CAEN_V1718_read(address16 a, unsigned long base=0, const char* errmsg = "read_16");
extern unsigned long  CAEN_V1718_read(address32 a, unsigned long base=0, const char* errmsg = "read_32");

extern int CAEN_V1718_readM32(unsigned long address, unsigned char* out, int count,
                              const char* errmsg="read_M32");

#endif /* usb_vme_H */


/* for emacs */
/*** Local Variables: ***/
/*** c-basic-offset:4 ***/
/*** indent-tabs-mode: nil ***/
/*** End: ***/
