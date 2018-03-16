/*******************************************************************************
*   Filename:      bsp_r8025t.c
*   Revised:       $Date: 2015-01-26
*   Revision:      $V1.0
*   Writer:        Roger-WY.
*
*   Description:   RX8025实时时钟芯片驱动模块。
*
*   Notes:
*
*
*   All copyrights reserved to Roger-WY.
*
*******************************************************************************/

/*******************************************************************************
* INCLUDES
*/
#include "bsp_r8025t.h"


#define  BSP_RX8025T_MODULE_EN   1
#if  BSP_RX8025T_MODULE_EN > 0

/*******************************************************************************
 * 描述： 该模块是否搭载uCOS系统使用
 */
#define  UCOS_EN        DEF_ENABLED

/*******************************************************************************
 * 描述： RX8025芯片型号定义
 */
#define RX8025AC                    0
#define RX8025T                     1

/*******************************************************************************
* 描述： RX8025的具体型号定义（一定要注意型号要对，否则无法工作！！！）
*/
static  uint8_t    DevType    = RX8025AC;


/***********************************************
* 描述： 时间（us）
*/
#define DLY_CLK_SDA             2               // >=1.0us
#define DLY_CLK_LOW             2               // >=1.3us
#define DLY_CLK_HIGH            3               // >=0.6us
#define DLY_SU_STA              2               // >=0.6us
#define DLY_HD_STA              2               // >=0.6us
#define DLY_SU_STO              2               // >=0.6us
#define DLY_SDA_BUF             3               // >=1.3us
/***********************************************
* 描述： 时间（us）
*/
#define DLY_WR_STO              70              // >61us
/***********************************************
* 描述： 应答定义
*/
#define I2C_ACK                 0X01            // 应答
#define I2C_NACK                0X00            // 非应答
/***********************************************
* 描述： 电平定义
*/
#define LOW                     0X00            // 低电平
#define HIGH                    0X01            // 高电平

/***********************************************
* 描述： RX8025T寄存器地址定义
*/
/* r8025N寄存器定义 */
#define RX8025_REG_SEC          0x00
#define RX8025_REG_MIN          0x01
#define RX8025_REG_HOUR         0x02
#define RX8025_REG_WDAY         0x03
#define RX8025_REG_MDAY         0x04
#define RX8025_REG_MONTH        0x05
#define RX8025_REG_YEAR         0x06
#define RX8025_REG_DIGOFF       0x07//#define RX8025_REG_DIGOFF       0x07  8025Digital Offset
#define RX8025_REG_ALWMIN       0x08
#define RX8025_REG_ALWHOUR      0x09
#define RX8025_REG_ALWWDAY      0x0a
#define RX8025_REG_TMCNT0       0x0b//#define RX8025_REG_ALDMIN       0x0b
#define RX8025_REG_TMCNT1       0x0c//#define RX8025_REG_ALDHOUR      0x0c
/* 0x0d is reserved */
#define RX8025_REG_FLAG         0x0e//#define RX8025_REG_CTRL1        0x0e
#define RX8025_REG_CTRL         0x0f//#define RX8025_REG_CTRL2        0x0f

#define RX8025_BIT_FLAG_CT0    (1<<0)//
#define RX8025_BIT_FLAG_CT1    (1<<1)//
#define RX8025_BIT_FLAG_CT2    (1<<2)//
#define RX8025_BIT_FLAG_TEST   (1<<3)// 厂家测试
#define RX8025_BIT_FLAG_RSV    (1<<4)// 预留
#define RX8025_BIT_FLAG_12_24  (1<<5)// 0：12小时制；1：24小时制
#define RX8025_BIT_FLAG_DALE   (1<<6)// 时分一致的警报发生功能
#define RX8025_BIT_FLAG_WALE   (1<<7)// 星期，时，分一致报警

#define RX8025_BIT_CTRL_DAFG   (1<<0)// 只在DAFG?bit为1时有效Alarm_D的设定时刻与现行时刻一致
#define RX8025_BIT_CTRL_WAFG   (1<<1)//
#define RX8025_BIT_CTRL_CTFG   (1<<2)//
#define RX8025_BIT_CTRL_RSV    (1<<3)//
#define RX8025_BIT_CTRL_PON    (1<<4)// 检测电源复位检测时PON?bit为1
#define RX8025_BIT_CTRL_XSTL   (1<<5)//
#define RX8025_BIT_CTRL_VDET   (1<<6)//
#define RX8025_BIT_CTRL_VDSL   (1<<7)//

