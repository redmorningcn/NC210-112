/********************   (C) COPYRIGHT 2013 www.armjishu.com   ********************
* 文件名  ：SZ_STM32F107VC_LIB.c
* 描述    ：提供STM32F107VC神舟IV号开发板的库函数
* 实验平台：STM32神舟开发板
* 作者    ：www.armjishu.com 
**********************************************************************************/

/* Includes ------------------------------------------------------------------*/
#include "SZ_STM32F107VC_LIB.h"
#include "RS485.h"
#ifdef PHOTOELECTRIC_VELOCITY_MEASUREMENT
#include <Speed_sensor.h>
#include "app_task_mater.h"
#else
#include <power_macro.h>
#endif
#include "SPI_CS5463_AC.h"
#include "FM24CL64.h"
#include "MX25.h"
#include "DS3231.h"
#include "crccheck.h"
#include "delay.h"
#include <includes.h>

__IO uint32_t TimingDelay;

USART_TypeDef* COM_USART[COMn] = {SZ_STM32_COM1, SZ_STM32_COM2}; 
GPIO_TypeDef* COM_TX_PORT[COMn] = {SZ_STM32_COM1_TX_GPIO_PORT, SZ_STM32_COM2_TX_GPIO_PORT};
GPIO_TypeDef* COM_RX_PORT[COMn] = {SZ_STM32_COM1_RX_GPIO_PORT, SZ_STM32_COM2_RX_GPIO_PORT};
const uint32_t COM_USART_CLK[COMn] = {SZ_STM32_COM1_CLK, SZ_STM32_COM2_CLK};
const uint32_t COM_TX_PORT_CLK[COMn] = {SZ_STM32_COM1_TX_GPIO_CLK, SZ_STM32_COM2_TX_GPIO_CLK};
const uint32_t COM_RX_PORT_CLK[COMn] = {SZ_STM32_COM1_RX_GPIO_CLK, SZ_STM32_COM2_RX_GPIO_CLK};
const uint16_t COM_TX_PIN[COMn] = {SZ_STM32_COM1_TX_PIN, SZ_STM32_COM2_TX_PIN};
const uint16_t COM_RX_PIN[COMn] = {SZ_STM32_COM1_RX_PIN, SZ_STM32_COM2_RX_PIN};

uint32_t STM32DeviceSerialID[3]; /* 全局变量IntDeviceSerial存放读到的设备ID */

/**-------------------------------------------------------
* @函数名 delay
* @功能   简单的delay延时函数.
* @参数   延迟周期数 0--0xFFFFFFFF
* @返回值 无
***------------------------------------------------------*/
void delay(__IO uint32_t nCount)
{
    for (; nCount != 0; nCount--);
}

/**-------------------------------------------------------
* @函数名 NVIC_GroupConfig
* @功能   配置NVIC中断优先级分组函数.
*         默认配置为1比特表示主优先级, 3比特表示次优先级
*         用户可以根据需要修改
* @参数   无
* @返回值 无
***------------------------------------------------------*/
void NVIC_GroupConfig(void)
{
    /* 配置NVIC中断优先级分组:
    - 1比特表示主优先级  主优先级合法取值为 0 或 1 
    - 3比特表示次优先级  次优先级合法取值为 0..7
    - 数值越低优先级越高，取值超过合法范围时取低bit位 
    */
    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_1);
    
    /*==================================================================================
    NVIC_PriorityGroup   |  主优先级范围  |  次优先级范围  |   描述
    ==================================================================================
    NVIC_PriorityGroup_0  |      0         |      0-15      |   0 比特表示主优先级
    |                |                |   4 比特表示次优先级 
    ----------------------------------------------------------------------------------
    NVIC_PriorityGroup_1  |      0-1       |      0-7       |   1 比特表示主优先级
    |                |                |   3 比特表示次优先级 
    ----------------------------------------------------------------------------------
    NVIC_PriorityGroup_2  |      0-3       |      0-3       |   2 比特表示主优先级
    |                |                |   2 比特表示次优先级 
    ----------------------------------------------------------------------------------
    NVIC_PriorityGroup_3  |      0-7       |      0-1       |   3 比特表示主优先级
    |                |                |   1 比特表示次优先级 
    ----------------------------------------------------------------------------------
    NVIC_PriorityGroup_4  |      0-15      |      0         |   4 比特表示主优先级
    |                |                |   0 比特表示次优先级   
    ==================================================================================*/
}

/**-------------------------------------------------------
* @函数名 SZ_STM32_SysTickInit
* @功能   初始化系统定时器SysTick
*         用户可以根据需要修改
* @参数   每秒中断次数
* @返回值 无
***------------------------------------------------------*/
void SZ_STM32_SysTickInit(uint32_t HzPreSecond)
{
    /* HzPreSecond = 1000 to Setup SysTick Timer for 1 msec interrupts.
    ------------------------------------------
    1. The SysTick_Config() function is a CMSIS function which configure:
    - The SysTick Reload register with value passed as function parameter.
    - Configure the SysTick IRQ priority to the lowest value (0x0F).
    - Reset the SysTick Counter register.
    - Configure the SysTick Counter clock source to be Core Clock Source (HCLK).
    - Enable the SysTick Interrupt.
    - Start the SysTick Counter.
    
    2. You can change the SysTick Clock source to be HCLK_Div8 by calling the
    SysTick_CLKSourceConfig(SysTick_CLKSource_HCLK_Div8) just after the
    SysTick_Config() function call. The SysTick_CLKSourceConfig() is defined
    inside the misc.c file.
    
    3. You can change the SysTick IRQ priority by calling the
    NVIC_SetPriority(SysTick_IRQn,...) just after the SysTick_Config() function 
    call. The NVIC_SetPriority() is defined inside the core_cm3.h file.
    
    4. To adjust the SysTick time base, use the following formula:
    
    Reload Value = SysTick Counter Clock (Hz) x  Desired Time base (s)
    - Reload Value is the parameter to be passed for SysTick_Config() function
    - Reload Value should not exceed 0xFFFFFF
    */
    if (SysTick_Config(SystemCoreClock / HzPreSecond))
    { 
        /* Capture error */ 
        while (1);
    }
}

/**-------------------------------------------------------
* @函数名 SysTickDelay
* @功能   系统定时器实现的延时函数，可以在用户程序中调用
*         用户可以根据需要修改
* @参数   延迟周期数，延迟时间为nTime/HzPreSecond
*         误差为 -1/HzPreSecond
* @返回值 无
***------------------------------------------------------*/
void SysTickDelay(__IO uint32_t nTime)
{ 
    TimingDelay = nTime;
    
    while(TimingDelay != 0);
}

