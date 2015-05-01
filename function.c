#include "head.h"
#include "delay.h"
/*----------------------------------------------------------
					静态数据
-----------------------------------------------------------*/

uchar code DuanMa[]={0xC0, 0xF9, 0xA4, 0xB0,0x99, 
0x92, 0x82, 0xF8,0x80, 0x90, 
0xC6,0x89,0xbf,
0x46,0x09,0x3f};// 显示段码值0~9和C H - C. H. -.

//位码，从0开始，依次对应把4-16译码器1到13位置1，锁存断全部为0，需另外置1打开
uchar code WeiMa[]={0x80,0x81,0x82,0x83,0x84,
0x85,0x86,0x87,0x88,0x89,0x8a,0x8b,0x8c,0x8d,0x8e,0x8f};

uchar code GZ[]={0xff,0xfe,0xfc,0xf8,0xf0,0xe0,
0xc0,0x80,0x00};//光柱精确显示


/*----------------------------------------------------------
					变量
-----------------------------------------------------------*/
uchar step; //中断时序
uchar number_choose; //数码管位选 0-7
uchar key_time,key_temp,key_number;//按键选择1-4

uchar floor; //所处功能层级:0_通道选择 1_量程设定 2_上限调整 3_下限调整
char channel_choose;//当前所选通道选择序号:1-4
uchar scale_val[4]={0,0,0,0};//量程设定值
uchar limit[]={0,0,0,0,0,0,0,0};//上下限值 0,1,2,3：上限 4,5,6,7：下限

//缓存
uchar number_SMG[8]={0xff};//数码管缓冲
uchar number_GZ[13]={0xfd};//光柱缓冲
uchar number_AD[4]={1,2,3,4};//AD数据各位
uchar number_limit[4]={0,0,0,0};//显示上下限


uint data_ad;//AD数据
float data_ad_scale; //换算为相应通道量程的AD数据

uchar counter; //中断计数 用于按键检测和主函数中的AD刷新


/*----------------------------------------------------------
					函数定义
------------------------------------------------------------*/

	
//初始化
void init()
{
	//定时器0
	TMOD=0x01;
	TH0=(65536-1500)/256;
	TL0=(65536-1500)%256;
	TR0=1;
	ET0=1;
	//继电器
	AL_L=1;
	AL_H=1;
	//数码管
	number_SMG[0]=DuanMa[10]; //C
	number_SMG[1]=DuanMa[11]; //H
	number_SMG[2]=DuanMa[12]; //-
	//光柱
	CLK1=0;
	//watch dog
	init_x25045();
	
	//读取ROM 通道号 量程*4 上下限*8
	channel_choose=Func_read_da(Address_channel);
	scale_val[0]=Func_read_da(Address_scale);
	scale_val[1]=Func_read_da(Address_scale+1);
	scale_val[2]=Func_read_da(Address_scale+2);
	scale_val[3]=Func_read_da(Address_scale+3);
	limit[0]=Func_read_da(Address_limit);
	limit[1]=Func_read_da(Address_limit+1);
	limit[2]=Func_read_da(Address_limit+2);
	limit[3]=Func_read_da(Address_limit+3);
	limit[4]=Func_read_da(Address_limit+4);
	limit[5]=Func_read_da(Address_limit+5);
	limit[6]=Func_read_da(Address_limit+6);
	limit[7]=Func_read_da(Address_limit+7);
	EA=1;//开启计时器
}

void judgement()
{
	if((floor==0)&&(counter==110))
	{
		Func_get_AD();
		Func_alarm(channel_choose);
	}
}
	

bit key_flag=0;
bit key_flag1=1;