#define RX8025_ADDR_SEC          0x00
#define RX8025_ADDR_MIN          0x01
#define RX8025_ADDR_HOUR         0x02
#define RX8025_ADDR_WEEK         0x03
#define RX8025_ADDR_DAY          0x04
#define RX8025_ADDR_MON          0x05
#define RX8025_ADDR_YEAR         0x06

#define RX8025_ADDR_CONTROL1     0x0E
#define RX8025_ADDR_CONTROL2     0x0F

#define BSP_RX8025T_ADDR         0x64
#define BSP_RX8025T_ADDR_R       0x65

#define I2C_MAX_BYTES            16



/***********************************************
* 描述： OS接口
*/
#if (UCOS_EN     == DEF_ENABLED)
    #if OS_VERSION > 30000U
    static  OS_SEM                   Bsp_RX8025tSem;    // 信号量
    #else
    static  OS_EVENT                *Bsp_RX8025tSem;    // 信号量
    #endif
#endif

/*******************************************************************************
* GLOBAL FUNCTIONS
*/
static void    I2C_Start               (void);
static void    I2C_Stop                (void);

static uint8_t    I2C_SendByte         (uint8_t  byte);                         //数据从高位到低位
static uint8_t    I2C_RecvByte         (uint8_t    ack, uint8_t *dat);          //数据从高位到低位

//============================================================================//
/***********************************************
* 描述： I2C关键时序
*
*                ：开始/结束/开始时序
*                : SCL ˉˉˉˉˉˉ\______/ˉˉˉˉ\______/ˉˉˉˉˉˉˉˉˉˉˉˉˉˉˉ\___
*                : SDA ˉˉˉˉ\_______/ˉˉˉˉˉˉ\_______/ˉˉˉˉˉˉˉˉˉˉˉ\_____
*                :       |   |                 |   |         |   |
*                :       START      <0.5S      STOP   >61uS  START
*                ：数据时序
*                : SCL ______/ˉˉˉˉˉˉ\___/ˉˉˉˉˉˉˉˉ\______
*                : SDA _____/ˉˉˉˉˉˉˉˉˉXˉˉˉˉˉˉˉˉˉˉˉ\
*                :     ˉˉˉˉˉˉˉˉˉˉˉˉˉˉˉ ˉˉˉˉˉˉˉˉˉˉˉˉˉˉˉˉˉ
*                ：      |数据保持  |   |数据保持|
*                ：                数据改变
*/

/*******************************************************************************
* 名    称： I2C_Start
* 功    能： 启动I2C总线，即发送I2C初始条件
* 入口参数： 无
* 出口参数： 无
* 作　　者： Roger-WY
* 创建日期： 2015-08-18
* 修    改：
* 修改日期：
* 备    注：  开始时序
*             SDA ˉˉˉˉ\______
*             SCL ˉˉˉˉˉˉ\____
*                   |   |
*                   START
*******************************************************************************/
static void I2C_Start(void)
{
    SET_RX8025x_SDA_OUTPUT();				/*SDA设为输出                       */
    RX8025_SDA_H();				            /*发送起始条件的数据信号            */
    Delay_Nus(DLY_CLK_SDA);
    RX8025_SCL_H();
    Delay_Nus(DLY_SU_STA);		            /*起始条件建立时间大于4.7us,延时    */
    RX8025_SDA_L();					        /*发送起始信号                      */
    Delay_Nus(DLY_HD_STA);			        /*起始条件保持时间大于4μs           */
    RX8025_SCL_L();					        /*允许数据线改变，准备发送或接收数据*/
    Delay_Nus(DLY_CLK_LOW);                 /*CLK与SDA之间延时                  */
}