/**-------------------------------------------------------
* @函数名 TimingDelay_Decrement
* @功能   系统节拍定时器服务函数调用的子函数
*         将全局变量TimingDelay减一，用于实现延时
* @参数   无
* @返回值 无
***------------------------------------------------------*/
void TimingDelay_Decrement(void)
{
    if (TimingDelay != 0x00)
    { 
        TimingDelay--;
    }
}

extern void SysTick_Handler_User(void);
/**-------------------------------------------------------
* @函数名 SysTick_Handler
* @功能   系统节拍定时器服务请求处理函数
* @参数   无
* @返回值 无
***------------------------------------------------------*/
void SysTick_Handler(void)
{
    TimingDelay_Decrement();
    
    SysTick_Handler_User();
}

void GetDeviceSerialID(void)
{
    STM32DeviceSerialID[0] = *(__IO uint32_t*)(0x1FFFF7E8);
    STM32DeviceSerialID[1] = *(__IO uint32_t*)(0x1FFFF7EC);
    STM32DeviceSerialID[2] = *(__IO uint32_t*)(0x1FFFF7F0);
}

/**-------------------------------------------------------
* @函数名 __SZ_STM32_COMInit
* @功能   对STM32的USART初始化底层函数
* @参数1  COM1  对应STM32的USART1 对应开发板上串口1
*         COM2  对应STM32的USART2 对应开发板上串口2
* @参数2  指向一个成员已赋值USART_InitTypeDef结构体的指针
* @返回值 无
***------------------------------------------------------*/
void __SZ_STM32_COMInit(COM_TypeDef COM, USART_InitTypeDef* USART_InitStruct)
{
    GPIO_InitTypeDef GPIO_InitStructure;
    
    /* Enable GPIO clock */
    /* 使能STM32的USART对应GPIO的Clock时钟 */
    RCC_APB2PeriphClockCmd(COM_TX_PORT_CLK[COM] | COM_RX_PORT_CLK[COM] | RCC_APB2Periph_AFIO, ENABLE);
    
    if (COM == COM1)
    {
        /* 使能STM32的USART1/USART2/USART3的Clock时钟 */
        RCC_APB2PeriphClockCmd(COM_USART_CLK[COM], ENABLE); 
    }
    else
    {
        /* Enable the USART2 Pins Software Remapping */
        /* 使能STM32的USART2的管脚重映射 */
        //        GPIO_PinRemapConfig(GPIO_Remap_USART2, ENABLE);    //电表电路中无需映射
        /* 使能STM32的USART的Clock时钟 */
        RCC_APB1PeriphClockCmd(COM_USART_CLK[COM], ENABLE);
    }
    
    /* Configure USART Tx as alternate function push-pull */
    /* 初始化STM32的USART的TX管脚，配置为复用功能推挽输出 */
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
    GPIO_InitStructure.GPIO_Pin = COM_TX_PIN[COM];
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(COM_TX_PORT[COM], &GPIO_InitStructure);
    
    /* Configure USART Rx as input floating */
    /* 初始化STM32的USART的RX管脚，配置为复用功能输入 */
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
    GPIO_InitStructure.GPIO_Pin = COM_RX_PIN[COM];
    GPIO_Init(COM_RX_PORT[COM], &GPIO_InitStructure);
    
    /* USART configuration */
    /* 根据传入的参数初始化STM32的USART配置 */
    USART_Init(COM_USART[COM], USART_InitStruct);
    
    /* Enable USART */
    /* 使能STM32的USART功能模块 */
    USART_Cmd(COM_USART[COM], ENABLE);
}

/**-------------------------------------------------------
* @函数名 __SZ_STM32_COMInit
* @功能   面向用户的STM32的USART初始化函数
* @参数1  COM1  对应STM32的USART1 对应开发板上串口1
*         COM2  对应STM32的USART2 对应开发板上串口2
* @参数2  BaudRate 串口的波特率，例如"115200"
* @返回值 无
***------------------------------------------------------*/
void SZ_STM32_COMInit(COM_TypeDef COM, uint32_t BaudRate)
{
    
    USART_InitTypeDef USART_InitStructure;
    
    /* USARTx 默认配置:
    - BaudRate = 115200 baud  
    - Word Length = 8 Bits
    - One Stop Bit
    - No parity
    - Hardware flow control disabled (RTS and CTS signals)
    - Receive and transmit enabled
    */
    USART_InitStructure.USART_BaudRate = BaudRate;              //串口的波特率，例如115200 最高达4.5Mbits/s
    USART_InitStructure.USART_WordLength = USART_WordLength_8b; //数据字长度(8位或9位)
    USART_InitStructure.USART_StopBits = USART_StopBits_1;      //可配置的停止位-支持1或2个停止位
    USART_InitStructure.USART_Parity = USART_Parity_No;         //无奇偶校验  
    USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None; //无硬件流控制
    USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx; //双工模式，使能发送和接收
    
    __SZ_STM32_COMInit(COM, &USART_InitStructure);  // 调用STM32的USART初始化底层函数
    
    SystemCoreClockUpdate();
    //    printf("\n\r 系统内核时钟频率(SystemCoreClock)为：%dHz.\n\r",
    //            SystemCoreClock);
}

/**-------------------------------------------------------
* @函数名 NVIC_COMConfiguration
* @功能   配置串口1和串口2的中断参数
* @参数   无
* @返回值 无
***------------------------------------------------------*/
void NVIC_COMConfiguration(void)
{
    NVIC_InitTypeDef NVIC_InitStructure;
    
    USART_ITConfig(SZ_STM32_COM1, USART_IT_RXNE, ENABLE);
    
    NVIC_InitStructure.NVIC_IRQChannel = SZ_STM32_COM1_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    
    NVIC_Init(&NVIC_InitStructure);
    
    USART_ITConfig(SZ_STM32_COM2, USART_IT_RXNE, ENABLE);
    
    NVIC_InitStructure.NVIC_IRQChannel = SZ_STM32_COM2_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 2;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 2;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    
    NVIC_Init(&NVIC_InitStructure);
}

