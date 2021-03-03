#ifndef USB_COM_HANDLE_H
#define USB_COM_HANDLE_H
#include <iostream>
#include <string>

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

extern volatile uint8_t exit_signal;
extern volatile uint16_t data_buf[6];

void usb_COM_handle_thread();

#endif