void T0_time() interrupt 1
{
	//CLK4=1;
	TH0=(65536-1500)/256;
	TL0=(65536-1500)%256;
	Block_Choose=14;
	Func_touch_watch_dog();// feed the watch dog
	Func_get_GZ(data_ad);
	if(step<=3)
	{
		Data=number_SMG[step];
		CLK2=1;
		CLK2=0;	
		Data=number_GZ[step];
		CLK1=1;
		CLK1=0;
		Block_Choose=WeiMa[step];
		if((KEY==1)&&(key_flag1==1))
		{
			if(key_flag==1)
			{
				//CLK4=1;
				key_number=step+1;
				key_flag=0;
				key_flag1=0;
			}
			else
			{
				key_flag=1;
			}
		}
	/*	else
		{
			key_flag=0;
		}*/
		if(counter%100==0)
		{
			key_flag1=1;
		}
		counter++;
		if(counter>150)
		{
			counter=0;
		}
		/*if((KEY==1)&&(key_time==0))
		{
			key_time++;
			key_temp=step;
		}
		if((key_time<5)&&(key_time>=1))
		{
			key_time++;
		}
		if((KEY==1)&&(key_time==5)&&(key_temp==step))
		{
			CLK4=0;
			key_number=key_temp+1;
			key_time=0;
		}
	}
	if((step>=4)&&(step<=7))
	{
		if(floor==0)
		{
			Func_display_number(number_AD);
		}
		if((floor==1)||(floor==2))
		{
			Func_display_number(number_limit);
		}*/
		
		Func_keyscan(key_number);
	}	
	if((step>=4)&&(step<=7))
	{
		Data=number_SMG[step];
		CLK2=1;
		CLK2=0;
		Data=number_GZ[step];
		CLK1=1;
		CLK1=0;
		Block_Choose=WeiMa[step];
	}
	if((step>=8)&&(step<=12))
	{
		Data=number_GZ[step];
		CLK1=1;
		CLK1=0;
		Block_Choose=WeiMa[step];
	}
	step++;
	if(step>=13)
	{
		step=0;
	}
}


//通道选择层
void Func_channel_choose(uchar channel)
{
	if(channel==1)
	{
		if(channel_choose<4)
			channel_choose++;
		else
			channel_choose=1;
	}
	if(channel==2)
	{
		if(channel_choose>1)
			channel_choose--;
		else
			channel_choose=4;
	}
	if(channel==3)
	{
		floor=1;
		number_SMG[0]=DuanMa[13];//将C变为C.
		Func_write_da(Address_channel,channel_choose);
		
	}
	number_SMG[3]=DuanMa[channel_choose];
	number_SMG[4]=number_AD[0];
	number_SMG[5]=number_AD[1];
	number_SMG[6]=number_AD[2];
	number_SMG[7]=number_AD[3];
	key_number=0;
	//Func_alarm(channel_choose);
}

//量程设定层
void Func_scale_choose(uchar val)
{
	if(val==1)
	{
		if(scale_val[channel_choose-1]<10)
		{
			scale_val[channel_choose-1]+=1;
		}
	}
	if(val==2)
	{
		if(scale_val[channel_choose-1]>0)
		{
			scale_val[channel_choose-1]-=1;
		}
	}
	if(val==3)
	{
		floor=2;
		number_SMG[1]=DuanMa[14];//将H变为H.
		Func_write_da(Address_scale+channel_choose-1,scale_val[channel_choose-1]);
	}
	if(val==4)
	{
		floor=0;
		number_SMG[0]=DuanMa[10];//将C.变为C
	}
	key_number=0;
	number_SMG[4]=DuanMa[scale_val[(channel_choose-1)]/1000];
	number_SMG[5]=DuanMa[scale_val[(channel_choose-1)]%1000/100];
	number_SMG[6]=DuanMa[scale_val[(channel_choose-1)]%100/10];
	number_SMG[7]=DuanMa[scale_val[(channel_choose-1)]%10];
}
//上限设定层
void Func_limit_high (uchar high_val)
{
	if(high_val==1)
	{
		if(limit[(channel_choose-1)]<scale_val[channel_choose-1])
			limit[(channel_choose-1)]+=1;
	}
	if(high_val==2)
	{
		if(limit[(channel_choose-1)]>=1)
			limit[(channel_choose-1)]-=1;
	}
	if(high_val==3)
	{
		floor=3;
		number_SMG[2]=DuanMa[15];//将 - 变为 -.
		Func_write_da(Address_limit+channel_choose-1,limit[channel_choose-1]);
		
	}
	if(high_val==4)
	{
		floor=1;
		number_SMG[1]=DuanMa[11];//将H. 变为 H
	}
	key_number=0;
	number_SMG[4]=DuanMa[limit[(channel_choose-1)]/1000];
	number_SMG[5]=DuanMa[(limit[(channel_choose-1)]%1000)/100];
	number_SMG[6]=DuanMa[(limit[(channel_choose-1)]%100)/10];
	number_SMG[7]=DuanMa[limit[(channel_choose-1)]%10];

	}