/*******************************************************************************
* 名    称： I2C_Stop
* 功    能： 结束I2C总线，即发送I2C结束条件
* 入口参数： 无
* 出口参数： 无
* 作　　者： Roger-WY
* 创建日期： 2015-08-18
* 修    改：
* 修改日期：
* 备    注： 停止时序
*            SDA _______/ˉˉˉˉˉ
*            SCL _____/ˉˉˉˉˉˉˉ
*                      |   |
*                      STOP
*******************************************************************************/
static void I2C_Stop(void)
{
	SET_RX8025x_SDA_OUTPUT();				            /*SDA设为输出*/
    RX8025_SCL_L(); 					            /*钳住I2C总线，准备发送或接收数据 */
	Delay_Nus(DLY_CLK_SDA);
    RX8025_SDA_L(); 					            /*发送结束条件的数据信号*/
    Delay_Nus(DLY_CLK_SDA);
    RX8025_SCL_H(); 					            /*发送结束条件的时钟信号*/
	Delay_Nus(DLY_SU_STO);			            /*结束条件建立时间大于4μs*/
    RX8025_SDA_H(); 					            /*发送I2C总线结束信号*/
    Delay_Nus(DLY_SDA_BUF); 		            /*>4us*/
}

/*******************************************************************************
* 名    称： I2C_SendByte
* 功    能： 字节数据传送函数，将数据 c 发送出去，可以是地址，也可以是数据，
             发完后等待应答，并对此状态位进行操作
* 入口参数： uint8_t 型数据
* 出口参数： 无
* 作　　者： Roger-WY
* 创建日期： 2015-08-18
* 修    改：
* 修改日期：
* 备    注： 数据时序
*            SDA _____/ˉˉˉˉˉˉˉˉˉXˉˉˉˉˉˉˉˉˉˉˉ\
*                ˉˉˉˉˉˉˉˉˉˉˉˉˉˉˉˉˉˉˉˉˉˉˉˉˉˉˉˉˉˉˉˉˉ
*            SCL ______/ˉˉˉˉˉˉ\___/ˉˉˉˉˉˉˉˉ\______
*                  |数据保持  |   |数据保持|
*                            数据改变
*******************************************************************************/
static uint8_t I2C_SendByte(uint8_t byte) 			//数据从高位到低位//
{
    INT08S  ctr     = 0;                            // 初始化数据位计数器
    uint8_t  ack     = 0;

    /***********************************************
    * 描述：发送数据
    */
    for ( ctr = 0 ; ctr < 8 ; ctr ++ ) {            // 检测是否已经发送完毕
        RX8025x_SET_SCL(LOW);                           // SCL = 0 允许数据线改变，准备发送数据
        RX8025x_SET_SDA((byte & 0x80) == 0x80);         // SDA = 0 发送数据：0;SDA = 1 发送数据：1
        RX8025x_SET_SCL(HIGH);                          // SCL = 1 锁定数据，从机可以读取
        byte <<= 1;                                 // 当前值左移一位
    };
    /***********************************************
    * 描述：检查应答
    */
    RX8025x_SET_SCL(LOW);                               // SCL = 0 允许数据线改变，准备接收应答位
    SET_RX8025x_SDA_INPUT();                        // SDA设为输入
    RX8025x_SET_SDA(HIGH);                              // SDA = 1 主机释放总线

    RX8025x_SET_SCL(HIGH);                              // SCL = 1 锁定数据
    RX8025x_GET_SDA(ack);                               // 读取从机应答ACK
    SET_RX8025x_SDA_INPUT();                        // SDA设为输出
    RX8025x_SET_SCL(LOW);                               // SCL = 0 允许数据线改变，准备接收应答位

    if (ack)                                    	// 检测应答信号
        return (FALSE);                             // NACK返回
    else
        return (TRUE);                              // ACK返回
}

