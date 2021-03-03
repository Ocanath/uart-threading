#include "usb-com-handle.h"
#include <Windows.h>

volatile uint8_t exit_signal = 0;

HANDLE serial_handle;
DWORD dwbyteswritten;
DWORD dwbytesread;

static volatile uint8_t read_buf[10] = { 0 };
volatile uint16_t data_buf[6] = { 0 };

int connect_com_port(const char * port)
{
	serial_handle = CreateFileA(port, GENERIC_READ | GENERIC_WRITE, 0, 0, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);
	DCB serial_params = { 0 };
	serial_params.DCBlength = sizeof(serial_params);
	
	GetCommState(serial_handle, &serial_params);
	serial_params.BaudRate = 921600;
	serial_params.ByteSize = DATABITS_8;
	serial_params.StopBits = ONESTOPBIT;
	serial_params.Parity = PARITY_NONE;
	if (!SetCommState(serial_handle, &serial_params))
	{
		printf("Error, serial device not connected\r\n");
		return -1;
	}
	else
		return 0;
}

/*
Generic hex checksum calculation.
TODO: use this in the psyonic API
*/
uint8_t get_checksum(uint8_t* arr, int size)
{

	int8_t checksum = 0;
	for (int i = 0; i < size; i++)
		checksum += (int8_t)arr[i];
	return -checksum;
}

/**/
void unpack_8bit_into_12bit(uint8_t* arr, uint16_t* vals, int valsize)
{
	for (int bidx = valsize * 12 - 4; bidx >= 0; bidx -= 4)
	{
		int validx = bidx / 12;
		int arridx = bidx / 8;
		int shift_val = (bidx % 8);
		vals[validx] |= ((arr[arridx] >> shift_val) & 0x0F) << (bidx % 12);
	}
}


/*
TODO: autofind the com port
*/
void usb_COM_handle_thread()
{
	connect_com_port("\\\\.\\COM4");

	COMMTIMEOUTS timeout = { 0 };
	timeout.ReadIntervalTimeout = 1;
	timeout.ReadTotalTimeoutConstant = 0;
	timeout.ReadTotalTimeoutMultiplier = 0;
	timeout.WriteTotalTimeoutConstant = 50;
	timeout.WriteTotalTimeoutMultiplier = 10;
	SetCommTimeouts(serial_handle, &timeout);
	
	uint64_t uart_tx_ts = 0;
	int log_idx = 0;
	while (exit_signal == 0)
	{
		int isread = ReadFile(serial_handle, (uint8_t*)read_buf, 10, &dwbytesread, NULL);
		if (read_buf[9] == get_checksum( (uint8_t*)read_buf, 9))
		{
			unpack_8bit_into_12bit((uint8_t*)read_buf, (uint16_t*)data_buf, 6);
		}
	}
	CloseHandle(serial_handle);
}


/*
Example of the uart/dongle side format:
float t = (float)HAL_GetTick()*.001f;


floatsend_t align_byte;
align_byte.d[3] = 0xDE;
align_byte.d[2] = 0xAD;
align_byte.d[1] = 0xBE;
align_byte.d[0] = 0xEF;
print_float(align_byte.v);

print_float(PI*sin_fast(t));
print_float(.5f*sin_fast(t)+HALF_PI);
print_float(PI);
print_float(0.0f);
print_float(0.8f*sin_fast(t - HALF_PI));
print_float(t*2);

*/