UART1 rx,tx;
//
//void USART_GetInputString(void)
//{
//	uint8_t mychar = 0;
//
//	mychar = USART_ReceiveData(USART1);
//	if(rx.ok == 0)
//	{
//		if(mychar == '\r')
//		{
//			rx.buf[rx.ptr] = '\0';
//			rx.ok = 1;
//		}
//		else if(mychar == '\b')
//		{
//			if(rx.ptr > 0)
//			{	
//				rx.ptr--;
//			}
//		}
//	    else 
//		{
//			rx.buf[rx.ptr++] = mychar;
//			printf("%d,  %x ",rx.ptr,mychar);
//		}
//		if(rx.ptr >= 3)
//		{
//		 	rx.buf[rx.ptr] = '\0';
//			rx.ok = 1;
//		}
//	}
//}
//
//
////Usart_1IrqHandlerUser()
////{
////	if(USART_GetITStatus(USART1, USART_IT_RXNE) != RESET)
////    {
////         /* clear interrupt */
////		USART_GetInputString();	
////		while(USART_GetITStatus(USART1, USART_IT_RXNE) == RESET);
////	}
////	if(USART_GetITStatus(USART1, USART_IT_RXNE) != RESET)
////	{
////		USART_ClearITPendingBit(USART1, USART_IT_RXNE);
////    }
////
////    /* If overrun condition occurs, clear the ORE flag 
////                              and recover communication */
////     /*数据溢出*/
////    if (USART_GetFlagStatus(USART1, USART_FLAG_ORE) != RESET)
////    {
//////		rs = USART_GetITStatus(USART1, USART_IT_RXNE);
//////		printf("RXNE状态为%d！",rs);
//////        printf("数据溢出");
//////		USART_ClearFlag(USART1,USART_FLAG_ORE);
//////		rx.buf[rx.ptr++] = 	USART1->DR & 0x01FF;
//////		printf("%d  , %x  ,  ",rx.ptr,rx.buf[rx.ptr-1]);
////		(void)USART_ReceiveData(USART1);
////    }
////  	 /*发送完成*/
////	if (USART_GetITStatus(USART1, USART_IT_TC) != RESET)
////	{
////		/* clear interrupt */
////		USART_ClearITPendingBit(USART1, USART_IT_TC);
////	}
//////	while (USART1->SR & USART_FLAG_RXNE == 0);
////}
//
//void Usart_1IrqHandlerUser()
//{
//	uint8_t l_COM0TempRev = 0;
// 	if(USART_GetITStatus(USART1, USART_IT_RXNE) != RESET)
//    {
//         /* clear interrupt */
//        USART_ClearITPendingBit(USART1, USART_IT_RXNE);
//		/* save on rx buffer */		
//		/* save character */
//        l_COM0TempRev = USART1->DR & 0x01ff;
//				printf("%x-", l_COM0TempRev);
//        rx.buf[rx.ptr] = l_COM0TempRev; 
//		rx.ptr++;
//		if(rx.ptr == 6)
//			rx.ptr = 0;
//		while (USART1->SR & USART_FLAG_RXNE == 0);
//    }
//
//    /* If overrun condition occurs, clear the ORE flag 
//                              and recover communication */
//     /*数据溢出*/
//    if (USART_GetFlagStatus(USART1, USART_FLAG_ORE) != RESET)
//    {
//        (void)USART_ReceiveData(USART1);
//    }
//  	 /*发送完成*/
//	if (USART_GetITStatus(USART1, USART_IT_TC) != RESET)
//	{
//		/* clear interrupt */
//		USART_ClearITPendingBit(USART1, USART_IT_TC);
//	}
//}
//
//void Uart_1Control(void)
//{
//	uint8_t     fram_clr[4]     = {0};
//	uint8_t     IDBuf[4]        = {0};
//	uint8_t     clear_buf[24]   = {0};
//	uint32_t    ID              = 0;
//    
//	uint16_t    crc16_Cal;
//	uint16_t    crc16_Rec;
//	
//	TIME        system;    
//    
//    /***********************************************
//    * 描述： 
//    */ 
//	if(rx.ok == 1) {
//		crc16_Cal = crc16((uint8_t *)&rx.buf[0],9);
//		crc16_Rec = rx.buf[9] | rx.buf[10]<<8;		  //接收到的crc16
//        
//		if((rx.buf[0]==0xAA)&&(rx.buf[1]==0x55)&&(crc16_Cal == crc16_Rec)) {
//			switch(rx.buf[2])
//			{
//			case 0xA1:				
//				ID = rx.buf[3]<<24 | rx.buf[4]<<16 | rx.buf[5]<<8 | rx.buf[6];
//				TO_Arayy(IDBuf , ID);
//				WriteFM24CL64(64 , IDBuf , 4);
//				printf("\r\n ID设置成功！\r\n ");
//				ReadFM24CL64(64 , IDBuf , 4);
//				printf("\r\n 本装置ID设置为：%02d%02d%02d%02d\r\n" , IDBuf[3],IDBuf[2],IDBuf[1],IDBuf[0]);
//				break;
//                
//			case 0xA2:				
//				ID = rx.buf[3]<<16 | rx.buf[4]<<8 | rx.buf[5];
//				ID = ID | 0xAA000000;
//				TO_Arayy(IDBuf , ID);
//				WriteFM24CL64(68 , IDBuf , 4);
//				printf("\r\n 车型车号设置成功！\r\n ");
//				ReadFM24CL64(68 , IDBuf , 4);
//				if(IDBuf[2] == 0x01)
//					printf("\r\n 装车车型为:HXD1D %02d%02d\r\n" , IDBuf[1],IDBuf[0]);
//				break;
//                
//			case 0xA3:				//设置时间
//				system.Year=rx.buf[3];
//				system.Month=rx.buf[4];
//				system.Day=rx.buf[5];
//				system.Hour=rx.buf[6];
//				system.Min=rx.buf[7];
//				system.Sec=rx.buf[8];
//				WriteTime(system);
//				printf("\r\n系统时间设置成功!\r\n");
//				DisplayTime();
//				break;
//                
//			case 0xA4:				//电压直流零点校准
//				CS5463_AC_Adjust(VDCOFFC);												
//				break;
//			case 0xA5:				//电流直流零点校准
//				CS5463_AC_Adjust(IDCOFFC);										
//				break;
//			case 0xA6:				//电压交流零点校准
//				CS5463_AC_Adjust(VACOFFC);												
//				break;
//			case 0xA7:				//电流交流零点校准
//				CS5463_AC_Adjust(IACOFFC);											
//				break;
//			case 0xA8:				//电压直流增益校准
//				CS5463_AC_Adjust(VACGAINC);									
//				break;
//			case 0xA9:				//电流直流增益校准
//				CS5463_AC_Adjust(IACGAINC);									
//				break;
//                
//			case 0xAB:
//				WriteFM24CL64(32, clear_buf, 24);
//				printf("\r\n 调试数据已清除\n");
//				break;
//			case 0xAC:				//正有功清零
//				AC.PPpulse = 0;
//				AC.PPpower_base = 0;
//				AC.PPpower_temp = 0;
//				printf("\r\n 正有功电量已清零\n");
//				break;
//			case 0xAD:				//负有功清零
//				AC.NPpulse = 0;
//				AC.NPpower_base = 0;
//				AC.NPpower_temp = 0;
//				printf("\r\n 负有功电量已清零\n");
//				break;
//			case 0xAE:				//正无功清零
//				AC.PQpulse = 0;
//				AC.PQpower_base = 0;
//				AC.PQpower_temp = 0;
//				printf("\r\n 正无功电量已清零\n");
//				break;
//			case 0xAF:				//负无功清零
//				AC.NQpulse = 0;
//				AC.NQpower_base = 0;
//				AC.NQpower_temp = 0;
//				printf("\r\n 负无功电量已清零\n");
//				break;
//                
//			case 0x5A:
//				CS5463_AC_Adjust(P_rate_ADD);
//				break;
//			case 0x5B:
//				CS5463_AC_Adjust(P_rate_SUB);
//				break;
//			case 0x5C:
//				CS5463_AC_Adjust(P_rate_DEL);
//				break;
//                
//			case 0x51:				
//				ReadFM24CL64(64 , IDBuf , 4);
//				printf("\r\n 本装置ID为：%02d%02d%02d%02d\r\n" , IDBuf[3],IDBuf[2],IDBuf[1],IDBuf[0]);
//				break;
//                
//		    case 0x52:
//				ReadFM24CL64(68 , IDBuf , 4);
//				if(IDBuf[3] == 0xAA)
//				{
//					if(IDBuf[2] == 0x01)
//						printf("\r\n 装车车型为：HXD1D %02d%02d\r\n" , IDBuf[1],IDBuf[0]);
//				}
//				else 
//					printf("\r\n 未设置装车车型！\r\n");
//				break;
//                
//			case 0x53:				//读取时间
//				DisplayTime();
//				break;
//                
//			case 0x54:						  //通过串口转储FLASH中的所有有效数据
//				DisplayTime();
//				ReadFM24CL64(64 , IDBuf , 4);
//				printf("\r\n 装置编号：%02d%02d%02d%02d\r\n" , IDBuf[3],IDBuf[2],IDBuf[1],IDBuf[0]);
//				ReadFM24CL64(68 , IDBuf , 4);
//				if(IDBuf[3] == 0xAA)
//				{
//					if(IDBuf[2] == 0x01)
//                        printf("\r\n 装车车型为：SS4 %02d%02d\r\n" , IDBuf[1],IDBuf[0]);
//				}
//				else 
//					printf("\r\n 未设置装车车型！\r\n");
//				DOWNLOAD(g_Flash_Adr/32);				
//				if(g_Flash_Adr == 0)
//					printf("\r\n FLASH芯片中无有效数据！\r\n");
//				break;
//                
//			case 0x55:				
//				printf("\r\n FLASH芯片擦除中............\r\n");
//				printf("\r\n 大约需要30秒，请耐心等待！\r\n");
//				MX25L3206_Erase(0, 4096);
//				printf("\r\n FLASH芯片已擦除\r\n");
//				break;
//                
//			case 0x56:
//				printf("\r\n FRAM芯片擦除中...\r\n");
//				WriteFM24CL64(60, fram_clr, 4);
//				g_Flash_Adr = 0;
//				printf("\r\n FRAM芯片已擦除\r\n");
//                
//			default:
//				break;
//			}
//		}
//	}
//	rx.ok = 0;
//	rx.ptr = 0;
//}	

