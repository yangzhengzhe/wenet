#include "stdio.h"
#include "EPD.h"

C_EPD *epd;

int init_EPD()
{
	epd=new C_EPD(8000, NULL);
	printf("EPD Initialized.\n");

	return(1);
}//int init_EPD()

int exit_EPD()
{
	delete (epd);
	printf("\nExit endpoint detection.\n");
	return(1);
}//int exit_EPD()

int detect(short *buffer, int len, int *start, int *end)
{
	//* buffer是输入语音数据指针； len, 输入语音数据长度， * start是检测到的开始位置,  * end是检测到的结束位置
	//函数返回1：检测到语音状态的变化（发现起点或终点）；返回0：没有检测到语音起点或终点

	int change_state=epd->doEPD(buffer,len,start, end);

	return(change_state);//现在change_state没有具体的意义。
}

int ReInitEPD()
{
	int re_initial=epd->re_initEPD();
	return(re_initial);

}                           
