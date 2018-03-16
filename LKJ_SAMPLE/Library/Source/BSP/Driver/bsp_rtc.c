/*******************************************************************************
 *   Filename:      bsp_rtc.c
 *   Revised:       $Date: 2013-06-30
 *   Revision:      $V1.0
 *   Writer:        wumingshen.
 *
 *   Description:
 *
 *   Notes:         STM32F103内部RTC驱动模块
 *                  使用RTC可以直接调用bsp_time.c中的函数获取和修改RTC实时时间
 *
 *   All copyrights reserved to wumingshen
 *
 *******************************************************************************/

/*******************************************************************************
 * INCLUDES
 */
#include <bsp_rtc.h>

#define BSP_RTC_MODULE_EN  1
#if BSP_RTC_MODULE_EN >  0

/*******************************************************************************
 * 描述： 该模块是否搭载uCOS系统使用
 */
#if defined( UCOS_EN )
#undef UCOS_EN
#endif
     
#define UCOS_EN  1

/*******************************************************************************
 * 描述： 允许时钟64分频到PC13脚输出时钟
 */
#define RTCClockOutput_Enable  /* RTC Clock/64 is output on tamper pin(PC.13) */

/*******************************************************************************
 * TYPEDEFS
 */

#if (UCOS_EN == ENABLE)
    #if OS_VERSION > 30000U
    static  OS_SEM                   RtcSem;    // 信号量
    #else
    static  OS_EVENT                *RtcSem;    // 信号量
    #endif
#endif

//============================================================================//

/*******************************************************************************
* 名    称： BSP_RTC_Alarm_ISR_Handler
* 功    能： RTC报警中断服务函数
* 入口参数： 无
* 出口参数： 无
* 作　 　者： 无名沈
* 创建日期： 2014-08-18
* 修    改：
* 修改日期：
* 备    注：
*******************************************************************************/
void BSP_RTC_Alarm_ISR_Handler (void)
{
//	if(RTC_GetITStatus(RTC_IT_ALR) != RESET) {
//        EXTI_ClearITPendingBit(EXTI_Line17);		// 清EXTI_Line17挂起位
//        if(PWR_GetFlagStatus(PWR_FLAG_WU) != RESET){// 检查唤醒标志是否设置
//            PWR_ClearFlag(PWR_FLAG_WU);				// 清除唤醒标志
//        }
//        RTC_WaitForLastTask();						// 等待最后一条命令写完成
//
//        RTC_ClearITPendingBit(RTC_IT_ALR);			// 清RTC报警中断挂起
//        RTC_WaitForLastTask();						// 等待最后一条命令写完成
//    }
}

/*******************************************************************************
* 名    称： RTC_Configuration
* 功    能： RTC配置函数
* 入口参数： 无
* 出口参数： 无
* 作　 　者： 无名沈
* 创建日期： 2014-08-18
* 修    改：
* 修改日期：
* 备    注：
*******************************************************************************/
void RTC_Configuration(void)
{
    /* Enable PWR and BKP clocks */
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_PWR | RCC_APB1Periph_BKP, ENABLE);

    /* Allow access to BKP Domain */
    PWR_BackupAccessCmd(ENABLE);

    /* Reset Backup Domain */
    BKP_DeInit();

    /* Enable LSE */
    RCC_LSEConfig(RCC_LSE_ON);
    /* Wait till LSE is ready */
    while (RCC_GetFlagStatus(RCC_FLAG_LSERDY) == RESET){}

    /* Select LSE as RTC Clock Source */
    RCC_RTCCLKConfig(RCC_RTCCLKSource_LSE);
    //RCC_RTCCLKConfig(RCC_RTCCLKSource_LSI);

    /* Enable RTC Clock */
    RCC_RTCCLKCmd(ENABLE);

    /* Wait for RTC registers synchronization */
    RTC_WaitForSynchro();

    /* Wait until last write operation on RTC registers has finished */
    RTC_WaitForLastTask();

    /* Enable the RTC Second */
	//RTC_ITConfig(RTC_IT_SEC, ENABLE);

	/* Wait until last write operation on RTC registers has finished */
    //RTC_WaitForLastTask();

    /* Set RTC prescaler: set RTC period to 1sec */
    RTC_SetPrescaler(32768); /* RTC period = RTCCLK/RTC_PR = (32.768 KHz)/(32767+1) */

    /* Wait until last write operation on RTC registers has finished */
    RTC_WaitForLastTask();
}


