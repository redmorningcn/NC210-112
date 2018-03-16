/********************   (C) COPYRIGHT 2013 www.armjishu.com   ********************
 * 文件名  ：SZ_STM32F107VC_LIB.h
 * 描述    ：提供STM32F107VC神舟IV号开发板的库函数
 * 实验平台：STM32神舟开发板
 * 作者    ：www.armjishu.com 
**********************************************************************************/
/* Includes ------------------------------------------------------------------*/
#include "stm32f10x.h"
#include <stdio.h>

#ifdef __cplusplus
 extern "C" {
#endif

/* 功能宏开关  如果未使用这些功能，不打开对应的宏开关可以减小编译结果的大小 */
//#define       USE_ADC
//#define       USE_RTC
//#define       USE_TIM5
//#define       USE_TIM3
//#define       USE_PVD
//#define       USE_IWDG
//#define       USE_RTC_ALARM

/* 别名区宏定义 bitband macro ------------------------------------------------*/
/* 使用bitband可以提高设计bit操作的程序的效率，外设GPIO管脚的控制效果尤为明显 */
/* 外设别名区 对32MB外设别名区的访问映射为对1MB 外设bit-band区的访问(实际大小根据芯片有关) */
#define Periph_BASE         0x40000000  // 外设基地址 Peripheral 
#define Periph_BB_BASE      0x42000000  // 外设别名区基地址 Peripheral bitband

/* 注意：传入常量参数时，在编译时编译器会计算出别名区地址，这样能达到提升效率的目的(推荐)
         如果传入变量参数，只能在运行时由STM32自己计算别名区地址，效率会大打折扣(不推荐) */
#define Periph_BB(PeriphAddr, BitNumber)    \
          *(__IO uint32_t *) (Periph_BB_BASE | ((PeriphAddr - Periph_BASE) << 5) | ((BitNumber) << 2))
	 
#define Periph_ResetBit_BB(PeriphAddr, BitNumber)    \
          (*(__IO uint32_t *) (Periph_BB_BASE | ((PeriphAddr - Periph_BASE) << 5) | ((BitNumber) << 2)) = 0)
   
#define Periph_SetBit_BB(PeriphAddr, BitNumber)       \
          (*(__IO uint32_t *) (Periph_BB_BASE | ((PeriphAddr - Periph_BASE) << 5) | ((BitNumber) << 2)) = 1)

#define Periph_GetBit_BB(PeriphAddr, BitNumber)       \
          (*(__IO uint32_t *) (Periph_BB_BASE | ((PeriphAddr - Periph_BASE) << 5) | ((BitNumber) << 2)))

/* 外设GPIO输入输出别名区定义，n为bit位置范围为0到15    */
/* 我们对GPIOA.15操作，首先要初始化GPIO，然后使用方法： */
/* 对GPIOA.15输出低电平：   PAOutBit(15) = 0;           */
/* 对GPIOA.15输出低电平：   PAOutBit(15) = 1;           */
/* 读取GPIOA.15输入的电平： data = PAInBit(15);         */
#define PAOutBit(n)     Periph_BB((uint32_t)&GPIOA->IDR,n)  //输出 
#define PASetBit(n)     (PAOutBit(n) = 1)                   //输出 高
#define PAResetBit(n)   (PAOutBit(n) = 0)                   //输出 低
#define PAInBit(n)      Periph_BB((uint32_t)&GPIOA->IDR,n)  //输入 

#define PBOutBit(n)     Periph_BB((uint32_t)&GPIOB->ODR,n)  //输出 
#define PBSetBit(n)     (PBOutBit(n) = 1)                   //输出 高
#define PBResetBit(n)   (PBOutBit(n) = 0)                   //输出 低
#define PBInBit(n)      Periph_BB((uint32_t)&GPIOB->IDR,n)  //输入 

#define PCOutBit(n)     Periph_BB((uint32_t)&GPIOC->ODR,n)  //输出 
#define PCSetBit(n)     (PCOutBit(n) = 1)                   //输出 高
#define PCResetBit(n)   (PCOutBit(n) = 0)                   //输出 低
#define PCInBit(n)      Periph_BB((uint32_t)&GPIOC->IDR,n)  //输入 

#define PDOutBit(n)     Periph_BB((uint32_t)&GPIOD->ODR,n)  //输出 
#define PDSetBit(n)     (PDOutBit(n) = 1)                   //输出 高
#define PDResetBit(n)   (PDOutBit(n) = 0)                   //输出 低
#define PDInBit(n)      Periph_BB((uint32_t)&GPIOD->IDR,n)  //输入 

#define PEOutBit(n)     Periph_BB((uint32_t)&GPIOE->ODR,n)  //输出 
#define PESetBit(n)     (PEOutBit(n) = 1)                   //输出 高
#define PEResetBit(n)   (PEOutBit(n) = 0)                   //输出 低
#define PEInBit(n)      Periph_BB((uint32_t)&GPIOE->IDR,n)  //输入

#define PFOutBit(n)     Periph_BB((uint32_t)&GPIOF->ODR,n)  //输出 
#define PFSetBit(n)     (PFOutBit(n) = 1)                   //输出 高
#define PFResetBit(n)   (PFOutBit(n) = 0)                   //输出 低
#define PFInBit(n)      Periph_BB((uint32_t)&GPIOF->IDR,n)  //输入

#define PGOutBit(n)     Periph_BB((uint32_t)&GPIOG->ODR,n)  //输出 
#define PGSetBit(n)     (PGOutBit(n) = 1)                   //输出 高
#define PGResetBit(n)   (PGOutBit(n) = 0)                   //输出 低
#define PGInBit(n)      Periph_BB((uint32_t)&GPIOG->IDR,n)  //输入


/* 内部SRAM别名区 对32MB SRAM别名区的访问映射为对1MB SRAMbit-band区的访问(实际大小根据芯片有关) */
#define RAM_BASE            0x20000000  // 内部SRAM基地址  
#define RAM_BB_BASE         0x22000000  // 内部SRAM别名区基地址

#define SRAM_ResetBit_BB(VarAddr, BitNumber)    \
          (*(__IO uint32_t *) (RAM_BB_BASE | ((VarAddr - RAM_BASE) << 5) | ((BitNumber) << 2)) = 0)
   
#define SRAM_SetBit_BB(VarAddr, BitNumber)       \
          (*(__IO uint32_t *) (RAM_BB_BASE | ((VarAddr - RAM_BASE) << 5) | ((BitNumber) << 2)) = 1)

#define SRAM_GetBit_BB(VarAddr, BitNumber)       \
          (*(__IO uint32_t *) (RAM_BB_BASE | ((VarAddr - RAM_BASE) << 5) | ((BitNumber) << 2)))

typedef enum 
{
  COM1 = 0,   
  COM2 = 1
} COM_TypeDef;   

#define UART_RX_BUFFER_SIZE    11
typedef struct 
{
  USART_TypeDef* uart;   
  uint32_t uart_rx_length;
  uint32_t uart_rx_done;
  uint32_t uart_rx_write_length;
  uint32_t uart_rx_read_point;
  uint32_t uart_rx_write_point;
  uint8_t uartRxBuffer[UART_RX_BUFFER_SIZE];
} SZ_USART_DATA_TypeDef;

extern SZ_USART_DATA_TypeDef SZ_USART1_DATA, SZ_USART2_DATA;

typedef struct
{
	uint8_t				buf[20];										//命令内容
	uint8_t				ptr;
	uint8_t             ok;
	uint32_t			timeptr;
}UART1;

extern UART1 rx,tx;

typedef enum {FAILED = 0, PASSED = !FAILED} TestStatus;

/** 串口管脚资源个数定义  **/
#define COMn                             2

/** 串口1管脚资源定义  **/
#define SZ_STM32_COM1_STR                "USART1"
#define SZ_STM32_COM1                    USART1
#define SZ_STM32_COM1_CLK                RCC_APB2Periph_USART1
#define SZ_STM32_COM1_TX_PIN             GPIO_Pin_9
#define SZ_STM32_COM1_TX_GPIO_PORT       GPIOA
#define SZ_STM32_COM1_TX_GPIO_CLK        RCC_APB2Periph_GPIOA
#define SZ_STM32_COM1_RX_PIN             GPIO_Pin_10
#define SZ_STM32_COM1_RX_GPIO_PORT       GPIOA
#define SZ_STM32_COM1_RX_GPIO_CLK        RCC_APB2Periph_GPIOA
#define SZ_STM32_COM1_IRQn               USART1_IRQn

/** 串口2管脚资源定义 (USART2 pins remapped on GPIOD) **/
#define SZ_STM32_COM2_STR                "USART2"
#define SZ_STM32_COM2                    USART2
#define SZ_STM32_COM2_CLK                RCC_APB1Periph_USART2
#define SZ_STM32_COM2_TX_PIN             GPIO_Pin_2
#define SZ_STM32_COM2_TX_GPIO_PORT       GPIOA
#define SZ_STM32_COM2_TX_GPIO_CLK        RCC_APB2Periph_GPIOA
#define SZ_STM32_COM2_RX_PIN             GPIO_Pin_3
#define SZ_STM32_COM2_RX_GPIO_PORT       GPIOA
#define SZ_STM32_COM2_RX_GPIO_CLK        RCC_APB2Periph_GPIOA
#define SZ_STM32_COM2_IRQn               USART2_IRQn

extern const uint8_t STM32F10x_STR[];
extern __IO uint32_t TimingDelay;
extern uint32_t STM32DeviceSerialID[3]; /* 全局变量IntDeviceSerial存放读到的设备ID */
extern __IO uint16_t ADCConvertedValue;  /* 12位ADC模数转换结果，只有ADCConvertedValue的低12位有效 */
extern __IO uint32_t TimeDisplay;

/** 通用函数声明  **/  
void delay(__IO uint32_t nCount);
void NVIC_GroupConfig(void);
void SZ_STM32_SysTickInit(uint32_t HzPreSecond);
void SysTickDelay(__IO uint32_t nTime);
void TimingDelay_Decrement(void);

void GetDeviceSerialID(void);

extern void __SZ_STM32_COMInit(COM_TypeDef COM, USART_InitTypeDef* USART_InitStruct);
extern void SZ_STM32_COMInit(COM_TypeDef COM, uint32_t BaudRate);

/**-------------------------------------------------------
  * @函数名 NVIC_COMConfiguration
  * @功能   配置串口1和串口2的中断参数
  * @参数   无
  * @返回值 无
***------------------------------------------------------*/
extern void NVIC_COMConfiguration(void);


//取接收完成标志  南睿+思维
extern uint8_t	GetFramRecEndFlg(void);

//清接收完成标志  南睿+思维
extern void  ClearFramRecEndFlg(void);

//取485数据  南睿+思维
extern uint16_t  ReadRs485Data(uint8_t *Buf);

extern uint32_t UsartReadData(SZ_USART_DATA_TypeDef* SZ_USART,  uint8_t* Buffer);

extern void Uart_1Control(void);
/**-------------------------------------------------------
  * @函数名 USART1_IRQHandler
  * @功能   串口1中断处理函数
  * @参数   无
  * @返回值 无
***------------------------------------------------------*/
extern void USART1_IRQHandler(void);

/**-------------------------------------------------------
  * @函数名 USART2_IRQHandler
  * @功能   串口2中断处理函数
  * @参数   无
  * @返回值 无
***------------------------------------------------------*/
extern void USART2_IRQHandler(void);

void SZ_STM32_ADC_Configuration(void);
uint16_t GetADCConvertedValue(void);
 
TestStatus Buffercmp(u8* pBuffer1, u8* pBuffer2, u16 BufferLength);
    
void TIM5_Init(void);

void TIM3_PWM_Init(void);

void SZ_STM32_IWDGInit(uint32_t TimeoutFreMs);

void SZ_STM32_PVDConfiguration(void);

#ifdef __cplusplus
}
#endif
/******************* (C) COPYRIGHT 2013 www.armjishu.com *****END OF FILE****/