uint8_t     l_COM0RdTmp;        					//串口2状态	
uint8_t     l_COM0TempRev;      					//串口2接收字节

uint8_t		RecTmpFlg0 =0,RecTmpFlg1 =0,RecTmpFlg2 =0;
uint8_t		l_FramRecEndFlg = 0;
uint32_t	COM0RecNumTmp0 = 0,l_COM0RecNumEnd = 0;
uint32_t	l_COM0RecLen = 0;

#define	    RS485_BUF_SIZE  	256
uint8_t 	Rs485RecDataBuf[RS485_BUF_SIZE];
/**-------------------------------------------------------
* @函数名 Usart_2IrqHandlerUser
* @功能   串口2中断处理函数
* @参数   无
* @返回值 无
***------------------------------------------------------*/
void Usart_2IrqHandlerUser()
{
	if(USART_GetITStatus(USART2, USART_IT_RXNE) != RESET) {
        /* clear interrupt */
        USART_ClearITPendingBit(USART2, USART_IT_RXNE);
		/* save on rx buffer */		
		/* save character */
        l_COM0TempRev = USART2->DR & 0x01ff;
        //				printf("%c-", l_COM0TempRev);
        Rs485RecDataBuf[COM0RecNumTmp0 % RS485_BUF_SIZE] = l_COM0TempRev; 
		COM0RecNumTmp0++;
		if(COM0RecNumTmp0 == RS485_BUF_SIZE)
			COM0RecNumTmp0 = 0;
		RecTmpFlg0 = l_COM0TempRev;
        if(RecTmpFlg0 == 0x7E) {
        	printf("\n\r接收到帧尾\n");
			//if(l_FramRecEndFlg == 0)							//数据未处理，不更新
        	{
	        	l_FramRecEndFlg = 1;							//帧接收完成，标志位赋值
	        	
	        	if(COM0RecNumTmp0 > l_COM0RecNumEnd)
	        	{
	           		l_COM0RecLen 	= COM0RecNumTmp0 - l_COM0RecNumEnd;
				}
				else
				{
	           		l_COM0RecLen 	= COM0RecNumTmp0 + (255 - l_COM0RecNumEnd) + 1; //考虑数据溢出
				}
				
	        	l_COM0RecNumEnd 	= COM0RecNumTmp0;
        	}
            
			RecRs485();
    	}
		while (USART2->SR & USART_FLAG_RXNE == 0);
    }
    
    /* If overrun condition occurs, clear the ORE flag 
    and recover communication */
    /*数据溢出*/
    if (USART_GetFlagStatus(USART2, USART_FLAG_ORE) != RESET)
    {
        (void)USART_ReceiveData(USART2);
    }
    /*发送完成*/
	if (USART_GetITStatus(USART2, USART_IT_TC) != RESET)
	{
		/* clear interrupt */
		USART_ClearITPendingBit(USART2, USART_IT_TC);
	}
}


