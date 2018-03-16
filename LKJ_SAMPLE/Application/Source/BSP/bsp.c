/*
*********************************************************************************************************
*                                     MICIRUM BOARD SUPPORT PACKAGE
*
*                             (c) Copyright 2013; Micrium, Inc.; Weston, FL
*
*               All rights reserved.  Protected by international copyright laws.
*               Knowledge of the source code may NOT be used to develop a similar product.
*               Please help us continue to provide the Embedded community with the finest
*               software available.  Your honesty is greatly appreciated.
*********************************************************************************************************
*/

/*
*********************************************************************************************************
*
*                                        BOARD SUPPORT PACKAGE
*
*                                     ST Microelectronics STM32
*                                              on the
*
*                                     Micrium uC-Eval-STM32F107
*                                        Evaluation Board
*
* Filename      : bsp.c
* Version       : V1.00
* Programmer(s) : EHS
*********************************************************************************************************
*/

/*
*********************************************************************************************************
*                                             INCLUDE FILES
*********************************************************************************************************
*/

#define  BSP_MODULE
#include <bsp.h>


/*
*********************************************************************************************************
*                                            LOCAL DEFINES
*********************************************************************************************************
*/


/*
*********************************************************************************************************
*                                           LOCAL CONSTANTS
*********************************************************************************************************
*/

#define  BSP_LED_START_BIT  (8 - 1)                            /* LEDs[3:1] are sequentially connected to PTD[15:13].  */


/*
*********************************************************************************************************
*                                          LOCAL DATA TYPES
*********************************************************************************************************
*/


/*
*********************************************************************************************************
*                                            LOCAL TABLES
*********************************************************************************************************
*/


/*
*********************************************************************************************************
*                                       LOCAL GLOBAL VARIABLES
*********************************************************************************************************
*/

CPU_INT08U  BSP_CPU_ClkFreq_MHz;


/*
*********************************************************************************************************
*                                      LOCAL FUNCTION PROTOTYPES
*********************************************************************************************************
*/
static  void  BSP_GpioInit   (void);
static  void  BSP_LED_Init   (void);
static  void  BSP_StatusInit (void);

/*
*********************************************************************************************************
*                                             REGISTERS
*********************************************************************************************************
*/

#define  DWT_CR      *(CPU_REG32 *)0xE0001000
#define  DWT_CYCCNT  *(CPU_REG32 *)0xE0001004
#define  DEM_CR      *(CPU_REG32 *)0xE000EDFC
#define  DBGMCU_CR   *(CPU_REG32 *)0xE0042004


/*
*********************************************************************************************************
*                                            REGISTER BITS
*********************************************************************************************************
*/

#define  DBGMCU_CR_TRACE_IOEN_MASK       0x10
#define  DBGMCU_CR_TRACE_MODE_ASYNC      0x00
#define  DBGMCU_CR_TRACE_MODE_SYNC_01    0x40
#define  DBGMCU_CR_TRACE_MODE_SYNC_02    0x80
#define  DBGMCU_CR_TRACE_MODE_SYNC_04    0xC0
#define  DBGMCU_CR_TRACE_MODE_MASK       0xC0

#define  DEM_CR_TRCENA                   (1 << 24)

#define  DWT_CR_CYCCNTENA                (1 <<  0)


