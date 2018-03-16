/*******************************************************************************
 *   Filename:       bsp_usart.c
 *   Revised:        All copyrights reserved to wumingshen.
 *   Revision:       v1.0
 *   Writer:	     wumingshen.
 *
 *   Description:    串口驱动模块
 *   Notes:          中断程序未对发送缓冲区满做处理.也就是说调用ComSendBuf()函数时，
 *		             如果开辟的发送缓冲区不足，将导致最先放入FIFO的数据被冲掉。
 *		             不过对以一般的应用程序，将发送缓冲区开的足够大以保证一个最长的通信帧能够容纳下即可。
 *     				 E-mail:shenchangwei945@163.com
 *
 *******************************************************************************/
#include "includes.h"
#include "bsp_usart.h"

#define    UCOS_EN        DEF_ENABLED


/*******************************************************************************
 * 名    称： InitHardUart
 * 功    能： 配置USART硬件参数
 * 入口参数： 无
 * 出口参数： 无
 * 作　 　者： 无名沈.
 * 创建日期： 2015-06-25
 * 修    改：
 * 修改日期：
 * 备    注：
 *******************************************************************************/
void BSP_UartCfg(u8 port_nbr,u32  baud)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	USART_InitTypeDef USART_InitStructure;

    /***********************************************
    * 描述： 串口3引脚接到地，不能使用
    */
    if ( port_nbr > 3 )
        return;
    
    switch ( port_nbr ) {
#if defined (Open_UART1)
    case 1:
	/* 第1步：打开GPIO和USART部件的时钟 */
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_AFIO, ENABLE);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1, ENABLE);

	/* 第2步：将USART Tx的GPIO配置为推挽复用模式 */
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOA, &GPIO_InitStructure);

	/* 第3步：将USART Rx的GPIO配置为浮空输入模式
    由于CPU复位后，GPIO缺省都是浮空输入模式，因此下面这个步骤不是必须的
    但是，我还是建议加上便于阅读，并且防止其它地方修改了这个口线的设置参数
	*/
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
	GPIO_Init(GPIOA, &GPIO_InitStructure);
	/*  第3步已经做了，因此这步可以不做
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	*/
	GPIO_Init(GPIOA, &GPIO_InitStructure);

	/* 第4步：设置串口硬件参数 */
	USART_InitStructure.USART_BaudRate = baud;
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;
	USART_InitStructure.USART_StopBits = USART_StopBits_1;
	USART_InitStructure.USART_Parity = USART_Parity_No ;
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
	USART_Init(USART1, &USART_InitStructure);
	USART_ITConfig(USART1, USART_IT_RXNE, ENABLE);	/* 使能接收中断 */
	
    //USART_ITConfig(USART1, USART_IT_TXE, ENABLE);
    /*注意: 不要在此处打开发送中断
    发送中断使能在SendUart()函数打开
	*/
	USART_Cmd(USART1, ENABLE);		/* 使能串口 */

	/***********************************************
    * 描述： CPU的小缺陷：串口配置好，如果直接Send，则第1个字节发送不出去
	*	     如下语句解决第1个字节无法正确发送出去的问题：
	*	     清发送完成标志，Transmission Complete flag
    */
	USART_ClearFlag(USART1, USART_FLAG_TC);     /* 清发送标志，Transmission Complete flag */

#if  (UCOS_EN == DEF_ENABLED)
    /***********************************************
    * 描述：中断函数映射处理，将USART1的中断服务函数入口地址映射到
    *       USARTx_RxTxISRHandler的函数入口地址上，并使能。
    */
    BSP_IntVectSet(BSP_INT_ID_USART1, USART1_IRQHandler);
    BSP_IntEn(BSP_INT_ID_USART1);

#else
    NVIC_InitTypeDef NVIC_InitStructure;
    /* Configure the NVIC Preemption Priority Bits */
    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_0);
    /* Enable the USART1 Interrupt */
    NVIC_InitStructure.NVIC_IRQChannel = USART1_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);

    /*在不使用UCOS系统时，将USART1_IRQHandler函数放在stm32_it.c文件的串口中断函数中*/