//下限设定层
void Func_limit_low(uchar low_val)
{
	if(low_val==1)
	{
		if(limit[channel_choose+3]<limit[(channel_choose-1)])
			limit[channel_choose+3]+=1;
	}
	if(low_val==2)
	{
		if(limit[channel_choose+3]>=1)
			limit[channel_choose+3]-=1;
	}
	if(low_val==3)
	{
		floor=0;
		number_SMG[0]=DuanMa[10];
		number_SMG[1]=DuanMa[11];
		number_SMG[2]=DuanMa[12];//将C.H.-.变为 CH-
		Func_write_da(Address_limit+channel_choose+3,limit[channel_choose+3]);
	}
	if(low_val==4)
	{
		floor=2;
		number_SMG[2]=DuanMa[12];
	}
	key_number=0;
	number_SMG[4]=DuanMa[limit[(channel_choose+3)]/1000];
	number_SMG[5]=DuanMa[(limit[(channel_choose+3)]%1000)/100];
	number_SMG[6]=DuanMa[(limit[(channel_choose+3)]%100)/10];
	number_SMG[7]=DuanMa[limit[(channel_choose+3)]%10];
	
}

//中断键盘扫描
void Func_keyscan(uchar t)
{
	if(floor==0)
	{
		Func_channel_choose(t);
		return ;
	}
	if(floor==1)
	{
		Func_scale_choose(t);
		return ;
	}
	if(floor==2)
	{
		Func_limit_high(t);
		return ;
	}
	if(floor==3)
	{
		Func_limit_low(t);
		return ;
	}
}

//实时AD检测显示层
void Func_get_AD()
{
	data_ad=get_AD(channel_choose);//选择读取0通道AD数据
	Func_transmit(scale_val[channel_choose-1]);
}


//通道名称数码管
void Func_display_channel(uchar val)
{	
	if(number_choose<=2)
	{
		EA=0;
		Data=DuanMa[(number_choose+10)];
		Block_Choose=WeiMa[number_choose];
		CLK2=1;
		CLK2=0;
		delay(100);
		Data=0xff;
		CLK2=1;
		CLK2=0;
		delay(1);
		EA=1;
	}
 if(number_choose==3)
	{
		Data=DuanMa[val];
		Block_Choose=WeiMa[3];
		CLK2=1;
		CLK2=0;
		delay(100);
		Data=0xff;
		CLK2=1;
		CLK2=0;
		delay(1);
	}
}

//数据显示

void Func_display_number(uchar *p)
{
	uchar temp;
		temp=*(p+step-4);
		Data=DuanMa[temp];
		Block_Choose=WeiMa[step];
		CLK2=1;
		CLK2=0;
	
}
//光柱缓存计算
void Func_get_GZ(uint x)
{
	uchar polar;
	uchar GZ_all;
	uchar GZ_tail;
	uchar i;
	if(floor==0)
	{
		polar=((float)x/4096)*101;
		GZ_all=polar/8;
		GZ_tail=polar%8;
	
		for(i=0;i<GZ_all;i++)
		{
			number_GZ[i]=GZ[8];
		}
		number_GZ[GZ_all]=GZ[GZ_tail];
		for(i=12;i>GZ_all;i--)
		{
			number_GZ[i]=GZ[0];
		}
	}
	else
	{
		for(i=0;i<13;i++)
		{
			number_GZ[i]=GZ[0];
		}
	}
	
}

 /*---------------------------------------------------------------
				4052相关函数
 ----------------------------------------------------------------*/

void init_4052(uchar da_4052)
{
	 A1_4052 =da_4052/2;
	 A0_4052 =da_4052%2;
}
 //_____________________________________________________________________
 void init_mcp3202(uchar t)
 {
  	 nCS_MCP3202=1;
	 init_4052(t);
 }
 //_____________________________________________________________________
 uint  get_AD(uchar tdh)
 {
    uint val=0;
    init_spi();	  
    init_mcp3202(tdh-1);//
	nCS_MCP3202=1;
    nCS_MCP3202=0;
	Func_write_bit(1);
	Func_write_bit(1);
   	Func_write_bit(0);
	Func_write_bit(1);
	val=Func_read_12();
	nCS_MCP3202=1;
	return val	 ;   
 }
 
 /*-----------------------------------------------------
					SPI
--------------------------------------------------------*/
//初始化SPI
void init_spi()
{
	SCLK=1;
}
//写1位
void Func_write_bit(bit spi_bit)
{
	SCLK=0;
	MOSI=spi_bit;
	SCLK=1;
}
//读1位
bit Func_read_bit()
{
	bit value;
	SCLK=0;
	value=MISO;
	SCLK=1;
	return value;
}  
//写8位
void Func_write_8(uchar spi_8)
{
	uchar i=0;
	for(i=0;i<8;i++)
	{ 
		if(spi_8&0x80)
		{
			Func_write_bit(1);
		}
		else
		{
			Func_write_bit(0);
		} 	
		spi_8<<=1;
	}
}