/*******************************************************************************
* 名    称： I2C_RecvByte
* 功    能： 接收从器件传来的数据，并判断总线错误（不发应答信号），收完后
             需要调用应答函数。
* 入口参数： 无
* 出口参数： uint8_t 型数据
* 作　　者： Roger-WY
* 创建日期： 2015-08-18
* 修    改：
* 修改日期：
* 备    注： 数据时序
*            SDA _____/ˉˉˉˉˉˉˉˉˉXˉˉˉˉˉˉˉˉˉˉˉ\
*                ˉˉˉˉˉˉˉˉˉˉˉˉˉˉˉˉˉˉˉˉˉˉˉˉˉˉˉˉˉˉˉˉˉ
*            SCL ______/ˉˉˉˉˉˉ\___/ˉˉˉˉˉˉˉˉ\______
*                  |数据保持  |   |数据保持|
*                            数据改变
*******************************************************************************/
static uint8_t I2C_RecvByte(uint8_t ack, uint8_t *dat)  	//数据从高位到低位
{
    uint8_t  ctr     = 0;
    INT08S  data    = 0;

    SET_RX8025x_SDA_INPUT();                        // SDA设为输入
    RX8025x_SET_SCL(LOW);                               // SCL = 0 允许数据线改变，准备读取从机数据
    /***********************************************
    * 描述：接收数据
    */
    for ( ctr = 0 ; ctr < 8 ; ctr ++ ) {            // 数据左移一位，由于第一个数据是0所以左移还是0
        data <<= 1;                                 // 准备下一位数据
        RX8025x_SET_SCL(HIGH);                          // SCL = 1 锁定数据，可以读取从机数据
        RX8025x_GET_SDA(data);                          // 如果为1则置1，否则不改变（默认为0）
        RX8025x_SET_SCL(LOW);                           // SCL = 0 允许数据线改变，准备读取从机数据
    };

    SET_RX8025x_SDA_OUTPUT();                       // SDA设为输出
    /***********************************************
    * 描述：检查应答
    */
    RX8025x_SET_SDA(!ack);                              // ACK,发送0

    RX8025x_SET_SCL(LOW);
    RX8025x_SET_SCL(HIGH);
    RX8025x_SET_SDA(LOW);
    RX8025x_SET_SDA(HIGH);

    *dat    = data;

    return (TRUE);                            	    // Return 8-bit data byte
}

/*******************************************************************************
* 名    称： AT24CXX_Enabled
* 功    能： I2C使能
* 入口参数： 无
* 出口参数：
* 作　　者： Roger-WY
* 创建日期： 2015-08-18
* 修    改：
* 修改日期：
* 备    注：
*******************************************************************************/
static uint8_t BSP_I2C_Enabled( void )
{
    I2C_Start();
    return ( I2C_SendByte( BSP_RX8025T_ADDR ) );
}

/*******************************************************************************
* 名    称： BSP_I2C_GetReady
* 功    能： I2C总线是否空闲
* 入口参数： 无
* 出口参数：
* 作　　者： Roger-WY
* 创建日期： 2015-08-18
* 修    改：
* 修改日期：
* 备    注：
*******************************************************************************/
static uint8_t BSP_I2C_GetReady(void)
{
    return TRUE;
    //    int   timeout = 100;
    //
    //    while ( --timeout ) {
    //        if ( BSP_I2C_Enabled() == TRUE )
    //            return TRUE;
    //#if UCOS_EN     == DEF_ENABLED
    //#if OS_VERSION > 30000U
    //        BSP_OS_TimeDly(1);
    //#else
    //        OSTimeDly(1);
    //#endif
    //#else
    //        Delay_Nms(1);
    //#endif
    //    }
    //    return FALSE;
}

/*******************************************************************************
* 名    称： BSP_I2C_SetAddr
* 功    能： 设置当前地址
* 入口参数： suba - 地址； opt - 0:写； 1:读
* 出口参数：
* 作　　者： Roger-WY
* 创建日期： 2015-08-18
* 修    改：
* 修改日期：
* 备    注：
*******************************************************************************/
static uint8_t BSP_I2C_SetAddr( INT16U suba, uint8_t opt)
{
    uint8_t      ack;

    ack = I2C_SendByte(BSP_RX8025T_ADDR);           // 发送设备地址
    if(!ack)    goto exit;                          // 转至结束

#if I2C_MAX_BYTES > 256u
    ack = I2C_SendByte(suba>>8);                    // 发送内存高地址
    if(!ack)    goto exit;                          // 转至结束
#endif

    ack = I2C_SendByte(suba);                       // 发送内存低地址
    if(!ack)    goto exit;                          // 转至结束
    /***********************************************
    * 描述： 如果为读，则发送读地址
    */
    if ( opt ) {
        I2C_Start();                                  // 重新发送起始条件
        ack = I2C_SendByte(BSP_RX8025T_ADDR+1);
        if(!ack)  goto exit;                          // 转至结束
    }
    exit:
    return ack;
}

