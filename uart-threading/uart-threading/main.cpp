#include <stdio.h>
#include <stdlib.h>

#include <thread>
#include <iostream>

#include "usb-com-handle.h"

void main_render_thread(void)
{
	while(exit_signal==0)
	{
		for (int i = 0; i < 6; i++)
			printf("%d  ", data_buf[i]);
		printf("\r\n");
	}
}

int main()
{
	std::thread t1(main_render_thread);
	std::thread t2(usb_COM_handle_thread);
	t2.join();
	t1.join();
}
