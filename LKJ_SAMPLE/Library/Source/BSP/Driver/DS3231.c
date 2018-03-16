
#include    "stm32f10x.h"
#include	"I2C_CLK.h"
#include	"DS3231.h"
#include    <stdio.h>
#include    "Display.h"
#include    "DELAY.h"
#ifdef PHOTOELECTRIC_VELOCITY_MEASUREMENT
#include <Speed_sensor.h>
#include "app_task_mater.h"
#else
#include <power_macro.h>
#endif
#include  <includes.h>


#define     DS3231_ADDR     		0xD0

#define     DS3231_SEC      		0x00
#define     DS3231_MIN      		0x01
#define     DS3231_HOUR     		0x02
#define     DS3231_DAT      		0x03	  //日期
#define     DS3231_DAY     		    0x04
#define     DS3231_MONTH    		0x05
#define     DS3231_YEAR     		0x06

#define     DS3231_CONTROL      	0x0E
#define     DS3231_STATUS       	0x0F
#define     DS3231_AGING_OFFSET     0x10
#define     DS3231_TEMP_MSB     	0x11
#define     DS3231_TEMP_LSB     	0x12

#define     RCC_GPIO_RST_CLK        RCC_APB2Periph_GPIOB
#define     RST_CLK                 GPIO_Pin_0
#define     GPIO_RST_CLK_PORT       GPIOB


/***********************************************
* 描述： OS接口
*/
#if UCOS_EN     == DEF_ENABLED
    #if OS_VERSION > 30000U
    static  OS_SEM                   Bsp_Sem;    // 信号量
    #else
    static  OS_EVENT                *Bsp_Sem;    // 信号量
    #endif
#endif
    
    
#if (UCOS_EN     == DEF_ENABLED)
/*******************************************************************************
* 名    称： EEP_WaitEvent
* 功    能： 等待信号量
* 入口参数： 无
* 出口参数：  0（操作有误），1（操作成功）
* 作　 　者： 无名沈
* 创建日期： 2015-08-18
* 修    改：
* 修改日期：
* 备    注： 仅在使用UCOS操作系统时使用
*******************************************************************************/
static uint8_t EEP_WaitEvent(void)
{
    /***********************************************
    * 描述： OS接口
    */

#if OS_VERSION > 30000U
    return BSP_OS_SemWait(&Bsp_Sem,0);           // 等待信号量
#else
    uint8_t       err;
    OSSemPend(Bsp_Sem,0,&err);                   // 等待信号量
    if ( err = OS_ERR_NONE )
      return TRUE;
    else
      return FALSE;
#endif
}

/*******************************************************************************
* 名    称： EEP_SendEvent
* 功    能： 释放信号量
* 入口参数： 无
* 出口参数： 无
* 作　 　者： 无名沈
* 创建日期： 2015-08-18
* 修    改：
* 修改日期：
* 备    注： 仅在使用UCOS操作系统时使用
*******************************************************************************/
static void EEP_SendEvent(void)
{
    /***********************************************
    * 描述： OS接口
    */
#if OS_VERSION > 30000U
    BSP_OS_SemPost(&Bsp_Sem);                        // 发送信号量
#else
    uint8_t       err;
    OSSemPost(Bsp_Sem);                              // 发送信号量
#endif
}
#endif /* end of (UCOS_EN     == DEF_ENABLED)*/

