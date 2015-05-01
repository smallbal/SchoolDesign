#include "delay.h"


//—” ±1mS
void delayms(uchar t)
{
	while(t--)
	{
     
     delay(245);
	 delay(245);
	}
}
//Œ¢—” ±
void delay(uchar t)
{
	 while(--t);
}