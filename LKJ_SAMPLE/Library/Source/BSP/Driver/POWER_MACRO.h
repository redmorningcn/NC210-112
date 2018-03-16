/******************** (C) COPYRIGHT 2014 www.nanche.com  ********************
* 文件名  ：POWER_MACRO.h
* 描述    ：实现CS5463底层宏定义
* 作者    ：ZW
*******************************************************************************/

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef _POWER_MACRO_H
#define _POWER_MACRO_H

/*命令字*/
#define		VDCOFFC							0xD1
#define		VACGAINC						0xD6
#define		VACOFFC							0xD5
#define		IDCOFFC							0xC9
#define		IACGAINC						0xCE
#define		IACOFFC							0xCD
#define     VDCGAINC                        0xD2
#define     IDCGAINC                        0xCA
#define     P_rate_ADD                      0xCB
#define     P_rate_SUB                      0xCC
#define     P_rate_SET                      0xC0
#define     P_rate_DEL                      0xAA

#define		CONFIG							0x00				//   配置寄存器 
#define		IDCOFF							0x02				//   I       电流通道直流偏
#define		IG								0x04				//   I       电流通道增益寄
#define		VDCOFF							0x06				//   电压通道直流偏
#define		VG								0x08				//   电压通道增益寄
#define		CYCLE_COUNT						0x0A				//   每个计算周期的A
#define		PULSE_RATE						0x0C				//   Pulse-Rate   用于设置
#define		INSTANT_I					    0x0E				//   I       瞬时电流寄存器
#define		INSTANT_V						0x10				//   V       瞬时电压寄存器
#define		INSTANT_P						0x12				//   P       瞬时功率寄存器
#define		PA								0x14				//   E       电能寄存器（最
#define		IRMS							0x16				//   I       电流有效值寄存
#define		VRMS							0x18				//   V       电压有效值寄存
#define		POFF							0x1C				//   Status  能量偏移
#define     FREQUENCY						0x1A                //   Freq    在线频率测量
#define		STATUS							0x1E				//   Status  状态寄存器 
#define		IACOFF							0x20				//   电流通道交流偏
#define		VACOFF							0x22				//   V       电压通道交流偏
#define     MODE                            0x24				//   MODE    操作模式寄存器
#define     TEMP                            0x26				//   TEMP    温度
#define     PQ                              0x2A           
#define     PF                              0x32                 //   PF      功率因数
#define		INTMASK     					0x34				//	 中断屏蔽寄存器 
#define     S								0x36				//   S       视在功率
#define		CTRL							0x38				//   Ctrl    控制寄存器

#define     SINGLE_CYCLE					0xE0                // 	 启动单计算周期
#define		CONTINUOUS_CYCLE				0xE8				//   启动连续计算周期 
#define		SYNC0							0xFE				//   SYNC0 命令 
#define		SYNC1 							0xFF				//   SYNC1 命令 

#define		STOP							0xA0				//   上电/暂停命令 
#define		IDLE							0x88				//   掉电控制 

#define     SOFT_RESET                      0x00
#define 	WAIT                            0x01
#define     SLEEP                           0x10 

#define     PAGE 							0x3E				//   写操作时为分页寄存器
#define 	PAGE_0                          0x000000
#define 	PAGE_1                          0x000001
#define 	PAGE_3                          0x000003
#define     PULSE_WIDTH                     0x00
#define     TGAIN                           0x04				//    温度传感器增益
#define     TOFF                            0x06				//    温度传感器偏移

#define 	Dummy_Byte           			0xFE	            //    即为SYNC0  

__packed
typedef struct
{
	float				U_RMS;					//电压	
	float				I_RMS;					//电流
	uint32_t			U_SCL;					//原边电压	
	uint32_t			I_SCL;					//原边电流
	float               Power_Freq;			    //频率
	float               Power_Factor;			//功率因数
	int32_t             ACTIVE_POWER;			//有功功率
	int32_t             REACTIVE_POWER;		    //无功功率
	uint32_t			PPpower_NUM;			//正有功电量										
	uint32_t			NPpower_NUM;			//负有功电量
	uint32_t			PPpulse;				//正有功脉冲数
	uint32_t            NPpulse;				//负有功脉冲数
	uint32_t            PPpower_base;			//正有功脉基数
	uint32_t            NPpower_base;			//负有功脉基数
	uint32_t            PPpower_temp;			//正有功脉临时变量
	uint32_t            NPpower_temp;			//负有功脉临时变量
	uint32_t			PQpower_NUM;			//正无功电量										
	uint32_t			NQpower_NUM;			//负无功电量
	uint32_t			PQpulse;				//正无功脉冲数
	uint32_t            NQpulse;				//负无功脉冲数
	uint32_t            PQpower_base;			//正无功脉基数
	uint32_t            NQpower_base;			//负无功脉基数
	uint32_t            PQpower_temp;			//正无功脉临时变量
	uint32_t            NQpower_temp;			//负无功脉临时变量

   float                U_O;//   u32 零压处理阀值
   float                U_K;//  f32 电压调节系数
   float                I_K;//  f32 电流调节系数
}ENERGY;