//取接收完成标志  南睿+思維
uint8_t	GetFramRecEndFlg(void)
{
	return	l_FramRecEndFlg;
}

//清接收完成标志  南睿+思維
void  ClearFramRecEndFlg(void)
{
	l_FramRecEndFlg = 0;
}

//取485数据  南睿+思維
uint16_t  ReadRs485Data(uint8_t *Buf)
{
	uint32_t  i;
	
	printf("\r\n RS485 %d %d ",l_COM0RecNumEnd,l_COM0RecLen);
	
	if(l_COM0RecLen > RS485_BUF_SIZE)
	{
		l_COM0RecLen = RS485_BUF_SIZE;
	}
    //	printf("去重复通讯前数据:");
	for(i = 0;i < l_COM0RecLen;i++)
	{
		Buf[i] = Rs485RecDataBuf[(l_COM0RecNumEnd + RS485_BUF_SIZE - l_COM0RecLen + i) 
            % RS485_BUF_SIZE];
        //		printf("%x ",Buf[i]);				
	}
    //	Sendrs485(Buf,l_COM0RecLen);						//打印接收到的数据
	
	return	l_COM0RecLen;
}

///**-------------------------------------------------------
//  * @函数名 USART1_IRQHandler
//  * @功能   串口1中断处理函数
//  * @参数   无
//  * @返回值 无
//***------------------------------------------------------*/
//void USART1_IRQHandler(void)
//{
//    /* 开始临界资源访问，禁止中断 */
////	__disable_irq(); 
//
//    /* 调用串口1中断处理子函数 */
//    Usart_1IrqHandlerUser();
//    
//    /* 结束临界资源访问，开放中断 */
////    __enable_irq();
//}

///**-------------------------------------------------------
//  * @函数名 USART2_IRQHandler
//  * @功能   串口2中断处理函数
//  * @参数   无
//  * @返回值 无
//***------------------------------------------------------*/
//void USART2_IRQHandler(void)
//{
//	/* 开始临界资源访问，禁止中断 */
////	__disable_irq(); 
//
//    /* 调用串口2中断处理子函数 */
//    Usart_2IrqHandlerUser();
////	RecRs485();
//    /* 结束临界资源访问，开放中断 */
////   __enable_irq();
//}

//uint32_t GetSysTime(void)
//{
//	return(SysTime);
//}

SZ_USART_DATA_TypeDef SZ_USART1_DATA, SZ_USART2_DATA;

/**-------------------------------------------------------
* @函数名 UsartIrqHandlerUser
* @功能   共用的串口中断处理函数
* @参数   SZ_USART 串口数据结构体指针
* @返回值 无
***------------------------------------------------------*/
void UsartIrqHandlerUser(SZ_USART_DATA_TypeDef* SZ_USART)
{
    uint8_t uartRxData = 0;
    //	static uint32_t Time = 0;
    //	Time = GetSysTime();
    if(USART_GetITStatus(SZ_USART->uart, USART_IT_RXNE) != RESET) {
        /* save on rx buffer */
        while (SZ_USART->uart->SR & USART_FLAG_RXNE) {
            /* save character */
            uartRxData      = SZ_USART->uart->DR & 0xff;
            rx.buf[rx.ptr]  = uartRxData;
            //			printf("%x",uartRxData);
            rx.ptr++;
        }
        
        /* clear interrupt */
        USART_ClearITPendingBit(SZ_USART->uart, USART_IT_RXNE);
        
        /* 接收完成 */
        if ((uartRxData == '\r') || (rx.ptr >= UART_RX_BUFFER_SIZE)) {
			rx.ok = 1;
            //			printf("接收完成");
        }
    }
    
    
    /* If overrun condition occurs, clear the ORE flag 
    and recover communication */
    if (USART_GetFlagStatus(SZ_USART->uart, USART_FLAG_ORE) != RESET)
    {
        (void)USART_ReceiveData(SZ_USART->uart);
    }
    
	if (USART_GetITStatus(SZ_USART->uart, USART_IT_TC) != RESET)
	{
		/* clear interrupt */
		USART_ClearITPendingBit(SZ_USART->uart, USART_IT_TC);
	}
}

/**-------------------------------------------------------
* @函数名 UsartReadData
* @功能   共用的串口数据读取子函数
* @参数1  SZ_USART 串口数据结构体指针
* @参数2  串口数据存放的Buffer
* @返回值 无
***------------------------------------------------------*/
uint32_t UsartReadData(SZ_USART_DATA_TypeDef* SZ_USART,  uint8_t* Buffer)
{
    uint32_t uartRxLength = 0, i;
    
    /* if the next position is read index, discard this 'read char' */
    if (SZ_USART->uart_rx_length != 0)
    {
        /* 开始临界资源访问，禁止中断 */
        __disable_irq(); 
        
        uartRxLength = SZ_USART->uart_rx_length;
        SZ_USART->uart_rx_length = 0;
        SZ_USART->uart_rx_done = 0;
        
        /* 结束临界资源访问，开放中断 */
        __enable_irq();
        
        for(i=0; i<uartRxLength; i++)
        {
            Buffer[i] = SZ_USART->uartRxBuffer[SZ_USART->uart_rx_read_point];
            SZ_USART->uart_rx_read_point++;
            
            if (SZ_USART->uart_rx_read_point >= UART_RX_BUFFER_SIZE)
            {
                SZ_USART->uart_rx_read_point = 0;
            }
        }
        
        Buffer[i] = '\0';
    }
    
    return uartRxLength;
}

/**-------------------------------------------------------
* @函数名 USART1_IRQHandler
* @功能   串口1中断处理函数
* @参数   无
* @返回值 无
***------------------------------------------------------*/
//void USART1_IRQHandler(void)
//{
//    /* 开始临界资源访问，禁止中断 */
//	__disable_irq(); 
//
//    /* 串口1中断时将指示灯1状态取反 */
////	LED1OBB = !LED1OBB;
//
//    /* 调用共用的串口中断处理子函数 */
//    UsartIrqHandlerUser(&SZ_USART1_DATA);
//    
//    /* 结束临界资源访问，开放中断 */
//    __enable_irq();
//}

