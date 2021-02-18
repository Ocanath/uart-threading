#ifndef USB_COM_HANDLE_H
#define USB_COM_HANDLE_H
#include <iostream>
#include <string>

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#define NUM_FLOATS_READ 1	//number of frames on the robot. Zeroeth index is an align word.

#define LOG_SIZE 3000

extern volatile float q_share[];
extern volatile float tau_share[];
extern volatile uint8_t r_share_mutex;
extern uint8_t exit_signal;
extern volatile float data_log[NUM_FLOATS_READ][LOG_SIZE];

void usb_COM_handle_thread();

#endif