__packed
typedef struct
{ 
	uint32_t			PPPower;			                //正有功电量    1kvarh      99999999 kvarh
	uint32_t			NPPower;			                //负有功电量    1kvarh      99999999 kvarh 
	uint32_t			PQPower;			                //正无功电量    1kvarh      99999999 kvarh
	uint32_t			NQPower;			                //负无功电量    1kvarh      99999999 kvarh
	uint32_t			PrimVolt;					        //原边电压      0.001V      0～35000.000V
	uint32_t			PrimCurr;					        //原边电流      0.001A      0～600.000A
	uint32_t            PowerFreq;			                //频率          0.001Hz    
	int32_t             PowerFactor;			            //功率因数      0.001       -1.000～1.000
	int32_t             ActivePower;			            //有功功率      0.001kW     -12000.000  kW～12000.000  kW
	int32_t             ReactivePower;		                //无功功率      0.001kvar   -12000.000  kvar～12000.000 
} StrEnergy;

__packed
typedef struct
{    
    /***************************************************
    * 描述： 头:13 bytes
    */
    uint8_t             CmdType;                            // 命令类型
    uint16_t            LocoTyp;                            // 机车型号
    uint16_t            LocoNbr;                            // 机车号
    uint32_t            DataType            : 8;            // 数据类型
    uint32_t            Speed               :10;            // 时速
    uint32_t            Rsv1                :14;            // 预留   
    
    /***************************************************
    * 描述： 时间:4 bytes
    */
    struct __sTIME__ {
    uint32_t            Sec                 : 6;            // D5~D0:秒
    uint32_t            Min                 : 6;            // D11~D6:分
    uint32_t            Hour                : 5;            // D16~D12:时
    uint32_t            Day                 : 5;            // D21~D17：日
    uint32_t            Mon                 : 4;            // D25~D22：月
    uint32_t            Year                : 6;            // D26~D31:年
    } Time;
    
    /***************************************************
    * 描述： 电量:40 bytes
    */
    StrEnergy           Energy;
    
    /***************************************************
    * 描述： TAX:70 bytes
    */
    union __uTAX_II__ {
        struct __sTAX_II__ {
            int8_t      Rsv[57];
        }udat;
        int8_t          Buf[57];
    } Tax; 
    uint32_t            RecordTime;
    uint32_t            Head;
    uint32_t            Tail;
    uint32_t            RecordNbr;                          // 记录号

    uint16_t            chk;
} StrMater;

extern  StrMater    Mater,History;
extern  ENERGY      AC;

__packed
typedef struct
{
	uint32_t			Idcoff;
	uint32_t			Vdcoff;
	uint32_t			Iacoff;
	uint32_t			Vacoff;
	uint32_t			Igain;
	uint32_t			Vgain;
	uint32_t            P_rate;
	uint32_t			P_off;
	uint32_t			ID;
	uint32_t			Number;
	uint8_t				Ref[120];
}CONFIGSFR;
extern CONFIGSFR   configsfr_AC , configsfr_AC_RD;

__packed
typedef struct
{
	 uint8_t 	        Time[6];			    //时间
	 uint8_t			POWER_FREQ;             //频率
	 uint8_t            POWER_FACTOR;           //功率因数
	uint32_t			PPpower_NUM;			//正有功电量										
	uint32_t			NPpower_NUM;			//负有功电量
	uint32_t			PQpower_NUM;			//正有功电量										
	uint32_t			NQpower_NUM;			//负有功电量
	uint16_t			VOLTAGE;				//原边电压
	uint16_t			CURRENT;				//原边电流
	uint16_t			RES;				    //预留2字节
	uint16_t 	        Crc16;					//CRC校验
}SDAT;

extern  SDAT 	  recordsfr;


__packed
typedef struct
{
	uint8_t				buf[100];							//命令内容
	uint8_t				Ok;										//命令结果
}UART;


extern  uint32_t  g_Flash_Adr;

#endif 