/*
*********************************************************************************************************
*                                     LOCAL CONFIGURATION ERRORS
*********************************************************************************************************
*/
/*
#if ((CPU_CFG_TS_TMR_EN          != DEF_ENABLED) && \
     (APP_CFG_PROBE_OS_PLUGIN_EN == DEF_ENABLED) && \
     (OS_PROBE_HOOKS_EN          >  0u))
#error  "CPU_CFG_TS_EN                  illegally #define'd in 'cpu.h'"
#error  "                              [MUST be  DEF_ENABLED] when    "
#error  "                               using uC/Probe COM modules    "
#endif

*/
void BSP_RccInit( u8 type, u8 freq )
{
    u32 mul = RCC_PLLMul_12;
    u32 div = RCC_PLLSource_HSI_Div2;
    
    RCC_DeInit();
    
    switch(freq) {
    case  8: mul    = RCC_PLLMul_2 ;            break;  // 4 * 2    = 8M
    case 12: mul    = RCC_PLLMul_3 ;            break;  // 4 * 3    = 12M
    case 16: mul    = RCC_PLLMul_4 ;            break;  // 4 * 4    = 16M
    case 20: mul    = RCC_PLLMul_5 ;            break;  // 4 * 5    = 20M
    case 24: mul    = RCC_PLLMul_6 ;            break;  // 4 * 6    = 24M
    case 28: mul    = RCC_PLLMul_7 ;            break;  // 4 * 7    = 28M
    case 32: mul    = RCC_PLLMul_8 ;            break;  // 4 * 8    = 32M
    case 36: mul    = RCC_PLLMul_9 ;            break;  // 4 * 9    = 36M
    case 40: mul    = RCC_PLLMul_10;            break;  // 4 * 10   = 40M
    case 44: mul    = RCC_PLLMul_11;            break;  // 4 * 11   = 44M
    case 48: mul    = RCC_PLLMul_12;            break;  // 4 * 12   = 48M
    case 52: mul    = RCC_PLLMul_13;            break;  // 4 * 13   = 52M
    case 56: mul    = RCC_PLLMul_14;            break;  // 4 * 14   = 56M
    case 60: mul    = RCC_PLLMul_15;            break;  // 4 * 15   = 60M
    case 64: mul    = RCC_PLLMul_16;            break;  // 4 * 16   = 64M  
    case 72: mul    = RCC_PLLMul_9; type = 1;   break;  // 8 * 9    = 72M  
    }
    
    if ( type == 1 ) {
        RCC_HSEConfig(RCC_HSE_ON);
        
        if ( SUCCESS == RCC_WaitForHSEStartUp() ) {
            if ( freq == 72 )
                div = RCC_PLLSource_HSE_Div1;
            else
                div = RCC_PLLSource_HSE_Div2;
        } else {
            mul = RCC_PLLMul_16;
            div = RCC_PLLSource_HSI_Div2;
            goto hsi_init;
        }
    } else {
    hsi_init:
        RCC_HSEConfig(RCC_HSE_OFF);
        RCC_HSICmd(ENABLE);
    }
    
    RCC_HCLKConfig(RCC_SYSCLK_Div1);
    RCC_PCLK2Config(RCC_HCLK_Div1);
    RCC_PCLK1Config(RCC_HCLK_Div2);
    RCC_ADCCLKConfig(RCC_PCLK2_Div6);
    
    FLASH_SetLatency(FLASH_Latency_2);
    FLASH_PrefetchBufferCmd(FLASH_PrefetchBuffer_Enable);
    RCC_PLLConfig(div, mul); 
    RCC_PLLCmd(ENABLE); 
    
    while (RCC_GetFlagStatus(RCC_FLAG_PLLRDY) == RESET) { ; }
    RCC_SYSCLKConfig(RCC_SYSCLKSource_PLLCLK);
    while (RCC_GetSYSCLKSource() != 0x08) { ; }
}
/*
*********************************************************************************************************
*                                               BSP_Init()
*
* Description : Initialize the Board Support Package (BSP).
*
* Argument(s) : none.
*
* Return(s)   : none.
*
* Caller(s)   : Application.
*
* Note(s)     : (1) This function SHOULD be called before any other BSP function is called.
*
*               (2) CPU instruction / data tracing requires the use of the following pins :
*                   (a) (1) Aysynchronous     :  PB[3]
*                       (2) Synchronous 1-bit :  PE[3:2]
*                       (3) Synchronous 2-bit :  PE[4:2]
*                       (4) Synchronous 4-bit :  PE[6:2]
*
*                   (b) The uC-Eval board MAY utilize the following pins depending on the application :
*                       (1) PE[5], MII_INT
*                       (1) PE[6], SDCard_Detection
*
*                   (c) The application may wish to adjust the trace bus width depending on I/O
*                       requirements.
*********************************************************************************************************
*/

