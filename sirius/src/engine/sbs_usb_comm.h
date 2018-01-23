// -*- c++ -*-

#ifndef SBS_USB_COMM_H
#define SBS_USB_COMM_H 1

void sbs_usb_select(bool use_usb);

bool sbs_usb_status();

void sbs_usb_open();

bool sbs_usb_check_buffer();

bool sbs_usb_fetch_buffer(unsigned int* buffer, unsigned bufsize);

void sbs_usb_close();

#endif /* SBS_USB_COMM_H */