/*******************************************************************************
* Function Name  : GPIO_RST_CLK_Configuration
* Description    : 时钟复位脚
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void GPIO_RST_CLK_Configuration(void)
{
  GPIO_InitTypeDef  GPIO_InitStructure; 

  RCC_APB2PeriphClockCmd(RCC_GPIO_RST_CLK,ENABLE);   

  GPIO_InitStructure.GPIO_Pin =  RST_CLK;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
  GPIO_Init(GPIO_RST_CLK_PORT, &GPIO_InitStructure);
}


//-------------------------------------------------------------------------------------------------------
//函数名称:         uint8 BCD2HEX(uint8 Bcd)  
//功    能:         将BCD码变为二进制码
//入口参数:         BCD码数据  
//出口参数:         二进行制数
//说明：            
//--------------------------------------------------------------------------------------------------------
uint8_t BCD2HEX(uint8_t Bcd)  
{
    uint8_t   Hex;
    Hex = (Bcd & 0x0f) + ((Bcd>>4) & 0x0f)*10;
    
    return Hex;
}

//-------------------------------------------------------------------------------------------------------
//函数名称:         uint8 BCD2HEX(uint8 Bcd)  
//功    能:         将BCD码变为二进制码
//入口参数:         BCD码数据  
//出口参数:         二进行制数
//说明：            
//--------------------------------------------------------------------------------------------------------
uint8_t HEX2BCD(uint8_t Hex)  
{
    uint8_t   Bcd;
    
    Bcd = Hex %10 + (((Hex/10)%10)<<4);
    
    return Bcd;
}

//-------------------------------------------------------------------------------------------------------
//函数名称:         ReadDS3231Byte()
//功    能:         从DS3231中读出一个字节
//入口参数:         DS3231设备号， 读取数据的地址
//出口参数:         返回读出的值
//说明：            
//--------------------------------------------------------------------------------------------------------
uint8_t ReadDS3231Byte(uint8_t addr)  
{
    uint8_t   SlaveAddr;
    uint8_t   Data;	
    
    SlaveAddr = DS3231_ADDR & (~(0x01));            //从机地址
    
	Data = Single_Read(SlaveAddr , addr);
    
    return  Data;
}    

//-------------------------------------------------------------------------------------------------------
//函数名称:         void WriteDS3231Byte(uint8 addr,uint8   Data)  
//功    能:         写一个字节到DS3231中
//入口参数:         addr:DS3231地址， Data:要写入的数据
//出口参数:         返回读出的值
//--------------------------------------------------------------------------------------------------------
void WriteDS3231Byte(uint8_t Addr,uint8_t Data)  
{
    uint8_t   SlaveAddr;
    
    SlaveAddr = DS3231_ADDR & (~(0x01));            //从设备地址
   
    Single_Write(SlaveAddr , Addr , Data);
}      


//-------------------------------------------------------------------------------------------------------
//函数名称:         ReadTemp(void)  
//功    能:         从DS3231中读出温度
//入口参数:         无
//出口参数:         返回读出的温度
//说明：            
//--------------------------------------------------------------------------------------------------------
float ReadTemp(void)  
{
	float         Temp;
	uint8_t       TempMsb;
	uint8_t       TempLsb;
	
	WriteDS3231Byte(DS3231_CONTROL,0x20);
	
	TempMsb  = ReadDS3231Byte(DS3231_TEMP_MSB);      //读高位
	TempLsb  = ReadDS3231Byte(DS3231_TEMP_LSB);      //读低位
	
	//	Temp = TempMsb + TempLsb/64*0.25;
	//	Temp = Temp + 0.25;
	Temp = (float)(TempMsb) + (float)(TempLsb)/256;
	//	tem = TempMsb + TempLsb/64*0.25;
	//  printf("\r\n当前温度：tem = %d,Temp = %d,TempMsb = %d,TempLsb = %d,",tem,Temp,TempMsb,TempLsb);
	printf("\r\n时钟芯片DS3231SN当前温度：Temp = %5.2f℃,TempMsb = %d,TempLsb = %d",Temp,TempMsb,TempLsb);		
	return  Temp;
}

//-------------------------------------------------------------------------------------------------------
//函数名称:         stcTime     ReadTime(void)  
//功    能:         写DS3231时间
//入口参数:         写时间
//出口参数:         
//--------------------------------------------------------------------------------------------------------
void    WriteTime(TIME  sTime)  
{
	uint8_t   	Sec;
	uint8_t   	Min;
	uint8_t     Hour;	
	uint8_t     Day;
	uint8_t     Month;
	uint8_t     Year;
	
#if (UCOS_EN     == DEF_ENABLED)
    /***********************************************
    * 描述： OS接口
    */
    EEP_WaitEvent();                            // 等信号量
#endif
	Sec     =   HEX2BCD(sTime.Sec);		
	Min     =   HEX2BCD(sTime.Min);		
	Hour    =   HEX2BCD(sTime.Hour);	
	Day     =   HEX2BCD(sTime.Day);		
	Month   =   HEX2BCD(sTime.Month);		
	Year    =   HEX2BCD(sTime.Year);
	
	WriteDS3231Byte(DS3231_SEC,Sec);     
	WriteDS3231Byte(DS3231_MIN,Min);        
	WriteDS3231Byte(DS3231_HOUR,Hour);
	WriteDS3231Byte(DS3231_DAY,Day);    
	WriteDS3231Byte(DS3231_MONTH,Month);  
	WriteDS3231Byte(DS3231_YEAR,Year);     