void  BSP_Init (void)
{
    /***********************************************
    * 描述： 恢复GPIO及USART为默认参数，防止进入应用程序发生异常
    *        如果，串口有数据，则在进入应用程序后会进入中断产生异常
    */
    /***********************************************
    * 描述： 端口恢复默认
    */
    GPIO_DeInit(GPIOA);
    GPIO_DeInit(GPIOB);
    GPIO_DeInit(GPIOC);
    GPIO_DeInit(GPIOD);
    GPIO_DeInit(GPIOE);
    
    /***********************************************
    * 描述： 串口恢复默认
    */
    USART_DeInit(USART1);
    USART_DeInit(USART2);
    USART_DeInit(USART3);
        
    /***********************************************
    * 描述： 定时器恢复默认
    */
    TIM_DeInit(TIM1);
    TIM_DeInit(TIM2);
    TIM_DeInit(TIM2);
    TIM_DeInit(TIM3);
    TIM_DeInit(TIM4);
    TIM_DeInit(TIM5);
    TIM_DeInit(TIM6);
    TIM_DeInit(TIM7);
    TIM_DeInit(TIM8);

    ADC_DeInit(ADC1);
    ADC_DeInit(ADC2);
    
    BSP_IntInit();
    BSP_GpioInit();
    BSP_RccInit(1,72);

    BSP_CPU_ClkFreq_MHz = BSP_CPU_ClkFreq() / (CPU_INT32U)1000000;

    BSP_CPU_ClkFreq_MHz = BSP_CPU_ClkFreq_MHz;                  /* Surpress compiler warning BSP_CPU_ClkFreq_MHz    ... */
                                                                /* ... set and not used.                                */

    BSP_LED_Init();                                             /* Initialize the I/Os for the LED      controls.       */

    BSP_StatusInit();                                           /* Initialize the status input(s)                       */
        
#ifdef TRACE_EN                                                 /* See project / compiler preprocessor options.         */
    DBGMCU_CR |=  DBGMCU_CR_TRACE_IOEN_MASK;                    /* Enable tracing (see Note #2).                        */
    DBGMCU_CR &= ~DBGMCU_CR_TRACE_MODE_MASK;                    /* Clr trace mode sel bits.                             */
    DBGMCU_CR |=  DBGMCU_CR_TRACE_MODE_SYNC_04;                 /* Cfg trace mode to synch 4-bit.                       */
#endif
}
/*
*********************************************************************************************************
*                                            BSP_GpioInit()
*
* Description : Read CPU registers to determine the CPU clock frequency of the chip.
*
* Argument(s) : none.
*
* Return(s)   : The CPU clock frequency, in Hz.
*
* Caller(s)   : Application.
*
* Note(s)     : none.
*********************************************************************************************************
*/

void BSP_GpioInit(void)
{
    GPIO_InitTypeDef gpio_init;

    /* Configure all unused GPIO port pins in Analog Input mode (floating input
     trigger OFF), this will reduce the power consumption and increase the device
     immunity against EMI/EMC *************************************************/
    RCC_APB2PeriphClockCmd( RCC_APB2Periph_GPIOA |
                            RCC_APB2Periph_GPIOB |
                            RCC_APB2Periph_GPIOC |
                            RCC_APB2Periph_GPIOD ,  ENABLE);

    gpio_init.GPIO_Pin  = GPIO_Pin_All;
    gpio_init.GPIO_Mode = GPIO_Mode_AIN;

    GPIO_Init(GPIOA, &gpio_init);
    GPIO_Init(GPIOB, &gpio_init);
    GPIO_Init(GPIOC, &gpio_init);
    GPIO_Init(GPIOD, &gpio_init);

    RCC_APB2PeriphClockCmd( RCC_APB2Periph_GPIOA |
                            RCC_APB2Periph_GPIOB |
                            RCC_APB2Periph_GPIOC |
                            RCC_APB2Periph_GPIOD ,  DISABLE);
}

/*
*********************************************************************************************************
*                                            BSP_CPU_ClkFreq()
*
* Description : Read CPU registers to determine the CPU clock frequency of the chip.
*
* Argument(s) : none.
*
* Return(s)   : The CPU clock frequency, in Hz.
*
* Caller(s)   : Application.
*
* Note(s)     : none.
*********************************************************************************************************
*/

CPU_INT32U  BSP_CPU_ClkFreq (void)
{
    RCC_ClocksTypeDef  rcc_clocks;


    RCC_GetClocksFreq(&rcc_clocks);

    return ((CPU_INT32U)rcc_clocks.HCLK_Frequency);
}