//
///*
//    加入以下代码,支持printf函数,不需要选择use MicroLIB	  
//*/
//#ifndef MicroLIB
//#pragma import(__use_no_semihosting)   //没有实现fgetc时需要声明该参数          
///* 标准库需要的支持函数 使用printf()调试打印不需要实现该函数 */               
//struct _Filet//__FILE 
//{ 
//	int handle; 
//    /* Whatever you require here. If the only file you are using is */    
//    /* standard output using printf() for debugging, no file handling */    
//    /* is required. */
//}; 
//
//FILE __stdout;       
////定义_sys_exit()以避免使用半主机模式    
//_sys_exit(int x) 
//{ 
//	x = x; 
//} 
///* 重定义fputc函数 如果使用MicroLIB只需要重定义fputc函数即可 */  
//int fputc(int ch, FILE *f)
//{
//    /* Place your implementation of fputc here */
//    /* Loop until the end of transmission */
//    while (USART_GetFlagStatus(SZ_STM32_COM1, USART_FLAG_TC) == RESET)
//    {}
//
//    /* e.g. write a character to the USART */
//    USART_SendData(SZ_STM32_COM1, (uint8_t) ch);
//
//    return ch;
//}
///*
//可以直接使用putchar
//不需要再定义 int putchar(int ch)，因为stdio.h中有如下定义
// #define putchar(c) putc(c, stdout)
//*/
//
//int ferror(FILE *f) {  
//    /* Your implementation of ferror */  
//    return EOF;  
//} 
//#endif 
//
//FILE __stdin;
//
//int fgetc(FILE *fp)
//{
//	int ch = 0;
//	
//    while(USART_GetFlagStatus(SZ_STM32_COM1, USART_FLAG_RXNE) == RESET)
//    {
//    }
//
//    ch = (int)SZ_STM32_COM1->DR & 0xFF;
//	
//    putchar(ch); //回显
//	
//	return ch;
//}
#define USE_IAR
#ifdef  USE_IAR
#define PUTCHAR_PROTOTYPE int putchar(int ch)
#else
#define PUTCHAR_PROTOTYPE int fputc(int ch,FILE *f)   
#endif

PUTCHAR_PROTOTYPE{
    //HAL_UART_Transmit(&huart1, (char *)(&(ch)), 1, 10);
    //return ch;
    int8_t retrys  = 5;
    
    while (USART_GetFlagStatus(SZ_STM32_COM1, USART_FLAG_TC) == RESET){
        if ( retrys -- < 0 )
            break;
        else
            BSP_OS_TimeDly(1);
    }
    
    /* e.g. write a character to the USART */
    USART_SendData(SZ_STM32_COM1, (uint8_t) ch);
    
    return ch;
} 

#ifdef USE_ADC
/**-------------------------------------------------------
* @函数名 ADC_GPIO_Configuration
* @功能   ADC模数转换的GPIO配置初始化函数
* @参数   无
* @返回值 无
***------------------------------------------------------*/
void ADC_GPIO_Configuration(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;
    
    /* Enable GPIOC clock */
    /* 使能GPIOC时钟      */
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC, ENABLE);	  //
    
    /* Configure PA.01 (ADC Channel12) as analog input -------------------------*/
    //PC0 作为模拟通道11输入引脚                         
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0;       //管脚0
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AIN;   //输入模式
    GPIO_Init(GPIOC, &GPIO_InitStructure);          //GPIO组
}

/* STM32芯片ADC转换结果DR寄存器基地址 */
#define DR_ADDRESS                  ((uint32_t)0x4001244C) 
/* 存放ADC为12位模数转换器结果的变量，只有ADCConvertedValue的低12位有效 */
__IO uint16_t ADCConvertedValue;     // 

/**-------------------------------------------------------
* @函数名 SZ_STM32_ADC_Configuration
* @功能   ADC模数转换的参数配置函数
* @参数   无
* @返回值 无
***------------------------------------------------------*/
void SZ_STM32_ADC_Configuration(void)
{
    ADC_InitTypeDef ADC_InitStructure;        //ADC初始化结构体声明
    DMA_InitTypeDef DMA_InitStructure;        //DMA初始化结构体声明     
    
    ADC_GPIO_Configuration();
    
    /* Enable DMA1 clock */
    RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1, ENABLE);		 //使能DMA时钟
    
    /* DMA1 channel1 configuration ----------------------------------------------*/
    DMA_DeInit(DMA1_Channel1);		  //开启DMA1的第一通道
    DMA_InitStructure.DMA_PeripheralBaseAddr = DR_ADDRESS;		  //DMA对应的外设基地址
    DMA_InitStructure.DMA_MemoryBaseAddr = (uint32_t)&ADCConvertedValue;   //内存存储基地址
    DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralSRC;	//DMA的转换模式为SRC模式，由外设搬移到内存
    DMA_InitStructure.DMA_BufferSize = 1;		   //DMA缓存大小，1个
    DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;	//接收一次数据后，设备地址禁止后移
    DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Disable;	//关闭接收一次数据后，目标内存地址后移
    DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_HalfWord;  //定义外设数据宽度为16位
    DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_HalfWord;  //DMA搬移数据尺寸，HalfWord就是为16位
    DMA_InitStructure.DMA_Mode = DMA_Mode_Circular;   //转换模式，循环缓存模式。
    DMA_InitStructure.DMA_Priority = DMA_Priority_High;	//DMA优先级高
    DMA_InitStructure.DMA_M2M = DMA_M2M_Disable;		  //M2M模式禁用
    DMA_Init(DMA1_Channel1, &DMA_InitStructure);          
    /* Enable DMA1 channel1 */
    DMA_Cmd(DMA1_Channel1, ENABLE);
    
    /* Enable ADC1 and GPIOC clock */
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC1, ENABLE);	  //使能ADC时钟
    
    /* ADC1 configuration ------------------------------------------------------*/
    ADC_InitStructure.ADC_Mode = ADC_Mode_Independent;		//独立的转换模式
    ADC_InitStructure.ADC_ScanConvMode = ENABLE;		  //开启扫描模式
    ADC_InitStructure.ADC_ContinuousConvMode = ENABLE;   //开启连续转换模式
    ADC_InitStructure.ADC_ExternalTrigConv = ADC_ExternalTrigConv_None;	//ADC外部开关，关闭状态
    ADC_InitStructure.ADC_DataAlign = ADC_DataAlign_Right;   //对齐方式,ADC为12位中，右对齐方式
    ADC_InitStructure.ADC_NbrOfChannel = 1;	 //开启通道数，1个
    ADC_Init(ADC1, &ADC_InitStructure);
    
    /* ADC1 regular channel10 configuration */ 
    ADC_RegularChannelConfig(ADC1, ADC_Channel_10, 1, ADC_SampleTime_55Cycles5);
    //ADC通道组， 第1个通道 采样顺序1，转换时间 
    /* Enable ADC1 DMA */
    ADC_DMACmd(ADC1, ENABLE);	  //ADC命令，使能
    /* Enable ADC1 */
    ADC_Cmd(ADC1, ENABLE);  //开启ADC1
    
    /* Enable ADC1 reset calibaration register */   
    ADC_ResetCalibration(ADC1);	  //重新校准
    /* Check the end of ADC1 reset calibration register */
    while(ADC_GetResetCalibrationStatus(ADC1));  //等待重新校准完成
    /* Start ADC1 calibaration */
    ADC_StartCalibration(ADC1);		//开始校准
    /* Check the end of ADC1 calibration */
    while(ADC_GetCalibrationStatus(ADC1));	   //等待校准完成
    /* Start ADC1 Software Conversion */ 
    ADC_SoftwareStartConvCmd(ADC1, ENABLE);	//连续转换开始，ADC通过DMA方式不断的更新RAM区。
}

