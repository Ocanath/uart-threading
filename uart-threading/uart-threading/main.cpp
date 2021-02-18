#include <stdio.h>
#include <stdlib.h>

#include <thread>
#include <iostream>

#include "usb-com-handle.h"

void main_render_thread(void)
{
	while(exit_signal==0)
	{
		printf("  %f\r\n", q_share[0]);	
	}
}

int main()
{

	std::thread t1(main_render_thread);
	std::thread t2(usb_COM_handle_thread);
	t2.join();
	t1.join();

	for (int i = 0; i < LOG_SIZE; i++)
		printf("%f\r\n", data_log[0][i]);
	//main_render_thread();
;
}
