#include    "stm32f10x.h"
#include	"I2C_CLK.h"
#include    <stdio.h>
#include    "DELAY.h"
#include	"FM24CL64.h"


#include  <global.h>
#include  <includes.h>


//铁电参数
#define		FRAM_SIZE               8192						         	//1024*8空间容量
#define		FRAM_START_ADDR	        0x0000	                                //铁电的起始地址
#define		FRAM_DIVICE_ADDR        0xa0	                                //铁电的设备地址
#define		FRAM_END_ADDR	      	(FRAM_START_ADDR + FRAM_SIZE)	

//#define		FRMA_PROTECT			P121
//
////------------------------------------------------------------------------
////  名  称 ：void WriteSCL( uint8 temp )
////  功  能 ：设置 SCL
//// 入口参数：无
//// 出口参数：无
////------------------------------------------------------------------------
//void WriteFRAM_PROTECT(uint8 temp)
//{
//    IO1DIR_OUT(FRMA_PROTECT);    
//    IO1PIN_W(temp,FRMA_PROTECT);
//}

#define  UCOS_EN            DEF_ENABLED

/***********************************************
* 描述： OS接口
*/
#if UCOS_EN     == DEF_ENABLED
    #if OS_VERSION > 30000U
    static  OS_SEM                   Bsp_EepSem;    // 信号量
    #else
    static  OS_EVENT                *Bsp_EepSem;    // 信号量
    #endif
#endif
  
    
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
    return BSP_OS_SemWait(&Bsp_EepSem,0);           // 等待信号量
#else
    uint8_t       err;
    OSSemPend(Bsp_EepSem,0,&err);                   // 等待信号量
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
    BSP_OS_SemPost(&Bsp_EepSem);                        // 发送信号量
#else
    uint8_t       err;
    OSSemPost(Bsp_EepSem);                              // 发送信号量
#endif
}

void EEP_OS_Init(void)
{
    /***********************************************
    * 描述： OS接口
    */
#if (UCOS_EN     == DEF_ENABLED)
#if OS_VERSION > 30000U
    BSP_OS_SemCreate(&Bsp_EepSem,1, "Bsp EepSem");      // 创建信号量
#else
    Bsp_EepSem     = OSSemCreate(1);                    // 创建信号量
#endif
#endif
}
//-------------------------------------------------------------------------------------------------------
//函数名称:         WriteFM24CL16()
//功    能:         对FM24CL16指定地址进行写数据
//入口参数:         ADDR:       操作地址    地址：0 ~ sizeof(FM24CL16)
//                  *DataBuf:   数据缓冲
//                  Len:        数据长度
//出口参数:         无
//说明：            
//--------------------------------------------------------------------------------------------------------
uint8_t WriteFM24CL64(uint16_t  Addr, uint8_t *DataBuf, uint16_t DataBufLen)            
{
    uint32_t  i = 0;
    uint8_t   SlaveAddr;                                      //从机地址
    uint16_t  AddrTemp = 0;                                   //地址缓存
    
//    WriteFRAM_PROTECT(0);									//关写保护
    
    EEP_WaitEvent();                                        // 等信号量
    
    AddrTemp = Addr;
    AddrTemp += FRAM_START_ADDR;                            //计算地址

    if( (uint16_t)AddrTemp + DataBufLen > FRAM_END_ADDR ) 
    {
        EEP_SendEvent();                                    // 释放信号量
        return  0;                                          //地址未超阶
    }
        
    SlaveAddr = FRAM_DIVICE_ADDR & (~(0x01));               //求FM24CL64设备写地址
    
    I2C_Start();                                             //启动IIC总线
    
    I2C_SendByte(SlaveAddr);                            //写FM24CL64地址
	while(I2C_WaitAck());			  	   //等待从设备应答
 
    I2C_SendByte(Addr >> 8);                            //写FM24CL64高位地址
	while(I2C_WaitAck());			  	   //等待从设备应答
    
    I2C_SendByte( Addr );                               //写FM24CL64低位地址
	while(I2C_WaitAck());			  	   //等待从设备应答     
  
    for(i = 0; i < DataBufLen; i++)                         //发送数据
    {
        I2C_SendByte(DataBuf[i]);                       //写数据
        while(I2C_WaitAck());			  	   //等待从设备应答
    }
    
    I2C_Stop();                                              //停止IIC操作   
    
    EEP_SendEvent();                                    // 释放信号量
    return  1;                                           //正确
}

//-------------------------------------------------------------------------------------------------------
//函数名称:         ReadFM24CL16()
//功    能:         读取FM24CL16指定地址进行数据
//入口参数:         ADDR:       操作地址   地址：0 ~ sizeof(FM24CL16)
//                  *DataBuf:   数据缓冲 
//                  Len:        数据长度
//出口参数:         无
//说明：            
//--------------------------------------------------------------------------------------------------------
uint8_t ReadFM24CL64(uint16_t  Addr, uint8_t *DataBuf, uint16_t DataBufLen)            
{
    uint32_t  i = 0;
    uint8_t   SlaveAddr;                                      //从机地址
    uint16_t  AddrTemp;                                       //地址缓存
    
    //	WriteFRAM_PROTECT(0);									//关写保护
    
    EEP_WaitEvent();                                        // 等信号量
    
    AddrTemp = Addr;
    
    AddrTemp += FRAM_START_ADDR;                            //计算地址
    
    if( (uint16_t)AddrTemp + DataBufLen > FRAM_END_ADDR ) 
    {
        EEP_SendEvent();                                // 释放信号量
        return  0;                                      //地址未超阶
    }
    
    SlaveAddr = FRAM_DIVICE_ADDR & (~(0x01));               //求FM24CL64设备写地址
    
    I2C_Start();                                             //启动IIC总线
    
    I2C_SendByte(SlaveAddr);                            //写FM24CL64设备写地址
	while(I2C_WaitAck());			  	   //等待从设备应答
    
    I2C_SendByte(Addr >> 8 );                           //写数据高位地址
	while(I2C_WaitAck());			  	   //等待从设备应答
    
    I2C_SendByte( Addr );                               //写数据低位地址
	while(I2C_WaitAck());			  	   //等待从设备应答
    
    SlaveAddr = (FRAM_DIVICE_ADDR)|(0x01);                  //求FM24CL64设备读地址
    
    I2C_Start();                                             //启动IIC总线
    
    I2C_SendByte(SlaveAddr);                            //写FM24CL16地址
	while(I2C_WaitAck());			  	   //等待从设备应答
    
    for(i = 0; i < (DataBufLen -1); i++)                    //发送数据
    {
        DataBuf[i] = I2C_ReadByte();                     //写数据
        I2C_Ack();                                     //主机应答从机
    }
    DataBuf[i] = I2C_ReadByte();                         //写数据
    
    I2C_NoAck();                                       //主机应答从机
    
    I2C_Stop();
    
    EEP_SendEvent();                                    // 释放信号量
    return  1;                                           //正确
}

void TO_Arayy(uint8_t *buf , uint32_t NUM)
{
	buf[0] = NUM & 0xFF;			  
	buf[1] = (NUM >> 8) & 0xFF;	   //低字节在前，高字节在后
	buf[2] = (NUM >> 16) & 0xFF;
	buf[3] = (NUM >> 24) & 0xFF;
}



