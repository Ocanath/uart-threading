#include "usb-com-handle.h"
#include <Windows.h>

#define HALF_PI 1.57079633f
#define PI 3.14159265359f



volatile float data_log[NUM_FLOATS_READ][LOG_SIZE];


volatile float q_share[NUM_FLOATS_READ];
volatile float tau_share[NUM_FLOATS_READ];
volatile uint8_t r_share_mutex = 0;
uint8_t exit_signal = 0;

#define NUM_RX_BUF_BYTES 4

typedef union
{
	float v[NUM_FLOATS_READ];
	uint8_t d[sizeof(float)* NUM_FLOATS_READ + 1];
}fread_arr_t;

volatile static fread_arr_t read_arr;



HANDLE serial_handle;
DWORD dwbyteswritten;
DWORD dwbytesread;


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


	//float dongle_rx_data[NUM_FLOATS_READ];
	//float dongle_tx_data[NUM_FLOATS_READ];
	
	uint64_t uart_tx_ts = 0;
	int log_idx = 0;
	while (exit_signal == 0)
	{
		int trans_size = sizeof(float) * NUM_FLOATS_READ+1;
		for (int i = 0; i < trans_size; i++)
			read_arr.d[i] = 0;

		int isread = ReadFile(serial_handle, (uint8_t*)(read_arr.d), trans_size, &dwbytesread, NULL);

		if (read_arr.d[trans_size - 1] == get_checksum((uint8_t*)read_arr.d, trans_size - 1))
		{
			for (int i = 0; i < NUM_FLOATS_READ; i++)
			{
				data_log[i][log_idx] = q_share[i];
				q_share[i] = read_arr.v[i];
			}
			log_idx++;
			if (log_idx >= LOG_SIZE)
				exit_signal = 1;
		}

		//if (GetTickCount64() > uart_tx_ts)
		//{
		//	WriteFile(serial_handle, (uint8_t*)(tau_share), trans_size, &dwbyteswritten, NULL);

		//	uart_tx_ts = GetTickCount64() + 10;
		//}
	}
	CloseHandle(serial_handle);
	//for (int i = 0; i < LOG_SIZE; i++)
	//	printf("%f\r\n", data_log[0][i]);
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