/*
*********************************************************************************************************
*********************************************************************************************************
*                                              LED FUNCTIONS
*********************************************************************************************************
*********************************************************************************************************
*/

/*
*********************************************************************************************************
*                                             BSP_LED_Init()
*
* Description : Initialize the I/O for the LEDs
*
* Argument(s) : none.
*
* Return(s)   : none.
*
* Caller(s)   : BSP_Init().
*
* Note(s)     : none.
*********************************************************************************************************
*/
static  void  BSP_LED_Init (void)
{
    GPIO_InitTypeDef  gpio_init;
        
    RCC_APB2PeriphClockCmd(RCC_LED, ENABLE);

    gpio_init.GPIO_Pin   = BSP_GPIO_LED01 | BSP_GPIO_LED02 ;
    gpio_init.GPIO_Speed = GPIO_Speed_50MHz;
    gpio_init.GPIO_Mode  = GPIO_Mode_Out_PP;
    GPIO_Init(GPIO_LED, &gpio_init);

    BSP_LED_Off(BSP_LED_ALL);
}

/*
*********************************************************************************************************
*                                             BSP_LED_On()
*
* Description : Turn ON any or all the LEDs on the board.
*
* Argument(s) : led     The ID of the LED to control:
*
*                       0    turn ON all LEDs on the board
*                       1    turn ON LED 1
*                       2    turn ON LED 2
*                       3    turn ON LED 3
*                       4    turn ON LED 4
*
* Return(s)   : none.
*
* Caller(s)   : Application.
*
* Note(s)     : none.
*********************************************************************************************************
*/

void  BSP_LED_On (CPU_INT08U led)
{
    switch (led) {
        case 0:
             GPIO_SetBits(GPIO_LED, BSP_GPIO_LED01);
             GPIO_SetBits(GPIO_LED, BSP_GPIO_LED02);
             break;
        case 1:
             GPIO_SetBits(GPIO_LED, BSP_GPIO_LED01);
             break;
        case 2:
             GPIO_SetBits(GPIO_LED, BSP_GPIO_LED02);
             break;

        default:
             break;
    }
}

/*
*********************************************************************************************************
*                                              BSP_LED_Off()
*
* Description : Turn OFF any or all the LEDs on the board.
*
* Argument(s) : led     The ID of the LED to control:
*
*                       0    turn OFF all LEDs on the board
*                       1    turn OFF LED 1
*                       2    turn OFF LED 2
*                       3    turn OFF LED 3
*                       4    turn OFF LED 4
*
* Return(s)   : none.
*
* Caller(s)   : Application.
*
* Note(s)     : none.
*********************************************************************************************************
*/

void  BSP_LED_Off (CPU_INT08U led)
{
    switch (led) {
        case 0:
             GPIO_ResetBits(GPIO_LED, BSP_GPIO_LED01);
             GPIO_ResetBits(GPIO_LED, BSP_GPIO_LED02);
             break;
        case 1:
             GPIO_ResetBits(GPIO_LED, BSP_GPIO_LED01);
             break;
        case 2:
             GPIO_ResetBits(GPIO_LED, BSP_GPIO_LED02);
             break;

        default:
             break;
    }
}

/*
*********************************************************************************************************
*                                            BSP_LED_Toggle()
*
* Description : TOGGLE any or all the LEDs on the board.
*
* Argument(s) : led     The ID of the LED to control:
*
*                       0    TOGGLE all LEDs on the board
*                       1    TOGGLE LED 1
*                       2    TOGGLE LED 2
*                       3    TOGGLE LED 3
*                       4    TOGGLE LED 4
*
* Return(s)   : none.
*
* Caller(s)   : Application.
*
* Note(s)     : none.
*********************************************************************************************************
*/

void  BSP_LED_Toggle (CPU_INT08U led)
{
    CPU_INT32U  pins;


    pins = GPIO_ReadOutputData(GPIO_LED);

    switch (led) {
        case 0:
             BSP_LED_Toggle(1);
             BSP_LED_Toggle(2);
             break;

        case 1:
             if ((pins & BSP_GPIO_LED01) == 0) {
                 GPIO_SetBits   (GPIO_LED, BSP_GPIO_LED01);
             } else {
                 GPIO_ResetBits (GPIO_LED, BSP_GPIO_LED01);
             }
            break;
        case 2:
             if ((pins & BSP_GPIO_LED02) == 0) {
                 GPIO_SetBits   (GPIO_LED, BSP_GPIO_LED02);
             } else {
                 GPIO_ResetBits (GPIO_LED, BSP_GPIO_LED02);
             }
            break;

        default:
             break;
    }
}

