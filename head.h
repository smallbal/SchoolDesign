#ifndef HEAD_H_
#define HEAD_H_

#include <reg52.h>

#define uchar unsigned char
#define uint unsigned int
//25045 指令宏
#define WREN 0x06	//Set the Write Enable Latch (Enable Write Operations)
#define WRDI 0x04	//Reset the Write Enable Latch (Disable Write Operations)
#define RDSR 0x05	//Read Status Register
#define WRSR 0x01	//Write Status Register (Block Lock Bits)
#define READ 0x03	//0000 A8011Read Data from Memory Array beginning at selectedaddress
#define WRITE 0x02	//0000 A8010Write Data to Memory Array beginning at Selected Address(1 to 4 Bytes)

#define Data P0	//定义数据输出端口
#define Block_Choose P2	//选择显示模块

#define Address_channel 0x00
#define Address_scale 0x01
#define Address_limit 0x05

/***************引脚定义*******************/
sbit MOSI=P1^0;		//3202模式控制端
sbit MISO=P1^1;		//3202数据输出端
sbit SCLK=P1^2;		//3202时钟控制端

sbit nCS_25045=P1^3;	//5045片选端（低电平有效）
sbit nCS_MCP3202=P1^4;	//3202片选端（低电平有效）

sbit KEY=P1^5;		//按键检测端 P1^5

sbit A0_4052=P1^6;		//4052通道选择端
sbit A1_4052=P1^7;		//4052通道选择端

sbit CLK1=P2^4;		//光柱阴极锁存端
sbit CLK2=P2^5;		//数码管段锁存
sbit CLK3=P2^6;		//(空)
sbit CLK4=P2^7;		//单片机动作信号指示灯

sbit AL_L=P3^6;		//报警下限控制端 0启动
sbit AL_H=P3^7;		//报警上限控制端 1关闭

//测试引脚
sbit P2_D=P2^3;
sbit P2_C=P2^2;
sbit P2_B=P2^1;
sbit P2_A=P2^1;


//函数原型

void init(void); //初始化
void judgement(); //判断键位，选择相应功能

void Func_channel_choose(uchar); //通道选择层 key_number
void Func_scale_choose(uchar);//量程设定层 key_number
void Func_limit_high(uchar);//上限设定层 key_number
void Func_limit_low(uchar);//下限设定层 key_number
void Func_get_AD();//实时AD检测显示层
void Func_keyscan(uchar);//中断中键盘扫描 key_number

/*----------显示函数------------------*/
void Func_display_channel(uchar);//通道名称数码管显示，输入通道号1——4
void Func_display_number(uchar *);//数据显示
void Func_get_GZ(uint); //将AD数据转换为光柱缓存数据
/*-----------AD-----------------------*/
void init_4052(uchar);
void init_mcp3202(uchar);
uint get_AD(uchar ); //输入AD通道(此次为0),返回AD值

/*----------功能化函数----------------*/
void Func_alarm(uchar);//上下限报警	 channel_choose
void Func_transmit(uchar);//量程转换 scale_val[x]

/*-----------SPI----------------------*/
void  init_spi();//将SCLK置1 初始化
void  Func_write_bit(bit);//写1位 上升沿写入
bit   Func_read_bit();//读1位 上升沿锁存
void  Func_write_8(uchar );//写8位
uchar Func_read_8();//读8位
uint  Func_read_12();//读12位
/*--------25045----------------------*/
void  init_x25045();//打开watch dog 用1.4S模式
void  Func_touch_watch_dog();//摸狗头，在设置时间内至少摸一次，否则它会咬你
bit   Func_is_busy();//验忙 忙为1 不忙为0
uchar Func_read_da(uchar);//读一个字节 地址
void  Func_write_da(uchar,uchar);//写一个字节 (地址,数据)
uint  Func_read_word(uchar);//读2个字节 地址
void  Func_write_word(uchar,uint);//写2个字节 (地址,数据)
uchar Func_read_status();//读状态寄存器


#endif