#if UCOS_EN     == DEF_ENABLED
/*******************************************************************************
* 名    称： BSP_RX8025T_WaitEvent
* 功    能： 等待信号量
* 入口参数： 无
* 出口参数：  0（操作有误），1（操作成功）
* 作　　者： Roger-WY
* 创建日期： 2015-08-18
* 修    改：
* 修改日期：
* 备    注： 仅在使用UCOS操作系统时使用
*******************************************************************************/
static uint8_t BSP_RX8025T_WaitEvent(void)
{
    /***********************************************
    * 描述： OS接口
    */
#if OS_VERSION > 30000U
    return BSP_OS_SemWait(&Bsp_RX8025tSem,0);       // 等待信号量
#else
    uint8_t       err;
    OSSemPend(Bsp_RX8025tSem,0,&err);               // 等待信号量
    if ( err = OS_ERR_NONE )
        return TRUE;
    else
        return FALSE;
#endif
}

/*******************************************************************************
* 名    称： BSP_RX8025T_SendEvent
* 功    能： 释放信号量
* 入口参数： 无
* 出口参数： 无
* 作　　者： Roger-WY
* 创建日期： 2015-08-18
* 修    改：
* 修改日期：
* 备    注： 仅在使用UCOS操作系统时使用
*******************************************************************************/
static void BSP_RX8025T_SendEvent(void)
{
    /***********************************************
    * 描述： OS接口
    */
#if OS_VERSION > 30000U
    BSP_OS_SemPost(&Bsp_RX8025tSem);                // 发送信号量
#else
    uint8_t       err;
    OSSemPost(Bsp_RX8025tSem);                      // 发送信号量
#endif
}
#endif /* end of (UCOS_EN     == DEF_ENABLED)*/


/*******************************************************************************
* 名    称： BSP_I2C_WriteByte
* 功    能： 从启动总线到发送地址、数据，结束总线的全过程，如果返回1，表示
             操作成功，否则操作有误。
* 入口参数： suba： 地址  byte写出的字节
* 出口参数： 0（操作有误），1（操作成功）
* 作　　者： Roger-WY
* 创建日期： 2015-08-18
* 修    改：
* 修改日期：
* 备    注：
*******************************************************************************/
uint8_t BSP_I2C_WriteByte(INT16U suba,uint8_t byte )
{
    uint8_t   ack;

    /***********************************************
    * 描述： OS接口
    */
    BSP_RX8025T_WaitEvent();                        // 等信号量
    Delay_Nus(DLY_WR_STO);                          // 上一个停止和下一个起始之间要最多延时10ms

    I2C_Start();                                    // 发送起始条件

    ack = BSP_I2C_SetAddr(suba,0);                  // 设置读地址
    if(!ack)    goto exit;                          // 转至结束

    ack = I2C_SendByte(byte);                       // 发送要写入的内容
    if(!ack)    goto exit;                          // 转至结束

    exit:
    I2C_Stop();                                     // 发送停止条件

    BSP_RX8025T_SendEvent();                        // 释放信号量

    return(ack);
}

/*******************************************************************************
* 名    称： BSP_I2C_ReadByte
* 功    能： 读一个字节
* 入口参数： suba： 地址  byte 读出的字节
* 出口参数： 0（操作有误），1（操作成功）
* 作　　者： Roger-WY
* 创建日期： 2015-08-18
* 修    改：
* 修改日期：
* 备    注：
*******************************************************************************/
uint8_t BSP_I2C_ReadByte( INT16U suba, uint8_t *byte)
{
    uint8_t       ack;
    /***********************************************
    * 描述： OS接口
    */
    BSP_RX8025T_WaitEvent();                        // 等信号量
    Delay_Nus(DLY_WR_STO);                          // 上一个停止和下一个起始之间要最多延时10ms

    I2C_Start();                                    // 发送起始条件

    ack = BSP_I2C_SetAddr(suba,1);                  // 设置读地址
    if(!ack)    goto exit;                          // 转至结束

    ack = I2C_RecvByte(I2C_NACK,byte);              // 接收内容并发关非应答
    exit:
    I2C_Stop();                                     // 发送停止条件

    BSP_RX8025T_SendEvent();                        // 释放信号量

    return ack;
}