/*
*********************************************************************************************************
*                                            BSP_LED_Flash()
*
* Description : Flash any or all the LEDs on the board.
*
* Argument(s) : led     The ID of the LED to control:
*
*                       0    TOGGLE all LEDs on the board
*                       1    TOGGLE LED 1
*                       2    TOGGLE LED 2
*                       3    TOGGLE LED 3
*                       4    TOGGLE LED 4
*
* Return(s)   : none.
*
* Caller(s)   : Application.
*
* Note(s)     : none.
*********************************************************************************************************
*/
void BSP_LED_Flash( CPU_INT08U led, CPU_INT16U cnt, CPU_INT32U cycle, CPU_INT32U duty)
{
    CPU_INT32U  timeOn;
    CPU_INT32U  timeOff;
    CPU_INT32U  i;

    if ( cycle < duty )
      return;
    if( duty == 0 )
      return;

    timeOn      = duty;
    timeOff     = cycle - duty;

    /***********************************************
    * 描述： 声音提示
    */
    for ( i = 0; i < cnt; i++  ) {
      BSP_LED_On(led);
      BSP_OS_TimeDly(timeOn);
      BSP_LED_Off(led);
      if ( i+1 == cnt)
        break;
      BSP_OS_TimeDly(timeOff);
    }
}
/*
*********************************************************************************************************
*                                            BSP_StatusInit()
*
* Description : Initialize the status port(s)
*
* Argument(s) : none.
*
* Return(s)   : none.
*
* Caller(s)   : BSP_Init()
*
* Note(s)     : none.
*********************************************************************************************************
*/

static  void  BSP_StatusInit (void)
{
    GPIO_InitTypeDef  GPIO_InitStructure;
    
    /***********************************************
    * 描述： 模块地址选择：0：CSNC A2 / Modbus 2；1：CSNC A2 / Modbus 1
    */
    RCC_APB2PeriphClockCmd(GPIO_RD0_RCC, ENABLE);
    GPIO_InitStructure.GPIO_Pin   = GPIO_RD0_PIN; 
    GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_IPU;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIO_RD0_PROT, &GPIO_InitStructure);
    
    /***********************************************
    * 描述： 模块类型选择：0: 测量模块；1：显示模块
    */
    RCC_APB2PeriphClockCmd(GPIO_RD1_RCC, ENABLE);
    GPIO_InitStructure.GPIO_Pin   = GPIO_RD1_PIN; 
    GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_IPD;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIO_RD1_PROT, &GPIO_InitStructure);
    
    /***********************************************
    * 描述： 通道数量选择：0：双通道；1：单通道
    */
    RCC_APB2PeriphClockCmd(GPIO_RD2_RCC, ENABLE);
    GPIO_InitStructure.GPIO_Pin   = GPIO_RD2_PIN; 
    GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_IPD;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIO_RD2_PROT, &GPIO_InitStructure);
}


/*
*********************************************************************************************************
*                                            BSP_StatusRd()
*
* Description : Get the current status of a status input
*
* Argument(s) : id    is the status you want to get.
*
* Return(s)   : DEF_ON    if the status is asserted
*               DEF_OFF   if the status is negated
*
* Caller(s)   : application
*
* Note(s)     : none.
*********************************************************************************************************
*/

CPU_BOOLEAN  BSP_StatusRd (CPU_INT08U  id)
{
    CPU_BOOLEAN  bit_val;

    switch (id) {
        case 1:
             bit_val = (CPU_BOOLEAN)GPIO_ReadInputDataBit(GPIO_RD0_PROT, GPIO_RD0_PIN);
             return (bit_val);

        case 2:
             bit_val = (CPU_BOOLEAN)GPIO_ReadInputDataBit(GPIO_RD1_PROT, GPIO_RD1_PIN);
             return (bit_val);

        case 3:
             bit_val = (CPU_BOOLEAN)GPIO_ReadInputDataBit(GPIO_RD2_PROT, GPIO_RD2_PIN);
             return (bit_val);
             
        default:
             return ((CPU_BOOLEAN)DEF_OFF);
    }
}