#endif

        break;
#endif /*end of open_uart1*/

#if defined (Open_UART2)
    case 2:
	/* 第1步：打开GPIO和USART部件的时钟 */
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_AFIO, ENABLE);
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART2, ENABLE);

	/* 第2步：将USART Tx的GPIO配置为推挽复用模式 */
	GPIO_InitStructure.GPIO_Pin   = GPIO_Pin_2;
	GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_AF_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOA, &GPIO_InitStructure);

	/* 第3步：将USART Rx的GPIO配置为浮空输入模式
		由于CPU复位后，GPIO缺省都是浮空输入模式，因此下面这个步骤不是必须的
		但是，我还是建议加上便于阅读，并且防止其它地方修改了这个口线的设置参数
	*/
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_3;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
	GPIO_Init(GPIOA, &GPIO_InitStructure);
	/*  第3步已经做了，因此这步可以不做
		GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	*/
	GPIO_Init(GPIOA, &GPIO_InitStructure);

	/* 第4步：设置串口硬件参数 */
	USART_InitStructure.USART_BaudRate = baud;
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;
	USART_InitStructure.USART_StopBits = USART_StopBits_1;
	USART_InitStructure.USART_Parity = USART_Parity_No ;
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
	USART_Init(USART2, &USART_InitStructure);
	USART_ITConfig(USART2, USART_IT_RXNE, ENABLE);	/* 使能接收中断 */
	/*
    USART_ITConfig(USART2, USART_IT_TXE, ENABLE);
    注意: 不要在此处打开发送中断
    发送中断使能在SendUart()函数打开
	*/
	USART_Cmd(USART2, ENABLE);		/* 使能串口 */

	/***********************************************
    * 描述： CPU的小缺陷：串口配置好，如果直接Send，则第1个字节发送不出去
	*	     如下语句解决第1个字节无法正确发送出去的问题：
	*	     清发送完成标志，Transmission Complete flag
    */
	USART_ClearFlag(USART2, USART_FLAG_TC);     /* 清发送标志，Transmission Complete flag */

#if  (UCOS_EN == DEF_ENABLED)
    /***********************************************
    * 描述：中断函数映射处理，将USART1的中断服务函数入口地址映射到
    *       USARTx_RxTxISRHandler的函数入口地址上，并使能。
    */
    BSP_IntVectSet(BSP_INT_ID_USART2, USART2_IRQHandler);
    BSP_IntEn(BSP_INT_ID_USART2);

#else
    NVIC_InitTypeDef NVIC_InitStructure;
    /* Configure the NVIC Preemption Priority Bits */
    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_0);
    /* Enable the USART1 Interrupt */
    NVIC_InitStructure.NVIC_IRQChannel = USART2_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);

    /*在不使用UCOS系统时，将USART2_IRQHandler函数放在stm32_it.c文件的串口中断函数中*/
#endif

        break;