/*******************************************************************************
* 名    称： BSP_I2C_ReadBytes
* 功    能： 读一串字符串
* 入口参数： suba： 地址  buf 缓存读出的字节数组 len 数组的大小
* 出口参数： 0（操作有误），1（操作成功）
* 作　　者： Roger-WY
* 创建日期： 2015-08-18
* 修    改：
* 修改日期：
* 备    注：
*******************************************************************************/
INT16U BSP_I2C_ReadBytes(INT16U suba,uint8_t *buf, INT16U len)
{
    INT16U      bytes   = 0;
    uint8_t      ack;
    /***********************************************
    * 描述： 检测是否超出存储容量
    */
    if ( (suba + len) > (I2C_MAX_BYTES) )
        return 0;
    /***********************************************
    * 描述： OS接口
    */
    BSP_RX8025T_WaitEvent();                        // 等信号量

    /***********************************************
    * 描述： 检测是否可操作
    */
    if ( BSP_I2C_GetReady() == FALSE )
        return 0;

    Delay_Nus(DLY_WR_STO);                          // 上一个停止和下一个起始之间要最多延时10ms
    /***********************************************
    * 描述： 检测是否到最后一字节
    */
    I2C_Start();                                    // 发送起始条件
    ack = BSP_I2C_SetAddr(suba,1);                  // 设置读地址

    if(!ack) {
        I2C_Stop();                                 // 发送停止条件
        return 0;
    }

    /***********************************************
    * 描述： 检测是否到最后一字节
    */
    for( bytes = 0; bytes < len; bytes++ ) {
        /***********************************************
        * 描述： 检测是否到最后一字节
        */
        if ( bytes == len - 1 ) {
            ack = I2C_RecvByte(I2C_NACK, buf++);
        } else {
            ack = I2C_RecvByte(I2C_ACK, buf++);
        }
        /***********************************************
        * 描述： 检测应答信号是否正确
        */
        if(!ack) {
            I2C_Stop();                             // 发送停止条件
            break;
        }
    }
    if(ack)
        I2C_Stop();                             // 发送停止条件
    /***********************************************
    * 描述： OS接口
    */
    BSP_RX8025T_SendEvent();                        // 释放信号量

    return bytes;
}


/*******************************************************************************
* 名    称： BSP_I2C_WriteBytes
* 功    能： 写多个字节
* 入口参数： suba： 地址  buf 需要写入的字节数组 len 数组的大小
* 出口参数： 0（操作有误），1（操作成功）
* 作　　者： Roger-WY
* 创建日期： 2015-08-18
* 修    改：
* 修改日期：
* 备    注：
*******************************************************************************/
INT16U BSP_I2C_WriteBytes( INT16U suba, uint8_t *pbuf, INT16U len )
{
    uint8_t      bytes   = 0;
    uint8_t      ack;
    /***********************************************
    * 描述： 检测地址是否会溢出，如果溢出则直接返回
    */
    if ( (suba + len) > (I2C_MAX_BYTES) )
        return 0;

    /***********************************************
    * 描述： OS接口
    */
    BSP_RX8025T_WaitEvent();                        // 等信号量

    if ( BSP_I2C_GetReady() == FALSE )
        return 0;

    Delay_Nus(DLY_WR_STO);                          // 上一个停止和下一个起始之间要最多延时10ms

    I2C_Start();

    ack = BSP_I2C_SetAddr(suba,0);                  // 设置读地址
    if(!ack)    goto exit;                          // 转至结束

    for ( ; bytes < len ; bytes++ ) {
        ack = I2C_SendByte( pbuf[ bytes ] );
        if(!ack)    goto exit;                      // 转至结束
    }
    exit:
    I2C_Stop();

    /***********************************************
    * 描述： OS接口
    */
    BSP_RX8025T_SendEvent();                        // 释放信号量

    return bytes;
}