/**-------------------------------------------------------
* @函数名 GetADCConvertedValue
* @功能   获取ADC模数转换的结果
* @参数   无
* @返回值 ADC转换结果的变量，只有的低12位有效
***------------------------------------------------------*/
uint16_t GetADCConvertedValue(void)
{
    return ADCConvertedValue;
}
#endif

/**-------------------------------------------------------
* @函数名 Buffercmp
* @功能   比较两个缓冲区给定长度的内容是否相同的函数
* @参数1  pBuffer1 和 pBuffer2 需要比较两个缓冲区起始地址
* @参数2  BufferLength 给定的比较长度，字节为单位
* @返回值 PASSED: 缓冲器给定长度的内容相同
*         FAILED: 缓冲器给定长度的内容不相同
***------------------------------------------------------*/
TestStatus Buffercmp(u8* pBuffer1, u8* pBuffer2, u16 BufferLength)
{
    while(BufferLength--)
    {
        if(*pBuffer1 != *pBuffer2)
        {
            return FAILED;
        }
        
        pBuffer1++;
        pBuffer2++;
    }
    
    return PASSED;
}

#ifdef USE_TIM5
/**-------------------------------------------------------
* @函数名 NVIC_TIM5Configuration
* @功能   配置TIM5中断向量参数函数
* @参数   无
* @返回值 无
***------------------------------------------------------*/
static void NVIC_TIM5Configuration(void)
{ 
    NVIC_InitTypeDef NVIC_InitStructure;
    
    /* Set the Vector Table base address at 0x08000000 */
    //NVIC_SetVectorTable(NVIC_VectTab_FLASH, 0x0000);
    
    /* Enable the TIM5 gloabal Interrupt */
    NVIC_InitStructure.NVIC_IRQChannel = TIM5_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 7;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 7;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    
    NVIC_Init(&NVIC_InitStructure);
}

/**-------------------------------------------------------
* @函数名 NVIC_TIM5Configuration
* @功能   配置TIM5参数函数，每秒计数器中断一次 
* @参数   无
* @返回值 无
***------------------------------------------------------*/
void TIM5_Init(void)
{
    TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;
    
    /* TIM5 clock enable */
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM5, ENABLE);
    
    /* ---------------------------------------------------------------
    TIM4 Configuration: Output Compare Timing Mode:
    TIM2CLK = 36 MHz, Prescaler = 7200, TIM2 counter clock = 7.2 MHz
    --------------------------------------------------------------- */
    
    /* Time base configuration */
    //这个就是自动装载的计数值，由于计数是从0开始的，计数10000次后为9999
    TIM_TimeBaseStructure.TIM_Period = (10000 - 1);
    // 这个就是预分频系数，当由于为0时表示不分频所以要减1
    TIM_TimeBaseStructure.TIM_Prescaler = (7200 - 1);
    // 高级应用本次不涉及。定义在定时器时钟(CK_INT)频率与数字滤波器(ETR,TIx)
    // 使用的采样频率之间的分频比例
    TIM_TimeBaseStructure.TIM_ClockDivision = 0;
    //向上计数
    TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
    //初始化定时器5
    TIM_TimeBaseInit(TIM5, &TIM_TimeBaseStructure);
    
    /* Clear TIM5 update pending flag[清除TIM5溢出中断标志] */
    TIM_ClearITPendingBit(TIM5, TIM_IT_Update);
    
    /* TIM IT enable */ //打开溢出中断
    TIM_ITConfig(TIM5, TIM_IT_Update, ENABLE);
    
    /* TIM5 enable counter */
    TIM_Cmd(TIM5, ENABLE);  //计数器使能，开始工作
    
    /* 中断参数配置 */
    NVIC_TIM5Configuration();
}


/**-------------------------------------------------------
* @函数名 TIM5_IRQHandler
* @功能   TIM5中断处理函数，每秒中断一次 
* @参数   无
* @返回值 无
***------------------------------------------------------*/
void TIM5_IRQHandler(void)
{
    /* www.armjishu.com ARM技术论坛 */
    static u32 counter = 0;
    
    if (TIM_GetITStatus(TIM5, TIM_IT_Update) != RESET)
    {
        TIM_ClearITPendingBit(TIM5, TIM_IT_Update);
        /* LED1指示灯状态取反 */
        //        SZ_STM32_LEDToggle(LED1);
        
        /* armjishu.com提心您: 不建议在中断中使用Printf, 此示例只是演示。 */
        printf("\n\rarmjishu.com提示您: 不建议在中断中使用Printf, 此示例只是演示。\n\r");
        printf("ARMJISHU.COM-->TIM5:%d\n\r", counter++);
    }
}
#endif

#ifdef USE_TIM3
/**-------------------------------------------------------
* @函数名 NVIC_TIM3Configuration
* @功能   配置TIM3中断向量参数函数
* @参数   无
* @返回值 无
***------------------------------------------------------*/
static void NVIC_TIM3Configuration(void)
{ 
    NVIC_InitTypeDef NVIC_InitStructure;
    
    /* Set the Vector Table base address at 0x08000000 */
    //NVIC_SetVectorTable(NVIC_VectTab_FLASH, 0x0000);
    
    /* Enable the TIM5 gloabal Interrupt */
    NVIC_InitStructure.NVIC_IRQChannel = TIM3_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 4;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 4;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    
    NVIC_Init(&NVIC_InitStructure);
}