/*
*********************************************************************************************************
*********************************************************************************************************
*                                           OS PROBE FUNCTIONS
*********************************************************************************************************
*********************************************************************************************************
*/

/*
*********************************************************************************************************
*                                           OSProbe_TmrInit()
*
* Description : Select & initialize a timer for use with the uC/Probe Plug-In for uC/OS-II.
*
* Argument(s) : none.
*
* Return(s)   : none.
*
* Caller(s)   : OSProbe_Init().
*
* Note(s)     : none.
*********************************************************************************************************
*/

#if ((APP_CFG_PROBE_OS_PLUGIN_EN == DEF_ENABLED) && \
     (OS_PROBE_HOOKS_EN          == 1))
void  OSProbe_TmrInit (void)
{
}
#endif


/*
*********************************************************************************************************
*                                            OSProbe_TmrRd()
*
* Description : Read the current counts of a free running timer.
*
* Argument(s) : none.
*
* Return(s)   : The 32-bit timer counts.
*
* Caller(s)   : OSProbe_TimeGetCycles().
*
* Note(s)     : none.
*********************************************************************************************************
*/

#if ((APP_CFG_PROBE_OS_PLUGIN_EN == DEF_ENABLED) && \
     (OS_PROBE_HOOKS_EN          == 1))
CPU_INT32U  OSProbe_TmrRd (void)
{
    return ((CPU_INT32U)DWT_CYCCNT);
}
#endif


/*$PAGE*/
/*
*********************************************************************************************************
*                                          CPU_TS_TmrInit()
*
* Description : Initialize & start CPU timestamp timer.
*
* Argument(s) : none.
*
* Return(s)   : none.
*
* Caller(s)   : CPU_TS_Init().
*
*               This function is an INTERNAL CPU module function & MUST be implemented by application/
*               BSP function(s) [see Note #1] but MUST NOT be called by application function(s).
*
* Note(s)     : (1) CPU_TS_TmrInit() is an application/BSP function that MUST be defined by the developer
*                   if either of the following CPU features is enabled :
*
*                   (a) CPU timestamps
*                   (b) CPU interrupts disabled time measurements
*
*                   See 'cpu_cfg.h  CPU TIMESTAMP CONFIGURATION  Note #1'
*                     & 'cpu_cfg.h  CPU INTERRUPTS DISABLED TIME MEASUREMENT CONFIGURATION  Note #1a'.
*
*               (2) (a) Timer count values MUST be returned via word-size-configurable 'CPU_TS_TMR'
*                       data type.
*
*                       (1) If timer has more bits, truncate timer values' higher-order bits greater
*                           than the configured 'CPU_TS_TMR' timestamp timer data type word size.
*
*                       (2) Since the timer MUST NOT have less bits than the configured 'CPU_TS_TMR'
*                           timestamp timer data type word size; 'CPU_CFG_TS_TMR_SIZE' MUST be
*                           configured so that ALL bits in 'CPU_TS_TMR' data type are significant.
*
*                           In other words, if timer size is not a binary-multiple of 8-bit octets
*                           (e.g. 20-bits or even 24-bits), then the next lower, binary-multiple
*                           octet word size SHOULD be configured (e.g. to 16-bits).  However, the
*                           minimum supported word size for CPU timestamp timers is 8-bits.
*
*                       See also 'cpu_cfg.h   CPU TIMESTAMP CONFIGURATION  Note #2'
*                              & 'cpu_core.h  CPU TIMESTAMP DATA TYPES     Note #1'.
*
*                   (b) Timer SHOULD be an 'up'  counter whose values increase with each time count.
*
*                   (c) When applicable, timer period SHOULD be less than the typical measured time
*                       but MUST be less than the maximum measured time; otherwise, timer resolution
*                       inadequate to measure desired times.
*
*                   See also 'CPU_TS_TmrRd()  Note #2'.
*********************************************************************************************************
*/