/*******************************************************************************
* 名    称： BSP_RX8025_Init
* 功    能： RX8025时钟芯片初始化
* 入口参数： 无
* 出口参数： 0（初始化失败），1（初始化成功）
* 作　　者： Roger-WY
* 创建日期： 2015-08-18
* 修    改：
* 修改日期：
* 备    注：
*******************************************************************************/
uint8_t BSP_RX8025T_Init( void )
{
    GPIO_InitTypeDef    gpio_init;

    RCC_APB2PeriphClockCmd(RX8025x_SCL_RCC, ENABLE);
    gpio_init.GPIO_Pin   = RX8025x_SCL_PIN;
    gpio_init.GPIO_Speed = GPIO_Speed_50MHz;
    gpio_init.GPIO_Mode  = GPIO_Mode_Out_OD;
    GPIO_Init(RX8025x_SCL_PROT, &gpio_init);
    RCC_APB2PeriphClockCmd(RX8025x_SDA_RCC, ENABLE);
    gpio_init.GPIO_Pin   = RX8025x_SDA_PIN;
    GPIO_Init(RX8025x_SDA_PROT, &gpio_init);


    /***********************************************
    * 描述： OS接口
    */
#if UCOS_EN     == DEF_ENABLED
    #if OS_VERSION > 30000U
        BSP_OS_SemCreate(&Bsp_RX8025tSem,1, "Bsp RX8025tSem");  // 创建信号量
    #else
        Bsp_RX8025tSem     = OSSemCreate(1);                    // 创建信号量
    #endif
#endif

    /***********************************************
    * 描述： 初始化I2C总线电平
    */
    I2C_Start();
    I2C_Stop();
#if UCOS_EN     == DEF_ENABLED
    BSP_OS_TimeDly(5);
#else
    Delay_Nms(5);
#endif
    // SEC    MIN     HOUR    WEEK    DAY     MONTH   YEAR
    uint8_t time[16]  =   {  0x00,   0x57,   0x20,   0x02,   0x19,   0x05,   0x15,
    // RAM    MIN_AL  HOUR_AL WEEK_AL DAY_AL  CTR0    CTR1
    0X00,   0X00,   0X00,   0X00,           0X00,   0X00,
    //EXTENSION       FLAG    CTRL
    0X00,           0X20,   0X00};
    /***********************************************
    * 描述： 读取寄存器状态
    */
    BSP_I2C_ReadBytes(0, &time[0], 16);
    if ( time[2] == 0xFF ) {
        DevType = RX8025T;
    }

    if ( DevType == RX8025AC ) {
        //        BSP_I2C_ReadBytes(0, &time[0], 16);
        //        /***********************************************
        //        * 描述： 判断写入的数据是否恢复
        //        */
        //        if ( RX8025_BIT_CTRL_PON == ( time[RX8025_REG_CTRL] & RX8025_BIT_CTRL_PON ) ) {
        //            BSP_I2C_WriteByte( RX8025_REG_DIGOFF, 0x00 );
        //            time[RX8025_REG_SEC]    = Hex2Bcd(30);
        //            time[RX8025_REG_MIN]    = Hex2Bcd(55);
        //            time[RX8025_REG_HOUR]   = Hex2Bcd(9);
        //            time[RX8025_REG_WDAY]   = 5;
        //            time[RX8025_REG_MDAY]   = Hex2Bcd(30);
        //            time[RX8025_REG_MONTH]  = Hex2Bcd(4);
        //            time[RX8025_REG_YEAR]   = Hex2Bcd(15);
        //            time[7]                 = 0x00;
        //            //for ( char i = 0; i < 16; i++ )
        //            BSP_I2C_WriteBytes(0, &time[0],16);
        //        }
    } else {
        for ( char i = 7; i < 8; i++ )
            BSP_I2C_ReadByte(i, &time[i]);
        if ( 0x05 != ( time[7] & 0x05 ) ) {
            time[RX8025_REG_SEC]    = Hex2Bcd(30);
            time[RX8025_REG_MIN]    = Hex2Bcd(55);
            time[RX8025_REG_HOUR]   = Hex2Bcd(9);
            time[RX8025_REG_WDAY]   = 5;
            time[RX8025_REG_MDAY]   = Hex2Bcd(30);
            time[RX8025_REG_MONTH]  = Hex2Bcd(4);
            time[RX8025_REG_YEAR]   = Hex2Bcd(15);
            time[7]                 = 0x05;

            for ( char i = 8; i < 16; i++)
                time[i] = 0x00;
            for ( char i = 0; i < 16; i++ )
                BSP_I2C_WriteByte(i, time[i]);
        }
    }

    return(TRUE);
}