#endif /*end of open_uart2*/
#if defined (Open_UART3)

    case 3:
	/* 第1步：打开GPIO和USART部件的时钟 */
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB | RCC_APB2Periph_AFIO, ENABLE);
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART3, ENABLE);

	/* 第2步：将USART Tx的GPIO配置为推挽复用模式 */
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;
	GPIO_InitStructure.GPIO_Mode =  GPIO_Mode_AF_PP;//GPIO_Mode_AF_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOB, &GPIO_InitStructure);

	/* 第3步：将USART Rx的GPIO配置为浮空输入模式
		由于CPU复位后，GPIO缺省都是浮空输入模式，因此下面这个步骤不是必须的
		但是，我还是建议加上便于阅读，并且防止其它地方修改了这个口线的设置参数
	*/
	GPIO_InitStructure.GPIO_Pin  = GPIO_Pin_11;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
	GPIO_Init(GPIOB, &GPIO_InitStructure);
	/*  第3步已经做了，因此这步可以不做
		GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	*/
	GPIO_Init(GPIOB, &GPIO_InitStructure);
    //    return ;
	/* 第4步：设置串口硬件参数 */
	USART_InitStructure.USART_BaudRate = baud;
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;
	USART_InitStructure.USART_StopBits = USART_StopBits_1;
	USART_InitStructure.USART_Parity = USART_Parity_No ;
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
	USART_Init(USART3, &USART_InitStructure);
	USART_ITConfig(USART3, USART_IT_RXNE, ENABLE);	/* 使能接收中断 */
	/*
    USART_ITConfig(USART3, USART_IT_TXE, ENABLE);
    注意: 不要在此处打开发送中断
    发送中断使能在SendUart()函数打开
	*/
	USART_Cmd(USART3, ENABLE);		/* 使能串口 */

	/***********************************************
    * 描述： CPU的小缺陷：串口配置好，如果直接Send，则第1个字节发送不出去
	*	     如下语句解决第1个字节无法正确发送出去的问题：
	*	     清发送完成标志，Transmission Complete flag
    */
	USART_ClearFlag(USART3, USART_FLAG_TC);     /* 清发送标志，Transmission Complete flag */

#if  (UCOS_EN == DEF_ENABLED)
    /***********************************************
    * 描述：中断函数映射处理，将USART1的中断服务函数入口地址映射到
    *       USARTx_RxTxISRHandler的函数入口地址上，并使能。
    */
    BSP_IntVectSet(BSP_INT_ID_USART3, USART3_IRQHandler);
    BSP_IntEn(BSP_INT_ID_USART3);

#else
    NVIC_InitTypeDef NVIC_InitStructure;
    /* Configure the NVIC Preemption Priority Bits */
    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_0);
    /* Enable the USART1 Interrupt */
    NVIC_InitStructure.NVIC_IRQChannel = USART3_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);

    /*在不使用UCOS系统时，将USART3_IRQHandler函数放在stm32_it.c文件的串口中断函数中*/
#endif

    break;
#endif /*end of open_uart3*/
    }
}

/*******************************************************************************
 * 名    称： UartIRQ
 * 功    能： 供中断服务程序调用，通用串口中断处理函数。
 * 入口参数： _pUart: 串口结构体指针
 * 出口参数： 无
 * 作　 　者： 无名沈.
 * 创建日期： 2015-06-25
 * 修    改：
 * 修改日期：
 * 备    注：
 *******************************************************************************/
#if defined(PM2D5_UART_CHK)
u8  Uart1RxBuf[11];
u8  Uart1RxCtr  = 0;
#endif
static void  USARTx_RxTxISRHandler (USART_TypeDef* USARTx)
{
    uint8_t t_receivedata = 0;
    /***********************************************
    * 描述：接收中断
    */
    if (SET == USART_GetFlagStatus(USARTx, USART_FLAG_RXNE)) {
        /***********************************************
        * 描述：接收一个字节
        */    
#if defined(PM2D5_UART_CHK)
        Uart1RxBuf[Uart1RxCtr++]   = USART_ReceiveData(USARTx);
        if ( Uart1RxCtr > 10 )
            Uart1RxCtr = 0;
        
        osal_start_timerEx( OS_TASK_ID_SENSOR,
                           OS_EVT_SEN_UART_TIMEOUT,
                            20);
#else
        t_receivedata   = USART_ReceiveData(USARTx);
        t_receivedata   = t_receivedata; 
#endif
        USART_ClearITPendingBit(USARTx, USART_IT_RXNE);         /* Clear the USARTx receive interrupt.                  */
    }

    /***********************************************
    * 描述：发送中断
    */
    if (SET == USART_GetFlagStatus(USARTx, USART_FLAG_TC)) {


        USART_ClearITPendingBit(USARTx, USART_IT_TC);           /*清除串口发送中断标志位*/
        USART_ITConfig(USARTx, USART_IT_TC, DISABLE);           /*关闭串口发送中断*/
    }
}