#if (CPU_CFG_TS_TMR_EN == DEF_ENABLED)
void  CPU_TS_TmrInit (void)
{
    CPU_INT32U  cpu_clk_freq_hz;


    DEM_CR         |= (CPU_INT32U)DEM_CR_TRCENA;                /* Enable Cortex-M3's DWT CYCCNT reg.                   */
    DWT_CYCCNT      = (CPU_INT32U)0u;
    DWT_CR         |= (CPU_INT32U)DWT_CR_CYCCNTENA;

    cpu_clk_freq_hz = BSP_CPU_ClkFreq();
    CPU_TS_TmrFreqSet(cpu_clk_freq_hz);
}
#endif


/*$PAGE*/
/*
*********************************************************************************************************
*                                           CPU_TS_TmrRd()
*
* Description : Get current CPU timestamp timer count value.
*
* Argument(s) : none.
*
* Return(s)   : Timestamp timer count (see Notes #2a & #2b).
*
* Caller(s)   : CPU_TS_Init(),
*               CPU_TS_Get32(),
*               CPU_TS_Get64(),
*               CPU_IntDisMeasStart(),
*               CPU_IntDisMeasStop().
*
*               This function is an INTERNAL CPU module function & MUST be implemented by application/
*               BSP function(s) [see Note #1] but SHOULD NOT be called by application function(s).
*
* Note(s)     : (1) CPU_TS_TmrRd() is an application/BSP function that MUST be defined by the developer
*                   if either of the following CPU features is enabled :
*
*                   (a) CPU timestamps
*                   (b) CPU interrupts disabled time measurements
*
*                   See 'cpu_cfg.h  CPU TIMESTAMP CONFIGURATION  Note #1'
*                     & 'cpu_cfg.h  CPU INTERRUPTS DISABLED TIME MEASUREMENT CONFIGURATION  Note #1a'.
*
*               (2) (a) Timer count values MUST be returned via word-size-configurable 'CPU_TS_TMR'
*                       data type.
*
*                       (1) If timer has more bits, truncate timer values' higher-order bits greater
*                           than the configured 'CPU_TS_TMR' timestamp timer data type word size.
*
*                       (2) Since the timer MUST NOT have less bits than the configured 'CPU_TS_TMR'
*                           timestamp timer data type word size; 'CPU_CFG_TS_TMR_SIZE' MUST be
*                           configured so that ALL bits in 'CPU_TS_TMR' data type are significant.
*
*                           In other words, if timer size is not a binary-multiple of 8-bit octets
*                           (e.g. 20-bits or even 24-bits), then the next lower, binary-multiple
*                           octet word size SHOULD be configured (e.g. to 16-bits).  However, the
*                           minimum supported word size for CPU timestamp timers is 8-bits.
*
*                       See also 'cpu_cfg.h   CPU TIMESTAMP CONFIGURATION  Note #2'
*                              & 'cpu_core.h  CPU TIMESTAMP DATA TYPES     Note #1'.
*
*                   (b) Timer SHOULD be an 'up'  counter whose values increase with each time count.
*
*                       (1) If timer is a 'down' counter whose values decrease with each time count,
*                           then the returned timer value MUST be ones-complemented.
*
*                   (c) (1) When applicable, the amount of time measured by CPU timestamps is
*                           calculated by either of the following equations :
*
*                           (A) Time measured  =  Number timer counts  *  Timer period
*
*                                   where
*
*                                       Number timer counts     Number of timer counts measured
*                                       Timer period            Timer's period in some units of
*                                                                   (fractional) seconds
*                                       Time measured           Amount of time measured, in same
*                                                                   units of (fractional) seconds
*                                                                   as the Timer period
*
*                                                  Number timer counts
*                           (B) Time measured  =  ---------------------
*                                                    Timer frequency
*
*                                   where
*
*                                       Number timer counts     Number of timer counts measured
*                                       Timer frequency         Timer's frequency in some units
*                                                                   of counts per second
*                                       Time measured           Amount of time measured, in seconds
*
*                       (2) Timer period SHOULD be less than the typical measured time but MUST be less
*                           than the maximum measured time; otherwise, timer resolution inadequate to
*                           measure desired times.
*********************************************************************************************************
*/

#if (CPU_CFG_TS_TMR_EN == DEF_ENABLED)
CPU_TS_TMR  CPU_TS_TmrRd (void)
{
    return ((CPU_TS_TMR)DWT_CYCCNT);
}
#endif