/*******************************************************************************
* 名    称： BSP_RTC_GetTime
* 功    能： RTC获取实时时间
* 入口参数： INT8U *time ：大小为3个字节的数组；time[0]:时 time[1]:分 time[2]:秒
* 出口参数： TRUE ： 时间获取成功
* 作　 　者： 无名沈
* 创建日期： 2014-08-18
* 修    改：
* 修改日期：
* 备    注：
*******************************************************************************/
BOOL BSP_RTC_GetTime( INT8U *time )
{
    INT32U TimeCnt;

#if (UCOS_EN == ENABLE)
#if OS_VERSION > 30000U
    CPU_BOOLEAN err;
    err = BSP_OS_SemWait(&RtcSem,1000);             // 等待信号量
    if ( err != OS_ERR_NONE )
        return FALSE;
#else
    INT8U   err;
    OSSemPend(RtcSem,1000,&err);                    // 信号量
    if ( err != OS_ERR_NONE )
        return FALSE;
#endif
#endif
    /* Display current time */
    TimeCnt = RTC_GetCounter();

    /* Compute  hours */
    *time++ = TimeCnt / 3600 % 24;
    /* Compute minutes */
    *time++ = (TimeCnt % 3600) / 60;
    /* Compute seconds */
    *time++ = (TimeCnt % 3600) % 60;

#if (UCOS_EN == ENABLE)

#if OS_VERSION > 30000U
    BSP_OS_SemPost(&RtcSem);                     // 发送信号量
#else
    OSSemPost(RtcSem);                              // 信号量
#endif

#endif

    return TRUE;
}


/*******************************************************************************
* 名    称： BSP_RTC_SetTime
* 功    能： 设置RTC的时间
* 入口参数： INT8U *time ：大小为3个字节的数组；time[0]:时 time[1]:分 time[2]:秒
* 出口参数： 无
* 作　 　者： 无名沈
* 创建日期： 2014-08-18
* 修    改：
* 修改日期：
* 备    注：
*******************************************************************************/
BOOL BSP_RTC_SetTime(INT8U *time)
{
    INT32U  cnts;

#if (UCOS_EN == ENABLE)
    #if OS_VERSION > 30000U
         CPU_BOOLEAN err;
         err = BSP_OS_SemWait(&RtcSem,1000);           // 等待信号量
         if ( err != OS_ERR_NONE )
          return FALSE;
    #else
        INT08U       err;
        OSSemPend(RtcSem,1000,&err);                   // 等待信号量
        if ( err != OS_ERR_NONE )
          return FALSE;
    #endif
#endif


  /* Wait until last write operation on RTC registers has finished */
  RTC_WaitForLastTask();

  cnts  = time[0] * 3600 + time[1] * 60 + time[2];

  /* Change the current time */
  RTC_SetCounter(cnts);
  /* Wait until last write operation on RTC registers has finished */
  RTC_WaitForLastTask();

#if (UCOS_EN == ENABLE)
#if OS_VERSION > 30000U
    BSP_OS_SemPost(&RtcSem);                        // 发送信号量
#else
    INT08U       err;
    OSSemPost(RtcSem);                              // 发送信号量
#endif
#endif
  return TRUE;
}


/*******************************************************************************
* 名    称： BSP_RTC_DispTime
* 功    能： RTC 实时时间显示
* 入口参数： 无
* 出口参数： 无
* 作　 　者： 无名沈
* 创建日期： 2014-08-18
* 修    改：
* 修改日期：
* 备    注：
*******************************************************************************/
void BSP_RTC_DispTime(void)
{
    INT8U TimeBuf[3];

	BSP_RTC_GetTime(TimeBuf);

//    GUI_SetDrawMode(GUI_DM_XOR);
//    GUI_SetFont(&GUI_FontComic18B_ASCII);
//
//    GUI_DispDecAt   (TimeBuf[0],    LCD_XSIZE - 20 * 8, LCD_YSIZE - 20, 2);
//    GUI_DispStringAt(":",           LCD_XSIZE - 20 * 6, LCD_YSIZE - 20);
//    GUI_DispDecAt   (TimeBuf[1],    LCD_XSIZE - 20 * 5, LCD_YSIZE - 20, 2);
//    GUI_DispStringAt(":",           LCD_XSIZE - 20 * 3, LCD_YSIZE - 20);
//    GUI_DispDecAt   (TimeBuf[2],    LCD_XSIZE - 20 * 2, LCD_YSIZE - 20, 2);
    //UartPrintf(COMM_PORT_MSG, "当前时间: %0.2d:%0.2d:%0.2d\r\n", TimeBuf[0], TimeBuf[1], TimeBuf[2]);
}