/*******************************************************************************
 * 名    称： USART1_IRQHandler
 * 功    能： USART1中断服务程序
 * 入口参数： 无
 * 出口参数： 无
 * 作　 　者： 无名沈.
 * 创建日期： 2015-06-25
 * 修    改：
 * 修改日期：
 * 备    注：
 *******************************************************************************/
void USART1_IRQHandler(void)
{
	USARTx_RxTxISRHandler (USART1);
}

/*******************************************************************************
 * 名    称： USART2_IRQHandler
 * 功    能： USART2中断服务程序
 * 入口参数： 无
 * 出口参数： 无
 * 作　 　者： 无名沈.
 * 创建日期： 2015-06-25
 * 修    改：
 * 修改日期：
 * 备    注：
 *******************************************************************************/
void USART2_IRQHandler(void)
{
	USARTx_RxTxISRHandler (USART2);
//     static u16   recvcont = 0;

//    /***********************************************
//    * 描述：接收中断
//    */
//    if (SET == USART_GetFlagStatus(USART2, USART_FLAG_RXNE)) {
//         extern u8 PM2D5_UartBuf[];
//         PM2D5_UartBuf[recvcont] = USART_ReceiveData(USART2);
//         if(++recvcont >= 14) {
//            recvcont = 0;
//            Ctrl.PM2D5.RxFaildCtr   = 0;
//         }
//         USART_ClearITPendingBit(USART2, USART_IT_RXNE);                /* Clear the USART2 receive interrupt.                  */
//     }
//
//    /***********************************************
//    * 描述：发送中断
//    */
//    if (SET == USART_GetFlagStatus(USART2, USART_FLAG_TC)) {
//        USART_ITConfig(USART2, USART_IT_TC, DISABLE);
//        USART_ClearITPendingBit(USART2, USART_IT_TC);                   /* Clear the USART2 receive interrupt.                */
//    }
}

/*******************************************************************************
 * 名    称： USART3_IRQHandler
 * 功    能： USART3中断服务程序
 * 入口参数： 无
 * 出口参数： 无
 * 作　 　者： 无名沈.
 * 创建日期： 2015-06-25
 * 修    改：
 * 修改日期：
 * 备    注：
 *******************************************************************************/