/*******************************************************************************
* 名    称： BSP_RX8025T_GetTime
* 功    能： 读取时间：年，月，日，星期，时，分，秒
* 入口参数： *t_tm - 时间接收结构体
* 出口参数： 无
* 作　　者： Roger-WY
* 创建日期： 2015-08-18
* 修    改：
* 修改日期：
* 备    注： 在bsp_time.c中调用该函数
*******************************************************************************/
uint8_t BSP_RX8025T_GetTime(struct tm *t_tm)
{
    u8 time[7];
    u8 i = 0;
    if ( DevType == RX8025AC ) {
        /***********************************************
        * 描述： 读取时间
        */
        if ( 7 == BSP_I2C_ReadBytes(0, time, 7) ) {
            t_tm->tm_sec     = Bcd2Hex(time[0]);
            t_tm->tm_min     = Bcd2Hex(time[1]);
            t_tm->tm_hour    = Bcd2Hex(time[2]);
            t_tm->tm_wday    = time[3];
            t_tm->tm_mday    = Bcd2Hex(time[4]);
            t_tm->tm_mon     = Bcd2Hex(time[5]);
            t_tm->tm_year    = 2000+Bcd2Hex(time[6]);
            return TRUE;
        }
    } else {
        for (i = 0; i < 7; i++ ) {
            BSP_I2C_ReadByte(i,&time[i]);
        }
        if ( i == 7) {
            t_tm->tm_sec     = Bcd2Hex(time[0]);
            t_tm->tm_min     = Bcd2Hex(time[1]);
            t_tm->tm_hour    = Bcd2Hex(time[2]);
            t_tm->tm_wday    = time[3];
            t_tm->tm_mday    = Bcd2Hex(time[4]);
            t_tm->tm_mon     = Bcd2Hex(time[5]);
            t_tm->tm_year    = 2000+Bcd2Hex(time[6]);
            return TRUE;
        }
    }
    return FALSE;
}

/*******************************************************************************
* 名    称： BSP_RX8025T_SetDate
* 功    能： 设置时间：年，月，时，星期，时，分，秒
* 入口参数： 无
* 出口参数： 无
* 作　　者： Roger-WY
* 创建日期： 2015-08-18
* 修    改：
* 修改日期：
* 备    注： 在bsp_time.c中调用该函数
*******************************************************************************/
uint8_t BSP_RX8025T_SetDate(struct tm t_tm)
{
    uint8_t  time[16];
    uint8_t  i;
    /***********************************************
    * 描述： 判断年是否合法
    */
    if ( t_tm.tm_year < 2000 )
        t_tm.tm_year = 2000;

    if ( DevType == RX8025AC ) {
        /***********************************************
        * 描述： 先读取设置
        */
        if ( 16 != BSP_I2C_ReadBytes(0, &time[0], 16) )
            return FALSE;

        time[0]     = Hex2Bcd(t_tm.tm_sec);
        time[1]     = Hex2Bcd(t_tm.tm_min);
        time[2]     = Hex2Bcd(t_tm.tm_hour);
        time[3]     = Hex2Bcd(t_tm.tm_wday);
        time[4]     = Hex2Bcd(t_tm.tm_mday);
        time[5]     = Hex2Bcd(t_tm.tm_mon);
        time[6]     = Hex2Bcd(t_tm.tm_year - 2000);
        /***********************************************
        * 描述： RAM
        */
        time[7]     = 0X00;
        /***********************************************
        * 描述： 24小时制
        */
        time[14]   |= 0x20;

        /***********************************************
        * 描述： 写入时间
        */
        if ( 16 == BSP_I2C_WriteBytes(0, &time[0], 16) )
            return TRUE;
    } else {
        /***********************************************
        * 描述： 先读取设置
        */
        time[0]     = Hex2Bcd(t_tm.tm_sec);
        time[1]     = Hex2Bcd(t_tm.tm_min);
        time[2]     = Hex2Bcd(t_tm.tm_hour);
        time[3]     = Hex2Bcd(t_tm.tm_wday);
        time[4]     = Hex2Bcd(t_tm.tm_mday);
        time[5]     = Hex2Bcd(t_tm.tm_mon);
        time[6]     = Hex2Bcd(t_tm.tm_year - 2000);
        /***********************************************
        * 描述： RAM
        */
        //time[7]     = 0X00;
        /***********************************************
        * 描述： 24小时制
        */
        //time[14]   |= 0x20;
        /***********************************************
        * 描述： 写入时间
        */
        for ( i = 0; i < 7; i++ ) {
            BSP_I2C_WriteByte(i,time[i]);
        }
        if ( i == 7)
            return TRUE;
    }
    return FALSE;
}

#endif
/*******************************************************************************
* 				end of file
*******************************************************************************/