#include "delay.h"


//��ʱ1mS
void delayms(uchar t)
{
	while(t--)
	{
     
     delay(245);
	 delay(245);
	}
}
//΢��ʱ
void delay(uchar t)
{
	 while(--t);
}