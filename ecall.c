#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/times.h>
#include "riscv.h"

void ecall()
{
	int sys_num = i_reg[17];
	int arg_0 = i_reg[10];
	int arg_1 = i_reg[11];
	int arg_2 = i_reg[12];
	int arg_3 = i_reg[13];

	int fields;
	void *io_buf;
	int nbytes;
	struct tms *tms_buf;

	if (sys_num == 63)			//read
	{
		fields = arg_0;
		io_buf = (void *)arg_1;
		nbytes = arg_2;
		i_reg[10] = read(fields, io_buf, nbytes);
	}
	else if (sys_num == 64)	//write
	{
		fields = arg_0;
		io_buf = (void *)arg_1;
		nbytes = arg_2;
		i_reg[10] = write(fields, io_buf, nbytes);
	}
	else if (sys_num == 153)	//times
	{
		tms_buf = (struct tms *)arg_0;
		i_reg[10] = times(tms_buf);
	}
	else if (sys_num == 93)		//exit
		END = 1;
	else
	{
		printf("Undefined ecall %d\r\n", sys_num);
		exit(0);
	}
}