//extern UART_HandleTypeDef  				Ctrl.Wifi.UartHandle;
//extern Device_ReadTypeDef               Device_ReadStruct;
void USART3_IRQHandler(void)
{
	USARTx_RxTxISRHandler (USART3);
//    uint8_t 	value;
//    OS_ERR      err;
//    
//    if(USART_GetITStatus(USART3, USART_IT_RXNE) != RESET) {
//		USART_ClearITPendingBit(USART3,USART_IT_RXNE);
//		value = USART_ReceiveData(USART3);
//        /***********************************************
//        * 描述： 发送WIFI串口接收超时标志位
//        */
//        //OSFlagPost( ( OS_FLAG_GRP  *)&Ctrl.Os.WifiEvtFlagGrp,
//        //            ( OS_FLAGS      )WIFI_EVT_FLAG_TIMEOUT,
//        //            ( OS_OPT        )OS_OPT_POST_FLAG_SET,
//        //            ( OS_ERR       *)&err); 
//        Ctrl.Wifi.UartHandle.UART_RecvFlag         = 1;
//        Ctrl.Wifi.UartHandle.UART_RecvTimeoutCtr   = 0;
//        /***********************************************
//        * 描述： 准备收取一包数据
//        */
//		if(Ctrl.Wifi.UartHandle.Package_Flag != 0 ) {
//            if ( Ctrl.Wifi.UartHandle.UART_Count >= Max_UartBuf ){
//                Ctrl.Wifi.UartHandle.UART_Count    = 0;
//                Ctrl.Wifi.UartHandle.Package_Flag  = 0;
//            }
//		} else {
//            /***********************************************
//            * 描述： 判断第一个字节是不是数据帧头0XFF
//            */
//			if(Ctrl.Wifi.UartHandle.UART_Flag1 ==0) {
//				if(value == 0xff) {
//                    Ctrl.Wifi.UartHandle.UART_Count = 0;
//                    Ctrl.Wifi.UartHandle.UART_Buf[Ctrl.Wifi.UartHandle.UART_Count]=value;
//                    Ctrl.Wifi.UartHandle.UART_Count++;
//                    Ctrl.Wifi.UartHandle.UART_Flag1 = 1;
//				}
//				return ;
//                /***********************************************
//                * 描述： 判断第二个字节是不是0XFF
//                */
//			} else if(Ctrl.Wifi.UartHandle.UART_Flag2 ==0) {
//                Ctrl.Wifi.UartHandle.UART_Buf[Ctrl.Wifi.UartHandle.UART_Count]=value;
//                Ctrl.Wifi.UartHandle.UART_Count++;
//                /***********************************************
//                * 描述： 第二个字节是0XFF，接受下一个数据
//                */
//                if(Ctrl.Wifi.UartHandle.UART_Buf[1] == 0xff) {
//                    Ctrl.Wifi.UartHandle.UART_Flag2 = 1;
//                    /***********************************************
//                    * 描述： 第二个字节不是0XFF重新开始接受数据
//                    */
//                } else {
//                    Ctrl.Wifi.UartHandle.UART_Flag1 = 0;
//                }
//                return ;
//                /***********************************************
//                * 描述： 当第一二个字节正确时，接受之后的字节
//                */
//			} else {
//                /***********************************************
//                * 描述：往串口接受缓冲区存放接受数据
//                */
//				Ctrl.Wifi.UartHandle.UART_Buf[Ctrl.Wifi.UartHandle.UART_Count] = value;
//                /***********************************************
//                * 描述：往串口接受缓冲区存放接受数据
//                */
//				if(Ctrl.Wifi.UartHandle.UART_Count >=4 && Ctrl.Wifi.UartHandle.UART_Buf[Ctrl.Wifi.UartHandle.UART_Count] == 0x55 && \
//                    Ctrl.Wifi.UartHandle.UART_Buf[Ctrl.Wifi.UartHandle.UART_Count - 1] == 0xFF){
//                        //待处理
//                    } else {
//                        Ctrl.Wifi.UartHandle.UART_Count++;
//                    }
//                
//				if(Ctrl.Wifi.UartHandle.UART_Count == 0x04) {
//					Ctrl.Wifi.UartHandle.UART_Cmd_len = Ctrl.Wifi.UartHandle.UART_Buf[2]*256+  Ctrl.Wifi.UartHandle.UART_Buf[3];
//                    //============================================//
//                    /***********************************************
//                    * 描述：该段代码自行加入,收到的长度错误时，重新接受数据
//                    */
//                    if((Ctrl.Wifi.UartHandle.UART_Cmd_len >= 255) || (Ctrl.Wifi.UartHandle.UART_Cmd_len  == 0)) {
//                        Ctrl.Wifi.UartHandle.UART_Cmd_len = 0;
//                        Ctrl.Wifi.UartHandle.UART_Flag1 = 0;
//                        Ctrl.Wifi.UartHandle.UART_Flag2 = 0;
//                        return;
//                    }
//                    //============================================//
//				}
//				if(Ctrl.Wifi.UartHandle.UART_Count ==  (Ctrl.Wifi.UartHandle.UART_Cmd_len + 4)) {
//					//OSSemPost ((OS_SEM  *)&SemUsartPackageFlag,
//                    //           (OS_OPT   )OS_OPT_POST_1,
//                    //           (OS_ERR  *)&err);
//                    
//                    /***********************************************
//                    * 描述： 发送WIFI串口接收标志位
//                    */
//                    OSFlagPost( ( OS_FLAG_GRP  *)&Ctrl.Os.WifiEvtFlagGrp,
//                               ( OS_FLAGS      )WIFI_EVT_FLAG_RECV,
//                               ( OS_OPT        )OS_OPT_POST_FLAG_SET,
//                               ( OS_ERR       *)&err); 
//                    /***********************************************
//                    * 描述： 发送WIFI串口接收超时标志位
//                    */
//                    //OSFlagPost( ( OS_FLAG_GRP  *)&Ctrl.Os.WifiEvtFlagGrp,
//                    //            ( OS_FLAGS      )WIFI_EVT_FLAG_TIMEOUT,
//                    //            ( OS_OPT        )OS_OPT_POST_FLAG_CLR,
//                    //            ( OS_ERR       *)&err); 
//                    
//                    
//                    Ctrl.Wifi.UartHandle.UART_RecvFlag = 0;
//                    Ctrl.Wifi.UartHandle.Package_Flag  = 1;
//					Ctrl.Wifi.UartHandle.UART_Flag1    = 0;
//					Ctrl.Wifi.UartHandle.UART_Flag2    = 0;
//                    //printf("Package_Flag =1 \r\n");
//				}
//			}
//		}
//    }
}