/*******************************************************************************
* 名    称： BSP_RTC_Init
* 功    能： RTC初始化函数
* 入口参数： 无
* 出口参数： TRUE : 初始化成功
* 作　 　者： 无名沈
* 创建日期： 2014-08-18
* 修    改：
* 修改日期：
* 备    注：
*******************************************************************************/
BOOL BSP_RTC_Init(void)
{
    if (BKP_ReadBackupRegister(BKP_DR1) != 0xA5A5) {
        INT8U time[] = {20, 4, 0};
        // Backup data register value is not correct or not yet programmed (when
        // the first time the program is executed)

        //UartPutString(COMM_PORT_MSG, "时钟未配置...\r\n");

        // RTC Configuration
        RTC_Configuration();

        //UartPutString(COMM_PORT_MSG, "时钟已配置...\r\n");

        // Adjust time by values entred by the user on the hyperterminal
        BSP_RTC_SetTime(time);

        BKP_WriteBackupRegister(BKP_DR1, 0xA5A5);
    } else {
        // Check if the Power On Reset flag is set
        if (RCC_GetFlagStatus(RCC_FLAG_PORRST) != RESET) {
          //UartPutString(COMM_PORT_MSG, "上电复位...\r\n");
        }
        // Check if the Pin Reset flag is set
        else if (RCC_GetFlagStatus(RCC_FLAG_PINRST) != RESET) {
          //UartPutString(COMM_PORT_MSG, "外部复位...\r\n");
        }

        //UartPutString(COMM_PORT_MSG, "时钟无需配置...\r\n");
        // Wait for RTC registers synchronization
        RTC_WaitForSynchro();

        // Enable the RTC Second
        //RTC_ITConfig(RTC_IT_SEC, ENABLE);
        // Wait until last write operation on RTC registers has finished
        //RTC_WaitForLastTask();
    }

#ifdef RTCClockOutput_Enable
    // Enable PWR and BKP clocks
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_PWR | RCC_APB1Periph_BKP, ENABLE);

    // Allow access to BKP Domain
    PWR_BackupAccessCmd(ENABLE);

    // Disable the Tamper Pin
    BKP_TamperPinCmd(DISABLE); // To output RTCCLK/64 on Tamper pin, the tamper
                               //  functionality must be disabled

    // Enable RTC Clock Output on Tamper Pin */
    // BKP_RTCOutputConfig(BKP_RTCOutputSource_CalibClock);
#endif

	/***********************************************
	* 描述：设置中断入口函数及中断优先级
	*/
	BSP_IntVectSet(BSP_INT_ID_RTC, BSP_RTC_Alarm_ISR_Handler);
	BSP_IntEn(BSP_INT_ID_RTC);
	/***********************************************
	* 描述：允许RTC报警中断
	*/
	RTC_ITConfig(RTC_IT_ALR, ENABLE);				// 允许RTC报警中断
	RTC_WaitForLastTask();							// 等待最后一条写指令完成

    // Clear reset flags
    RCC_ClearFlag();

#if (UCOS_EN == ENABLE)
#if OS_VERSION > 30000U
    BSP_OS_SemCreate(&RtcSem,1, "Bsp RtcSem");      // 创建信号量
#else
    RtcSem     = OSSemCreate(1);                    // 创建信号量
#endif
#endif
    return TRUE;
}


/*******************************************************************************
* 名    称： RTC_Configuration
* 功    能： RTC配置函数
* 入口参数： 无
* 出口参数： 无
* 作　 　者： 无名沈
* 创建日期： 2014-08-18
* 修    改：
* 修改日期：
* 备    注：
*******************************************************************************/
//void BSP_RTC_Main(void)
//{
//    BSP_RTC_Init();
//
//    while(1){
//        BSP_RTC_DispTime();
//        OSTimeDly(OS_TICKS_PER_SEC);
//        break;
//    }
//}

/*******************************************************************************
 *              end of file                                                    *
 *******************************************************************************/
#endif