/**-------------------------------------------------------
* @函数名 TIM3_PWM_Init
* @功能   配置TIM3为PWM模式
* @参数   无
* @返回值 无
***------------------------------------------------------*/
void TIM3_PWM_Init(void)
{
    TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;
    TIM_OCInitTypeDef  TIM_OCInitStructure;
  	const uint16_t Period_Val = 1020;
    __IO uint16_t CCR1_Val = Period_Val/2;
    __IO uint16_t CCR2_Val = Period_Val-20;
    uint16_t PrescalerValue = 0;
    
    /* System Clocks Configuration */ /* PCLK1 = HCLK/2 = 36MHz */
    /* TIM3 clock enable */
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3, ENABLE);
    
    /* -----------------------------------------------------------------------
    TIM3 Configuration: generate 4 PWM signals with 4 different duty cycles:
    The TIM3CLK frequency is set to SystemCoreClock (Hz), to get TIM3 counter
    clock at 24 MHz the Prescaler is computed as following:
    - Prescaler = (TIM3CLK / TIM3 counter clock) - 1
    SystemCoreClock is set to 72 MHz for Low-density, Medium-density, High-density
    and Connectivity line devices and to 24 MHz for Low-Density Value line and
    Medium-Density Value line devices
    
    The TIM3 is running at 36 KHz: TIM3 Frequency = TIM3 counter clock/(ARR + 1)
    = 24 MHz / 1000 = 24 KHz
    TIM3 Channel1 duty cycle = (TIM3_CCR1/ TIM3_ARR)* 100 = 50%
    TIM3 Channel2 duty cycle = (TIM3_CCR2/ TIM3_ARR)* 100 = 37.5%
    ----------------------------------------------------------------------- */
    /* Compute the prescaler value */
    PrescalerValue = (uint16_t) (SystemCoreClock / 24000000) - 1;
    /* Time base configuration */
    TIM_TimeBaseStructure.TIM_Period = Period_Val;
    TIM_TimeBaseStructure.TIM_Prescaler = PrescalerValue;
    TIM_TimeBaseStructure.TIM_ClockDivision = 0;
    TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
    
    TIM_TimeBaseInit(TIM3, &TIM_TimeBaseStructure);
    
    /* PWM1 Mode configuration: Channel1 */
    TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_PWM2;
    TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Disable;
    TIM_OCInitStructure.TIM_Pulse = CCR1_Val;
    TIM_OCInitStructure.TIM_OCPolarity = TIM_OCPolarity_High;
    
    TIM_OC1Init(TIM3, &TIM_OCInitStructure);
    
    TIM_OC1PreloadConfig(TIM3, TIM_OCPreload_Enable);
    
    /* PWM1 Mode configuration: Channel2 */
    TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Disable;
    TIM_OCInitStructure.TIM_Pulse = CCR2_Val;
    
    TIM_OC2Init(TIM3, &TIM_OCInitStructure);
    
    TIM_OC2PreloadConfig(TIM3, TIM_OCPreload_Enable);
    
    /* TIM IT enable */
    TIM_ITConfig(TIM3, TIM_IT_CC1 | TIM_IT_CC2, ENABLE);
    
    //TIM_ARRPreloadConfig(TIM3, ENABLE);
    
    /* TIM3 enable counter */
    TIM_Cmd(TIM3, ENABLE);
    
    NVIC_TIM3Configuration();
}



/**-------------------------------------------------------
* @函数名 TIM3_IRQHandler
* @功能   TIM3中断处理函数，PWM控制指示灯亮度
* @参数   无
* @返回值 无
***------------------------------------------------------*/
void TIM3_IRQHandler(void)
{
    if (TIM_GetITStatus(TIM3, TIM_IT_CC1) != RESET)
    {
        TIM_ClearITPendingBit(TIM3, TIM_IT_CC1);
    }
    
    if (TIM_GetITStatus(TIM3, TIM_IT_CC2) != RESET)
    {
        TIM_ClearITPendingBit(TIM3, TIM_IT_CC2);
    }
}
#endif

#ifdef USE_IWDG
/**-------------------------------------------------------
* @函数名 SZ_STM32_IWDGInit
* @功能   STM32独立看门狗初始化配置函数
* @参数   TimeoutFreMs  看门狗定时器多少毫秒溢出一次
* @返回值 无
* @喂狗方式(Reload IWDG counter) IWDG_ReloadCounter(); 
***------------------------------------------------------*/
void SZ_STM32_IWDGInit(uint32_t TimeoutFreMs)/* 2 -  3276*/
{
    uint32_t ReloadValue;
    
    /* IWDG timeout equal to 280 ms (the timeout may varies due to LSI frequency
    dispersion) */
    /* Enable write access to IWDG_PR and IWDG_RLR registers */
    IWDG_WriteAccessCmd(IWDG_WriteAccess_Enable);
    
    /* IWDG counter clock: 40KHz(LSI) / 32 = 1.25 KHz */
    IWDG_SetPrescaler(IWDG_Prescaler_32);
    
    /* Set counter reload value to 349 *//* range from 0 to 4095 */
    //IWDG_SetReload(349);
    if(TimeoutFreMs < 2)
    {
        TimeoutFreMs = 2;
    }
    
    if(TimeoutFreMs > 3276)
    {
        TimeoutFreMs = 3276;
    }
    
    ReloadValue = ((TimeoutFreMs * 1250)/1000) - 1;
    IWDG_SetReload(ReloadValue);
    
    printf("\r\n IWDG Timeout frequency %dms, ReloadValue is set to %d.", TimeoutFreMs, ReloadValue);
    
    /* Reload IWDG counter */
    IWDG_ReloadCounter();
    
    /* Enable IWDG (the LSI oscillator will be enabled by hardware) */
    IWDG_Enable();
    
}
#endif


#ifdef  USE_FULL_ASSERT
// 需要在工程设置Option(快捷键ALT+F7)C++属性页的define栏输入"USE_FULL_ASSERT"
/**
* @brief  Reports the name of the source file and the source line number
*   where the assert_param error has occurred.
* @param  file: pointer to the source file name
* @param  line: assert_param error line source number
* @retval None
*/
void assert_failed(uint8_t* file, uint32_t line)
{ 
    /* User can add his own implementation to report the file name and line number,
    ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
    
    printf("Wrong parameters value: file %s on line %d\r\n", file, line);
    
    /* Infinite loop */
    while (1)
    {
    }
}
#endif

/******************* (C) COPYRIGHT 2013 www.armjishu.com *****END OF FILE****/