#if (UCOS_EN     == DEF_ENABLED)
    EEP_SendEvent();                            // 释放信号量
#endif     
}

//-------------------------------------------------------------------------------------------------------
//函数名称:         void    SetTime(uint8_t Year,uint8_t Month,uint8_t Day,uint8_t Hour,uint8_t Min,uint8_t Sec)   
//功    能:         设置时间
//入口参数:         
//出口参数:         
//说明：            
//--------------------------------------------------------------------------------------------------------
void SetTime(uint8_t Year,uint8_t Month,uint8_t Day,uint8_t Hour,uint8_t Min,uint8_t Sec)  
{
  TIME  sTime;

  sTime.Sec   = Sec;  				
  sTime.Min   = Min;  				
  sTime.Hour  = Hour; 			
  sTime.Day  = Day; 			
  sTime.Month = Month;    			
  sTime.Year  = Year;	
  
  WriteTime(sTime);	
}

//-------------------------------------------------------------------------------------------------------
//函数名称:         void    InitDS3231(void)  
//功    能:         初始化DS3231设备模式
//入口参数:         无
//出口参数:         无
//--------------------------------------------------------------------------------------------------------
void    InitDS3231(void)  
{
    
	WriteDS3231Byte(DS3231_CONTROL,0);     
	
	WriteDS3231Byte(DS3231_STATUS,0);

    
    /***********************************************
    * 描述： OS接口
    */
#if (UCOS_EN     == DEF_ENABLED)
#if OS_VERSION > 30000U
    BSP_OS_SemCreate(&Bsp_Sem,1, "Bsp EepSem");      // 创建信号量
#else
    Bsp_Sem     = OSSemCreate(1);                    // 创建信号量
#endif
#endif
      	
}


void GetTime(TIME *t)
{
	uint8_t   	Sec;
    uint8_t   	Min;
    uint8_t     Hour;
    uint8_t     Day;
    uint8_t     Month;
    uint8_t     Year;
    	
#if (UCOS_EN     == DEF_ENABLED)
    /***********************************************
    * 描述： OS接口
    */
    EEP_WaitEvent();                            // 等信号量
#endif
    Sec     = ReadDS3231Byte(DS3231_SEC); 
    Min     = ReadDS3231Byte(DS3231_MIN);  
    Hour    = ReadDS3231Byte(DS3231_HOUR);  
    Day     = ReadDS3231Byte(DS3231_DAY); 
    Month   = ReadDS3231Byte(DS3231_MONTH); 
    Year    = ReadDS3231Byte(DS3231_YEAR);
    	
    t->Sec   = BCD2HEX(Sec); 				
    t->Min   = BCD2HEX(Min); 				
    t->Hour  = BCD2HEX(Hour);    			
    t->Day   = BCD2HEX(Day);    			
    t->Month = BCD2HEX(Month);   			
    t->Year  = BCD2HEX(Year);
#if (UCOS_EN     == DEF_ENABLED)
    EEP_SendEvent();                            // 释放信号量
#endif
}


//-------------------------------------------------------------------------------------------------------
//函数名称:         void   DisplayTime()   
//功    能:         显示时间
//入口参数:         无
//出口参数:         无
//说明：            
//--------------------------------------------------------------------------------------------------------
void DisplayTime(void)
{   
#if (UCOS_EN     == DEF_ENABLED)
    /***********************************************
    * 描述： OS接口
    */
    EEP_WaitEvent();                            // 等信号量
#endif

	GetTime((TIME *)&recordsfr.Time[0]);
	uartprintf("\r\n 当前时间为：20%d-%d-%d  %d:%u:%d", recordsfr.Time[0], recordsfr.Time[1], recordsfr.Time[2],
	                                                recordsfr.Time[3], recordsfr.Time[4], recordsfr.Time[5]);				
    
#if (UCOS_EN     == DEF_ENABLED)
    EEP_SendEvent();                            // 释放信号量
#endif
}