//读8位
 uchar Func_read_8()
{
	uchar value=0,i=0;
	bit read_8_ai;
	for(i=0;i<8;i++)
	{
		value=(value<<1) ; 
		read_8_ai=Func_read_bit();  
		if(read_8_ai)
		{
			value=value|0x01;
		}
	}
	return value;
}

//读12位

uint Func_read_12()
{
	uint value=0;
	uchar i=0;
	bit read_12_ai;
	SCLK=0;
	SCLK=1;
	for(i=0;i<12;i++)
	{
        value=(value<<1);
		read_12_ai=Func_read_bit(); 
		if(read_12_ai)
		{
			value=value|0x0001;
		} 
	}		 
	return value;
}


/*---------------------------------------------------
					功能函数
---------------------------------------------------*/
//上下线报警
void Func_alarm(uchar ch)
{
	uchar x;
	x=data_ad_scale*1000/1;
		if(limit[ch-1]<data_ad_scale)
		{
			AL_H=0;
			AL_L=1;
		}
		else if(limit[ch+3]>data_ad_scale)
		{
			AL_H=1;
			AL_L=0;
		}
		else
		{
			AL_H=1;
			AL_L=1;
		}
}
//量程转换
void Func_transmit(uchar scale_set)
{
	uint value;
	data_ad_scale=((float)data_ad/4096)*scale_set;
	value=data_ad_scale*1000;
	number_AD[0]=DuanMa[value/1000]-0x80;
	number_AD[1]=DuanMa[(value%1000)/100];
	number_AD[2]=DuanMa[(value%100)/10];
	number_AD[3]=DuanMa[value%10];
}
/*----------------------------------------------------
				25045
----------------------------------------------------*/	
//打开 watch dog
void init_x25045()
	{  
	   nCS_25045=0;
	   Func_write_8(WRSR);//写寄存器
	   Func_write_8(0x00);//1.4 Seconds
	   nCS_25045=1;
	}
//喂狗
void Func_touch_watch_dog()  
{
	nCS_25045=0;
	nCS_25045=1;
}
//验忙
bit Func_is_busy()			
{	
    uchar sta=0;
   	sta=Func_read_status();
	if(sta&0x01)
	{
		return 1;
	}
	else 
	return  0;
}
//读一个字节
uchar Func_read_da(uchar address)	
{
    uchar va;
    nCS_25045=0;
		Func_write_8(READ); 
    Func_write_8(address); 
		va=Func_read_8();
		nCS_25045=1;
		return va;
}
//写一个字节
void Func_write_da(uchar addr,uchar dat)	
{
	nCS_25045=0;
	Func_write_8(WREN);
	nCS_25045=1;
	nCS_25045=0;
	Func_write_8(WRITE); 
	Func_write_8(addr);
	Func_write_8(dat);  
	nCS_25045=1;
	while(Func_is_busy());
	    
}
//读2个字节
uint Func_read_word(uchar address)		 
{
	uchar v1,v2;
	nCS_25045=0;
	Func_write_8(READ); 
    Func_write_8(address); 
	v1=Func_read_8();
	v2=Func_read_8();
	nCS_25045=1;
	return v1*256+v2;
}
//写2个字节
void Func_write_word(uchar address,uint dat)		 
{
   uchar v1,v2;
   v1= dat/256;
   v2=dat%256;
   nCS_25045=0;
   Func_write_8(WREN);
   nCS_25045=1;
   nCS_25045=0;
   Func_write_8(WRITE); 
   Func_write_8(address);
   Func_write_8(v1);  
   Func_write_8(v2);  
   nCS_25045=1;
   while(Func_is_busy());
}
//读状态寄存器
uchar Func_read_status()				  
{
	uchar va;
	nCS_25045=0;  
	Func_write_8(RDSR);
	va=Func_read_8();
	nCS_25045=1;
	return va;
}