/*******************************************************************************
 * 名    称： USART_OPen_INT
 * 功    能： 打开某个串口的中断
 * 入口参数： USART_TypeDef* USARTx 串口号
 * 出口参数： 无
 * 作　 　者： 无名沈.
 * 创建日期： 2015-06-25
 * 修    改：
 * 修改日期：
 * 备    注：
 *******************************************************************************/
void USART_OPen_INT(USART_TypeDef* USARTx)
{
    USART_ClearITPendingBit(USARTx,USART_IT_RXNE);
    USART_ITConfig(USARTx, USART_IT_RXNE, ENABLE);

}

/*******************************************************************************
 * 名    称： USART_Close_INT
 * 功    能： 关闭某个串口的中断
 * 入口参数： USART_TypeDef* USARTx 串口号
 * 出口参数： 无
 * 作　 　者： 无名沈.
 * 创建日期： 2015-06-25
 * 修    改：
 * 修改日期：
 * 备    注：
 *******************************************************************************/
void USART_Close_INT(USART_TypeDef* USARTx)
{
    USART_ITConfig(USARTx, USART_IT_RXNE, DISABLE);

}

/*******************************************************************************
 * 名    称： USART_Send_Char
 * 功    能： 查询法发送一个字节
 * 入口参数： USART_TypeDef* USARTx 串口号 ；data ：需要发送的字节
 * 出口参数： 无
 * 作　 　者： 无名沈.
 * 创建日期： 2015-06-25
 * 修    改：
 * 修改日期：
 * 备    注：
 *******************************************************************************/
void USART_Send_Char(USART_TypeDef* USARTx,unsigned char data)
{
    USART_ClearFlag(USARTx,USART_FLAG_TC);
    USART_SendData(USARTx,data);
    while(USART_GetFlagStatus(USARTx, USART_FLAG_TXE) == RESET);     //等待发送完毕
}

/*******************************************************************************
 * 名    称： USART_Send_String
 * 功    能： 查询法发送一串字符串
 * 入口参数： USART_TypeDef* USARTx 串口号 ；unsigned char *buffer：需要发送的字符串
 *            unsigned int len ： 字符串长度
 * 出口参数： 无
 * 作　 　者： 无名沈.
 * 创建日期： 2015-06-25
 * 修    改：
 * 修改日期：
 * 备    注：
 *******************************************************************************/
void USART_WriteBytes(USART_TypeDef* USARTx,unsigned char *buffer,unsigned int len)
{
    if ( len ) {
        while(len--) {
            USART_Send_Char(USARTx,*buffer);
            buffer++;
        }
    } else {        
        while(*buffer) {
            USART_Send_Char(USARTx,*buffer);
            buffer++;
        }
    }
}

