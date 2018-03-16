
#ifndef __CONFIG_H 
#define __CONFIG_H

//This segment should not be modified
#ifndef TRUE
#define TRUE  1
#endif

#ifndef FALSE
#define FALSE 0
#endif

#define NULL 0
typedef unsigned char  uint8;                   /* defined for unsigned 8-bits integer variable 	无符号8位整型变量  */
typedef signed   char  int8;                    /* defined for signed 8-bits integer variable		有符号8位整型变量  */
typedef unsigned short uint16;                  /* defined for unsigned 16-bits integer variable 	无符号16位整型变量 */
typedef signed   short int16;                   /* defined for signed 16-bits integer variable 		有符号16位整型变量 */
typedef unsigned int   uint32;                  /* defined for unsigned 32-bits integer variable 	无符号32位整型变量 */
typedef unsigned long long uint64;
typedef signed   int   int32;                   /* defined for signed 32-bits integer variable 		有符号32位整型变量 */
typedef float          fp32;                    /* single precision floating point variable (32bits) 单精度浮点数（32位长度） */
typedef double         fp64;                    /* double precision floating point variable (64bits) 双精度浮点数（64位长度） */

/********************************/
/*      ARM specital code      */
/*      ARM的特殊代码           */
/********************************/
//This segment should not be modify
//这一段无需改动

#include    "stm32f10x.h"


/********************************/
/*Application Program Configurations*/
/*     应用程序配置             */
/********************************/
//This segment could be modified as needed.
//以下根据需要改动

/********************************/
/*Configuration of the example */
/*     本例子的配置             */
/********************************/
/* System configuration .Fosc、Fcclk、Fcco、Fpclk must be defined */
/* 系统设置, Fosc、Fcclk、Fcco、Fpclk必须定义*/
#define Fosc            12000000                 //Crystal frequence,10MHz~25MHz，should be the same as actual status. 
						    //应当与实际一至晶振频率,10MHz~25MHz，应当与实际一至
#define Fcclk           (Fosc *6)                  //System frequence,should be (1~32)multiples of Fosc,and should be equal or less  than 60MHz. 
						    //系统频率，必须为Fosc的整数倍(1~32)，且<=60MHZ
#define Fcco            (Fcclk * 4)                 //CCO frequence,should be 2、4、8、16 multiples of Fcclk, ranged from 156MHz to 320MHz. 
						    //CCO频率，必须为Fcclk的2、4、8、16倍，范围为156MHz~320MHz
#define Fpclk           (Fcclk / 4) * 1             //VPB clock frequence , must be 1、2、4 multiples of (Fcclk / 4).
						    //VPB时钟频率，只能为(Fcclk / 4)的1、2、4倍

#include    "target.h"              //This line may not be deleted 这一句不能删除

#endif
/*********************************************************************************************************
**                            End Of File
********************************************************************************************************/

#define		VDCOFFC							0xD1
#define		VACGAINC						0xD6
#define		VACOFFC							0xD5
#define		IDCOFFC							0xC9
#define		IACGAINC						0xCE
#define		IACOFFC							0xCD

#define		CONFIG							0x00				//     配置寄存器 
#define		IDCOFF							0x02				//   I      电流通道直流偏
#define		IA								0x04				//   I        电流通道增益寄
#define		VDCOFF							0x06				//      电压通道直流偏
#define		VA								0x08				//      电压通道增益寄
#define		CYCLE							0x0A				// 每个计算周期的A
#define		PULSE							0x0C				//  Pulse-Rate   用于设置
#define		IG								0x0E				//   I       瞬时电流寄存器
#define		VG								0x10				//   V       瞬时电压寄存器
#define		PG								0x12				//   P       瞬时功率寄存器
#define		EG								0x14				//   E       电能寄存器（最
#define		IRMS							0x16				//   I      电流有效值寄存
#define		VRMS							0x18				//   V      电压有效值寄存
#define		POFF							0x1C				//   Status     能量偏移
#define		STATUS							0x1E				//   Status     状态寄存器 
#define		IACOFF							0x20				//   电流通道交流偏
#define		VACOFF							0x22				//   V      电压通道交流偏
#define		MASK     						0x34				//中断屏蔽寄存器 
#define		CTRL							0x38				//   Ctrl     控制寄存器 

#define		START							0xE8				// 启动转换 
#define		SYNC0							0xFE				// SYNC0 命令 
#define		SYNC1 							0xFF
#define		STOP							0xA0				// 上电/暂停命令 
#define		IDLE							0x88				//掉电控制 


#define     DS3231_ADDR     		0xD0
#define     FRAM_ADDR     			0xA0



#define     RST_USB		0x01<<29					//P129
#define 	BUSY_USB	0x01<<21					//P121
#define 	CS_USB		0x01<<15					//P015
#define 	INT_USB		0x01<<16					//P016
#define		DO_USB		0x01<<18					//P018
#define		DI_USB		0x01<<19					//P019
#define		SCK_USB		0x01<<17					//P017



#define 	NOPS			    __asm { NOP ; NOP; NOP; NOP; NOP; NOP}

typedef struct
{
	volatile	uint16			Vrms;										//电压										
	volatile	uint16			Irms;										//电流
	volatile	long long		Ppower;										//正电量										
	volatile	uint32			Npower;										//负电量
	volatile	uint32			Pulse;										//计数器脉冲										
	volatile	uint8			Time[8];									//时间
	volatile	uint32			PagePtr;									//在Flash中页指针
	volatile	uint32			RecordPtr;									//在页里的记录指针
	volatile	uint32			UsbPagePtr;									//上次转储位置
	volatile	uint32			ID;
	volatile	uint32			Status;
	volatile	uint8			Reserve[14];
	volatile	uint16			Crc16;
}SYSTEM;
typedef struct
{
	volatile	uint32				Idcoff;
	volatile	uint32				Vdcoff;
	volatile	uint32				Iacoff;
	volatile	uint32				Vacoff;
	volatile  	uint32				Igain;
	volatile	uint32				Vgain;
	volatile	uint32				Poff;
	volatile	uint32				ID;
	volatile	uint32				Number;
	volatile	uint8				Ref[124];
}CONFIGSFR;
typedef struct
{
	volatile	uint8				buf[100];							//命令内容
	volatile	uint8				Ok;										//命令结果
}UART;
typedef struct
{
	uint8							buf[8*20];							//显示数据
	uint8							FPtr;
	uint8							CPtr;
	uint8							EPtr;
}SHOWSFR;

#define VER							1.0


#define	xEndCH376Cmd( )	{ IO0SET = CS_USB; }  /* SPI片选无效,结束CH376命令,仅用于SPI接口方式 */


#define	LONG_NAME_BUF_LEN	( LONG_NAME_PER_DIR * 20 )	/* 自行定义的长文件名缓冲区长度,最小值为LONG_NAME_PER_DIR*1 */
