/*******************************************************************************
*   Filename:       bsp_eeprom.c
*   Revised:        All copyrights reserved to Roger.
*   Date:           2015-08-18
*   Revision:       v1.0
*   Writer:	        wumingshen.
*
*   Description:    AT24Cxxx系列EEPROM驱动模块
*
*
*   Notes:
*
*   All copyrights reserved to wumingshen
*******************************************************************************/

/*******************************************************************************
 * INCLUDES
 */
#include <bsp_eeprom.h>


#define  BSP_EEPROM_MODULE_EN   0
#if  BSP_EEPROM_MODULE_EN > 0

/*******************************************************************************
 * 描述： 该模块是否搭载uCOS系统使用
 */
#define  UCOS_EN            DEF_ENABLED
#define  GPIO_I2C1_PORT_EN  DEF_ENABLED
#define  GPIO_I2C2_PORT_EN  DEF_DISABLED
/***********************************************
* 描述： AT24Cxx时间
*
Symbol  Parameter               |1.8-volt       |2.7-volt       |5.0-volt       Units
                                |Min    Max     |Min    Max     |Min    Max
fSCL    Clock Frequency, SCL    |       100     |       400     |       1000    kHz
tLOW    Clock Pulse Width Low   |4.7            |1.3            |0.6            μs
tHIGH   Clock Pulse Width High  |4.0            |1.0            |0.4            μs
tAA     Clock Low to Data Out   |               |               |
        Valid                   |0.1     4.5    |0.05    0.9    |0.05   0.55    μs
tBUF    Time the bus must be    |               |               |
        free before a new       |               |               |
        transmission can start  |4.7            |1.3            |0.5            μs
tHD.STA Start Hold Time         |4.0            |0.6            |0.25           μs
tSU.STA Start Set-up Time       |4.7            |0.6            |0.25           μs
tHD.DAT Data In Hold Time       |0              |0              |0              μs
tSU.DAT Data In Set-up Time     |200            |100            |100            ns
tR      Inputs Rise Time(1)     |        1.0    |        0.3    |       0.3     μs
tF      Inputs Fall Time(1)     |        300    |        300    |       100     ns
tSU.STO Stop Set-up Time        |4.7            |0.6            |0.25           μs
tDH     Data Out Hold Time      |100            |50             |50             ns
tWR     Write Cycle Time        |        20     |         10    |10             ms
Endurance(1)                    |               |               |
        5.0V, 25°C, Page Mode   |100K           |100K           |100K           Write Cycles
按2.7V供电计算时间
*/
/***********************************************
* 描述： AT24Cxx时间（us）
*/
#define DLY_CLK_SDA             10               // >=1.0us
#define DLY_CLK_LOW             10               // >=1.3us
#define DLY_CLK_HIGH            10               // >=1.0us
#define DLY_SU_STA              10               // >=0.6us
#define DLY_HD_STA              10               // >=0.6us
#define DLY_SU_STO              10               // >=0.6us
#define DLY_SDA_BUF             10               // >=1.3us

/***********************************************
* 描述： AT24Cxx时间（ms）
*/
#define DLY_WR_STO              5               // >=10ms

/***********************************************
* 描述： AT24CXX应答定义
*/
#ifndef I2C_ACK
#define I2C_ACK                 0X01            // 应答
#endif
#ifndef I2C_NACK
#define I2C_NACK                0X00            // 非应答
#endif
#ifndef HIGH
#define LOW                     0X00            // 低电平
#endif
#ifndef HIGH
#define HIGH                    0X01            // 高电平
#endif
/***********************************************
* 描述： AT24CXX读写模式定义
*/
#define MODE_PAGE_RW            0               // 页读写
#define MODE_SINGLE_RW          1               // 单个读写
#define BSP_EEP_READ_MODE       MODE_PAGE_RW    // 0分页读，1单字节读
#define BSP_EEP_WRITE_MODE      MODE_PAGE_RW    // 0分页写，1单字节写

INT32U  AT24CXX_SCL_PIN         = GPIO_Pin_6;
INT32U  AT24CXX_SDA_PIN         = GPIO_Pin_7;

/***********************************************
* 描述： AT24CXX使用的IO，配置为开漏输出
*/
#define I2C1_SCL_RCC            RCC_APB2Periph_GPIOE
#define I2C1_SCL_PORT           GPIOE
#define I2C1_SCL_PIN            GPIO_Pin_7

#define I2C1_SDA_RCC            RCC_APB2Periph_GPIOE
#define I2C1_SDA_PORT           GPIOE
#define I2C1_SDA_PIN            GPIO_Pin_8

#define I2C2_SCL_RCC            RCC_APB2Periph_GPIOB
#define I2C2_SCL_PORT           GPIOB
#define I2C2_SCL_PIN            GPIO_Pin_10

#define I2C2_SDA_RCC            RCC_APB2Periph_GPIOB
#define I2C2_SDA_PORT           GPIOB
#define I2C2_SDA_PIN            GPIO_Pin_11


#define AT24CXX_SCL_RCC         RCC_APB2Periph_GPIOB
#define AT24CXX_SCL_PORT        GPIOB
extern INT32U  AT24CXX_SCL_PIN;//         GPIO_Pin_6

#define AT24CXX_SDA_RCC         RCC_APB2Periph_GPIOB
#define AT24CXX_SDA_PORT        GPIOB
extern INT32U  AT24CXX_SDA_PIN;//         GPIO_Pin_7
/*******************************************************************************
* 描述： AT24CXX使用的端口设置(直接操作寄存器)，直接修改该处端口
*/
#define AT_SCL_1                GPIOB->BSRR = AT24CXX_SCL_PIN
#define AT_SCL_0                GPIOB->BRR  = AT24CXX_SCL_PIN

#define AT_SDA_1                GPIOB->BSRR = AT24CXX_SDA_PIN
#define AT_SDA_0                GPIOB->BRR  = AT24CXX_SDA_PIN

#define AT_SCL_S                ( GPIOB->IDR  & AT24CXX_SCL_PIN )
#define AT_SDA_S                ( GPIOB->IDR  & AT24CXX_SDA_PIN )


//============================================================================//

/***********************************************
* 描述： AT24CXX使用的端口设置，直接修改该处端口
*/
#define AT24CXX_SET_SCL(sta)    { if(sta) {AT_SCL_1; Delay_Nus(DLY_CLK_HIGH);} else { AT_SCL_0; Delay_Nus(DLY_CLK_SDA); } }
#define AT24CXX_SET_SDA(sta)    { if(sta) {AT_SDA_1; Delay_Nus(DLY_CLK_LOW); } else { AT_SDA_0; Delay_Nus(DLY_CLK_LOW); } }
#define AT24CXX_GET_SCL(sta)    { if( AT_SCL_S ) sta |= 1; else sta &= ~1; }
#define AT24CXX_GET_SDA(sta)    { if( AT_SDA_S ) sta |= 1; else sta &= ~1; }

/***********************************************
* 描述： AT24CXX引脚设置为开漏，可无须设置IO的方向
*/
#define DIR_SEL         0

#if DIR_SEL == 0
#define SET_SDA_INPUT()
#define SET_SDA_OUTPUT()
#define SET_SCL_INPUT()
#define SET_SCL_OUTPUT()
#else
#define SET_SDA_INPUT()   { GPIO_InitTypeDef  GPIO_InitStructure;\
                            GPIO_InitStructure.GPIO_Pin =  AT24CXX_SDA_PIN;\
                            GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;\
                            GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;\
                            GPIO_Init(AT24CXX_SDA_PORT, &GPIO_InitStructure);  }

#define SET_SDA_OUTPUT()  { GPIO_InitTypeDef  GPIO_InitStructure;\
                            GPIO_InitStructure.GPIO_Pin =  AT24CXX_SDA_PIN;\
                            GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;\
                            GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;\
                            GPIO_Init(AT24CXX_SDA_PORT, &GPIO_InitStructure);  }

#define SET_SCL_INPUT()   { GPIO_InitTypeDef  GPIO_InitStructure;\
                            GPIO_InitStructure.GPIO_Pin =  AT24CXX_SCL_PIN;\
                            GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;\
                            GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;\
                            GPIO_Init(AT24CXX_SCL_PORT, &GPIO_InitStructure);  }

#define SET_SCL_OUTPUT()  { GPIO_InitTypeDef  GPIO_InitStructure;\
                            GPIO_InitStructure.GPIO_Pin =  AT24CXX_SCL_PIN;\
                            GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;\
                            GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;\
                            GPIO_Init(AT24CXX_SCL_PORT, &GPIO_InitStructure);  }
#endif

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
 * GLOBAL FUNCTIONS
 */
static void    I2C_Start      (void);
static void    I2C_Stop       (void);

static uint8_t I2C_SendByte   (uint8_t  byte);                //数据从高位到低位
static uint8_t I2C_RecvByte   (uint8_t    ack, uint8_t *dat); //数据从高位到低位

//============================================================================//

/*******************************************************************************
* 描述： AT24Cxx关键时序
*
*                ：开始/结束时序
*                : SDA ˉˉˉˉ\_______/ˉˉˉˉˉˉ\_______/ˉˉˉˉˉ
*                : SCL ˉˉˉˉˉˉ\______/ˉˉˉˉ\______/ˉˉˉˉˉˉˉ
*                :       |   |                   |   |
*                :       START                   STOP
*                ：数据时序
*                : SDA _____/ˉˉˉˉˉˉˉˉˉXˉˉˉˉˉˉˉˉˉˉˉ\
*                :     ˉˉˉˉˉˉˉˉˉˉˉˉˉˉˉˉˉˉˉˉˉˉˉˉˉˉˉˉˉˉˉˉˉ
*                : SCL ______/ˉˉˉˉˉˉ\___/ˉˉˉˉˉˉˉˉ\______
*                ：      |数据保持  |   |数据保持|
*                ：                数据改变
*/


/*******************************************************************************
* 名    称： I2C_Start
* 功    能： 启动I2C总线，即发送I2C初始条件
* 入口参数： 无
* 出口参数： 无
* 作　 　者： 无名沈
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
    SET_SDA_OUTPUT();				/*SDA设为输出                       */
    AT_SDA_1;						/*发送起始条件的数据信号            */
    Delay_Nus(DLY_CLK_SDA);
    AT_SCL_1;
    Delay_Nus(DLY_SU_STA);		    /*起始条件建立时间大于4.7us,延时    */
    AT_SDA_0;						/*发送起始信号                      */
    Delay_Nus(DLY_HD_STA);			/*起始条件保持时间大于4μs           */
    AT_SCL_0;						/*允许数据线改变，准备发送或接收数据*/
    Delay_Nus(DLY_CLK_LOW);         /*CLK与SDA之间延时                  */
}

/*******************************************************************************
* 名    称： I2C_Stop
* 功    能： 结束I2C总线，即发送I2C结束条件
* 入口参数： 无
* 出口参数： 无
* 作　 　者： 无名沈
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
	SET_SDA_OUTPUT();				/*SDA设为输出*/
    AT_SCL_0; 						/*钳住I2C总线，准备发送或接收数据 */
	Delay_Nus(DLY_CLK_SDA);
    AT_SDA_0; 						/*发送结束条件的数据信号*/
    Delay_Nus(DLY_CLK_SDA);
    AT_SCL_1; 						/*发送结束条件的时钟信号*/
	Delay_Nus(DLY_SU_STO);			/*结束条件建立时间大于4μs*/
    AT_SDA_1; 						/*发送I2C总线结束信号*/
    Delay_Nus(DLY_SDA_BUF); 		/*>4us*/
}

/*******************************************************************************
* 名    称： I2C_SendByte
* 功    能： 字节数据传送函数，将数据 c 发送出去，可以是地址，也可以是数据，
             发完后等待应答，并对此状态位进行操作
* 入口参数： uint8_t 型数据
* 出口参数： 无
* 作　 　者： 无名沈
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
static uint8_t I2C_SendByte(uint8_t byte) 					    //数据从高位到低位//
{
    INT08S  ctr     = 0;                            // 初始化数据位计数器
    uint8_t  ack     = 0;

    /***********************************************
    * 描述：发送数据
    */
    for ( ctr = 0 ; ctr < 8 ; ctr ++ ) {            // 检测是否已经发送完毕
        AT24CXX_SET_SCL(LOW);                       // SCL = 0 允许数据线改变，准备发送数据
        AT24CXX_SET_SDA((byte & 0x80) == 0x80);     // SDA = 0 发送数据：0;SDA = 1 发送数据：1
        AT24CXX_SET_SCL(HIGH);                      // SCL = 1 锁定数据，从机可以读取
        byte <<= 1;                                 // 当前值左移一位
    };
    /***********************************************
    * 描述：检查应答
    */
    AT24CXX_SET_SCL(LOW);                           // SCL = 0 允许数据线改变，准备接收应答位
    SET_SDA_INPUT();                                // SDA设为输入
    AT24CXX_SET_SDA(HIGH);                          // SDA = 1 主机释放总线

    AT24CXX_SET_SCL(HIGH);                          // SCL = 1 锁定数据
    AT24CXX_GET_SDA(ack);                           // 读取从机应答ACK
    SET_SDA_OUTPUT();                               // SDA设为输出
    AT24CXX_SET_SCL(LOW);                           // SCL = 0 允许数据线改变，准备接收应答位

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
* 作　 　者： 无名沈
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
static uint8_t I2C_RecvByte(uint8_t ack, uint8_t *dat)  	        //数据从高位到低位
{
    uint8_t  ctr     = 0;
    INT08S  data    = 0;

    SET_SDA_INPUT();                                // SDA设为输入
    /***********************************************
    * 描述：接收数据
    */
    for ( ctr = 0 ; ctr < 8 ; ctr ++ ) {            // 数据左移一位，由于第一个数据是0所以左移还是0
        data <<= 1;                                 // 准备下一位数据
        AT24CXX_SET_SCL(HIGH);                      // SCL = 1 锁定数据，可以读取从机数据
        AT24CXX_GET_SDA(data);                      // 如果为1则置1，否则不改变（默认为0）
        AT24CXX_SET_SCL(LOW);                       // SCL = 0 允许数据线改变，准备读取从机数据
    };

    SET_SDA_OUTPUT();                               // SDA设为输出
    /***********************************************
    * 描述：检查应答
    */
    AT24CXX_SET_SDA(!ack);                          // ACK,发送0

    AT24CXX_SET_SCL(HIGH);
    AT24CXX_SET_SCL(LOW);
    AT24CXX_SET_SDA(HIGH);

    *dat    = data;

    return (TRUE);                            	    // Return 8-bit data byte
}

/*******************************************************************************
* 名    称： AT24CXX_Enabled
* 功    能： I2C使能
* 入口参数： 无
* 出口参数：
* 作　 　者： 无名沈
* 创建日期： 2015-08-18
* 修    改：
* 修改日期：
* 备    注：
*******************************************************************************/
uint8_t AT24CXX_Enabled( void )
{
    INT32S timeout  = 100;
    do {
        I2C_Start();
        if ( I2C_SendByte( 0xA0 ) == TRUE )
            return TRUE;
    } while ( --timeout );
    
    return FALSE;
}

/*******************************************************************************
* 名    称： AT24CXX_Init
* 功    能： 初始化IIC
* 入口参数： 无
* 出口参数： 无
* 作　 　者： 无名沈
* 创建日期： 2015-08-18
* 修    改：
* 修改日期：
* 备    注： 停止时序
*            SDA _______/ˉˉˉˉˉ
*            SCL _____/ˉˉˉˉˉˉˉ
*                      |   |
*                      STOP
*******************************************************************************/
void AT24CXX_Init( void )
{
    uint8_t size = 0;
    do {
        AT24CXX_SET_SCL(LOW);                       // SCL = 0 允许数据线改变，准备发送数据
        AT24CXX_SET_SDA(HIGH);                      // SDA = 1 主机释放总线
        AT24CXX_SET_SCL(HIGH);                      // SCL = 1 锁定数据，从机可以读取
        size        += 1;
    } while ( ( size < 10 ) && ( AT_SDA_S == 0 ) );

    I2C_Start();
    I2C_Stop();
}

/*******************************************************************************
* 名    称： BSP_EEP_SetAddr
* 功    能： 设置当前地址
* 入口参数： suba - 地址； opt - 0:写； 1:读
* 出口参数：
* 作　 　者： 无名沈
* 创建日期： 2015-08-18
* 修    改：
* 修改日期：
* 备    注：
*******************************************************************************/
uint8_t BSP_EEP_SetAddr( uint16_t suba, uint8_t opt)
{
    uint8_t      ack;
#if    AT_DEV_NO_SEL_PIN ==  DEF_ENABLED 
    //uint8_t    ctrl = BSP_EEP_I2C_ADDR | ((suba >> 7 ) & 0x0E);
    uint8_t    ctrl = BSP_EEP_I2C_ADDR | ((suba / 256) * 2 );
    ack = I2C_SendByte(ctrl);                   // 发送设备地址 + 存储块
#else
    ack = I2C_SendByte(BSP_EEP_I2C_ADDR);       // 发送设备地址
#endif
    if(!ack)    goto exit;                      // 转至结束

#if    AT_DEV_NO_SEL_PIN ==  DEF_ENABLED 
    #if AT_MAX_BYTES > 32*1024u
    ack = I2C_SendByte(suba>>8);                // 发送内存高地址
    if(!ack)    goto exit;                      // 转至结束
    #endif
#else
    #if AT_MAX_BYTES > 256u
    ack = I2C_SendByte(suba>>8);                // 发送内存高地址
    if(!ack)    goto exit;                      // 转至结束
    #endif
#endif
    ack = I2C_SendByte(suba);                   // 发送内存低地址
    if(!ack)    goto exit;                      // 转至结束
    /***********************************************
    * 描述： 如果为读，则发送读地址
    */
    if ( opt ) {
      I2C_Start();                              // 重新发送起始条件
      ack = I2C_SendByte(BSP_EEP_I2C_ADDR+1);
      if(!ack)  goto exit;                      // 转至结束
    }
exit:
    return ack;
}

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
uint8_t EEP_WaitEvent(void)
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
void EEP_SendEvent(void)
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
#endif /* end of (UCOS_EN     == DEF_ENABLED)*/


/*******************************************************************************
* 名    称： BSP_EEP_WriteByte
* 功    能： 从启动总线到发送地址、数据，结束总线的全过程，如果返回1，表示
             操作成功，否则操作有误。
* 入口参数： suba： EEPROM地址  byte写出的字节
* 出口参数： 0（操作有误），1（操作成功）
* 作　 　者： 无名沈
* 创建日期： 2015-08-18
* 修    改：
* 修改日期：
* 备    注：
*******************************************************************************/
uint8_t BSP_EEP_WriteByte(uint16_t suba,uint8_t byte )
{
    uint8_t   ack;

#if (UCOS_EN     == DEF_ENABLED)
    /***********************************************
    * 描述： OS接口
    */
    EEP_WaitEvent();                            // 等信号量
#endif

    I2C_Start();                                // 发送起始条件

    ack = BSP_EEP_SetAddr(suba,0);              // 设置读地址
    if(!ack)    goto exit;                      // 转至结束

    ack = I2C_SendByte(byte);                   // 发送要写入的内容
    if(!ack)    goto exit;                      // 转至结束

exit:
    I2C_Stop();                                 // 发送停止条件

#if (UCOS_EN     == DEF_ENABLED)
    EEP_SendEvent();                            // 释放信号量
#endif

    return(ack);
}

/*******************************************************************************
* 名    称： BSP_EEP_ReadByte
* 功    能： 读1个字节。
* 入口参数： suba： EEPROM地址  byte读出的字节
* 出口参数： 0（操作有误），1（操作成功）
* 作　 　者： 无名沈
* 创建日期： 2015-08-18
* 修    改：
* 修改日期：
* 备    注：
*******************************************************************************/
uint8_t BSP_EEP_ReadByte( uint16_t suba, uint8_t *byte)
{
    uint8_t       ack;

#if (UCOS_EN     == DEF_ENABLED)
    /***********************************************
    * 描述： OS接口
    */
    EEP_WaitEvent();                            // 等信号量
#endif

    I2C_Start();                                // 发送起始条件

    ack = BSP_EEP_SetAddr(suba,1);              // 设置读地址
    if(!ack)    goto exit;                      // 转至结束

    ack = I2C_RecvByte(I2C_NACK,byte);          // 接收内容并发关非应答
exit:
    I2C_Stop();                                 // 发送停止条件

#if (UCOS_EN     == DEF_ENABLED)
    EEP_SendEvent();                            // 释放信号量
#endif

    return ack;
}

/*******************************************************************************
* 名    称： BSP_EEP_WriteByteChk
* 功    能： 写一个字节并且将写入的字节再都出来。
* 入口参数： suba： EEPROM地址  byte写入的字节
* 出口参数： 等于写入的字节byte
* 作　 　者： 无名沈
* 创建日期： 2015-08-18
* 修    改：
* 修改日期：
* 备    注：
*******************************************************************************/
uint8_t BSP_EEP_WriteByteChk(uint16_t suba,uint8_t byte )
{
    uint8_t   chk;
    uint8_t   retrys  = 10;

    do {
#if (UCOS_EN     == DEF_ENABLED)
#if OS_VERSION > 30000U
        BSP_OS_TimeDly(DLY_WR_STO);
#else
        OSTimeDly(DLY_WR_STO);
#endif
#else
        Delay_Nms(DLY_WR_STO);
#endif
        if( BSP_EEP_WriteByte( suba, byte ) ) {
            chk     = 0;
#if (UCOS_EN     == DEF_ENABLED)
#if OS_VERSION > 30000U
            BSP_OS_TimeDly(DLY_WR_STO);
#else
            OSTimeDly(DLY_WR_STO);
#endif
#else
            Delay_Nms(DLY_WR_STO);
#endif
            BSP_EEP_ReadByte( suba, &chk );
            if ( chk == byte )
                return TRUE;
        }
    } while(--retrys);

    return(FALSE);
}

/*******************************************************************************
* 名    称： BSP_EEP_WritePage
* 功    能： 写一页数据
* 入口参数： suba： EEPROM地址  pbuf ： 写入字节的数组 len : 数组的大小
* 出口参数： 0（操作有误），1（操作成功）
* 作　 　者： 无名沈
* 创建日期： 2015-08-18
* 修    改：
* 修改日期：
* 备    注：
*******************************************************************************/
uint8_t BSP_EEP_WritePage( uint16_t suba , uint8_t * pbuf , uint8_t len )
{
    uint8_t    size   = 0;
    uint8_t    ack;

    if ( len > AT_PAGE_SIZE )
      return 0;

    if ( AT24CXX_Enabled() == FALSE )
        return 0;
    
#if (UCOS_EN     == DEF_ENABLED)
#if OS_VERSION > 30000U
    BSP_OS_TimeDly(DLY_WR_STO);
#else
    OSTimeDly(DLY_WR_STO);
#endif
#else
    Delay_Nms(DLY_WR_STO);                      // 上一个停止和下一个起始之间要最多延时10ms
#endif

    I2C_Start();

    ack = BSP_EEP_SetAddr(suba,0);              // 设置读地址
    if(!ack)    goto exit;                      // 转至结束

    for ( ; size < len ; size ++ ) {
        ack = I2C_SendByte( pbuf[ size ] );
        if(!ack)    goto exit;                      // 转至结束
    }
exit:
    I2C_Stop();
    return size;
}

/*******************************************************************************
* 名    称： BSP_EEP_ReadPage
* 功    能： 读一页数据
* 入口参数： suba： EEPROM地址  pbuf ： 写入字节的数组 len : 数组的大小
* 出口参数： 0（操作有误），1（操作成功）
* 作　 　者： 无名沈
* 创建日期： 2015-08-18
* 修    改：
* 修改日期：
* 备    注：
*******************************************************************************/
uint16_t BSP_EEP_ReadPage(uint16_t suba,uint8_t *pbuf, uint16_t len)
{
    uint8_t    size   = 0;
    uint8_t    ack;
    /***********************************************
    * 描述： 检测是否超出存储容量
    */
    if ( (suba + len) > (AT_MAX_BYTES) )
        return 0;

    /***********************************************
    * 描述： 检测是否可操作
    */
    if ( AT24CXX_Enabled() == FALSE )
        return 0;
    
#if (UCOS_EN     == DEF_ENABLED)
#if OS_VERSION > 30000U
    BSP_OS_TimeDly(DLY_WR_STO);
#else
    OSTimeDly(DLY_WR_STO);
#endif
#else
    Delay_Nms(DLY_WR_STO);                          // 上一个停止和下一个起始之间要最多延时10ms
#endif
    /***********************************************
    * 描述： 检测是否到最后一字节
    */
    I2C_Start();                                    // 发送起始条件
    ack = BSP_EEP_SetAddr(suba,1);                  // 设置读地址

    if(!ack) {
        I2C_Stop();                               // 发送停止条件
        return 0;
    }

    /***********************************************
    * 描述： 检测是否到最后一字节
    */
    for( size = 0; size < len; size++ ) {
        /***********************************************
        * 描述： 检测是否到最后一字节
        */
        if ( size == len - 1 ) {
            ack = I2C_RecvByte(I2C_NACK, pbuf++);
        } else {
            ack = I2C_RecvByte(I2C_ACK, pbuf++);
        }
        /***********************************************
        * 描述： 检测应答信号是否正确
        */
        if(!ack) {
            //I2C_Stop();                   // 发送停止条件
            break;
        }
    }
 
    I2C_Stop();                   // 发送停止条件

    return size;
}

static uint8_t  buf[AT_PAGE_SIZE];
/*******************************************************************************
* 名    称： BSP_EEP_WriteBytes
* 功    能： 写多个字节
* 入口参数： suba： EEPROM地址  pbuf ：写入字节的数组 len : 数组的大小
* 出口参数： 0（操作有误），1（操作成功）
* 作　 　者： 无名沈
* 创建日期： 2015-08-18
* 修    改：
* 修改日期：
* 备    注：
*******************************************************************************/
uint16_t BSP_EEP_WriteBytes( uint16_t suba, uint8_t *pbuf, uint16_t len )
{
    uint16_t      bytes   = 0;
    /***********************************************
    * 描述： 检测地址是否会溢出，如果溢出则直接返回
    */
    if ( (suba + len) > (AT_MAX_BYTES) )
        return 0;

#if (UCOS_EN     == DEF_ENABLED)
    /***********************************************
    * 描述： OS接口
    */
    EEP_WaitEvent();                            // 等信号量
#endif

    uint8_t    size    = 0;

    /***********************************************
    * 描述： 非页首开始，读取之前数据进行填充
    */
    if ( ( suba % AT_PAGE_SIZE ) != 0 ) {
        uint8_t  readLen;
        uint16_t  pageAddr;

        size        = AT_PAGE_SIZE - ( suba % AT_PAGE_SIZE );
        readLen     = AT_PAGE_SIZE - size;
        pageAddr    = suba - readLen;
        BSP_EEP_ReadPage(pageAddr, buf, AT_PAGE_SIZE);

        if ( size > len )
            size = len;
        memcpy(&buf[readLen],pbuf,size);

        BSP_EEP_WritePage( pageAddr , buf , AT_PAGE_SIZE );

        suba    += size;
        pbuf    += size;
        len     -= size;
        bytes   += size;
    }

    /***********************************************
    * 描述： 页首开始，整页
    */
    size = len / AT_PAGE_SIZE;
    while ( size > 0 ) {
        BSP_EEP_WritePage( suba , pbuf, AT_PAGE_SIZE );
        suba    += AT_PAGE_SIZE;
        pbuf    += AT_PAGE_SIZE;
        len     -= AT_PAGE_SIZE;
        bytes   += AT_PAGE_SIZE;
        size    -= 1;
    }

    /***********************************************
    * 描述： 页首开始，不足一页
    */
    if ( len > 0 ) {
        BSP_EEP_WritePage( suba , pbuf , len );
        bytes   += len;
    }

#if (UCOS_EN     == DEF_ENABLED)
    /***********************************************
    * 描述： OS接口
    */
    EEP_SendEvent();                            // 释放信号量
#endif

    return(bytes);
}


/*******************************************************************************
* 名    称： BSP_EEP_ReadBytes
* 功    能： 读多个字节
* 入口参数： suba： EEPROM地址  pbuf ：读出字节的数组 len : 数组的大小
* 出口参数： 0（操作有误），1（操作成功）
* 作　 　者： 无名沈
* 创建日期： 2015-08-18
* 修    改：
* 修改日期：
* 备    注：
*******************************************************************************/
uint16_t BSP_EEP_ReadBytes(uint16_t suba,uint8_t *buf, uint16_t len)
{
    uint16_t      bytes   = 0;
    uint8_t      ack;
    /***********************************************
    * 描述： 检测是否超出存储容量
    */
    if ( (suba + len) > (AT_MAX_BYTES) )
        return 0;
#if (UCOS_EN     == DEF_ENABLED)
    /***********************************************
    * 描述： OS接口
    */
    EEP_WaitEvent();                            // 等信号量
#endif

    /***********************************************
    * 描述： 检测是否可操作
    */
    if ( AT24CXX_Enabled() == FALSE )
        goto exit;
    
#if (UCOS_EN     == DEF_ENABLED)
#if OS_VERSION > 30000U
    BSP_OS_TimeDly(DLY_WR_STO);
#else
    OSTimeDly(DLY_WR_STO);
#endif
#else
    Delay_Nms(DLY_WR_STO);                          // 上一个停止和下一个起始之间要最多延时10ms
#endif
    /***********************************************
    * 描述： 检测是否到最后一字节
    */
    I2C_Start();                                    // 发送起始条件
    ack = BSP_EEP_SetAddr(suba,1);                  // 设置读地址

    if(!ack) {
        //I2C_Stop();                                 // 发送停止条件
        bytes   = 0;
        goto exit;
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
            bytes++;
            break;
        } else {
            ack = I2C_RecvByte(I2C_ACK, buf++);
        }
        /***********************************************
        * 描述： 检测应答信号是否正确
        */
        if(!ack) {
            break;
        }
    }
exit:
    I2C_Stop();                   // 发送停止条件
#if (UCOS_EN     == DEF_ENABLED)
    /***********************************************
    * 描述： OS接口
    */
    EEP_SendEvent();                            // 释放信号量
#endif

    return bytes;
}


/*******************************************************************************
* 名    称： BSP_EEP_Writeuint16_t
* 功    能： 写入一个16位的数（占两个字节）
* 入口参数： suba： EEPROM地址   dat：16的数
* 出口参数： 0（操作有误），1（操作成功）
* 作　 　者： 无名沈
* 创建日期： 2015-08-18
* 修    改：
* 修改日期：
* 备    注：
*******************************************************************************/
uint8_t BSP_EEP_WriteINT16U( uint16_t suba, uint16_t dat )
{
    uint8_t       bytes   = 0;
    uint8_t      *pbuf   = (uint8_t *)&dat;

    if ( suba > (AT_MAX_BYTES-1u-2u) )
        return 0;


    /***********************************************
    * 描述：单字节写入，先写高字节
    */
    for( bytes = 0; bytes < 2; bytes++ ) {
        if( !BSP_EEP_WriteByteChk( suba++, *pbuf++ ) ) {
            break;
        }
    }

    return(bytes);
}

/*******************************************************************************
* 名    称： BSP_EEP_Readuint16_t
* 功    能： 读一个16位的数（占两个字节）
* 入口参数： suba： EEPROM地址   dat：16的数
* 出口参数： 0（操作有误），1（操作成功）
* 作　 　者： 无名沈
* 创建日期： 2015-08-18
* 修    改：
* 修改日期：
* 备    注：
*******************************************************************************/
uint8_t BSP_EEP_ReadINT16U( uint16_t suba,uint16_t *dat)
{
    uint8_t       bytes   = 0;
    uint8_t      *pbuf   = (uint8_t *)dat;

    if ( suba > (AT_MAX_BYTES-1u-2u) )
        return 0;

    memset(buf,0x00,2);

    /***********************************************
    * 描述：单字节读取，先读高字节
    */
    for( bytes = 0; bytes < 2; bytes++ ) {
        if( !BSP_EEP_ReadByte( suba++, pbuf++ ) ) {
            break;
        }
    }

    return bytes;
}

/*******************************************************************************
* 名    称： BSP_EEP_Writeuint32_t
* 功    能： 写入一个32位的数（占四个字节）
* 入口参数： suba： EEPROM地址   dat：32位的数
* 出口参数： 0（操作有误），1（操作成功）
* 作　 　者： 无名沈
* 创建日期： 2015-08-18
* 修    改：
* 修改日期：
* 备    注：
*******************************************************************************/
uint8_t BSP_EEP_WriteINT32U( uint16_t suba, uint32_t dat )
{
    uint8_t       bytes;
    uint8_t      *pbuf   = (uint8_t *)&dat;

    if ( suba > (AT_MAX_BYTES-1u-4u) )
        return 0;

    /***********************************************
    * 描述：单字节写入，先写高字节
    */
    for( bytes = 0; bytes < 4; bytes++ ) {
        if( !BSP_EEP_WriteByteChk( suba++, *pbuf++ ) ) {
            break;
        }
    }

    return(bytes);
}

/*******************************************************************************
* 名    称： BSP_EEP_ReadINT32U
* 功    能： 读一个32位的数（占四个字节）
* 入口参数： suba： EEPROM地址   dat：32位的数
* 出口参数： 0（操作有误），1（操作成功）
* 作　 　者： 无名沈
* 创建日期： 2015-08-18
* 修    改：
* 修改日期：
* 备    注：
*******************************************************************************/
uint8_t BSP_EEP_ReadINT32U( uint16_t suba,uint32_t *dat)
{
    uint8_t       bytes   = 0;
    uint8_t      *pbuf   = (uint8_t *)dat;

    if ( suba > (AT_MAX_BYTES-1u-4u) )
        return 0;

    memset(buf,0x00,4);

    /***********************************************
    * 描述：单字节写入，先写高字节
    */
    for( bytes = 0; bytes < 4; bytes++ ) {
        if( !BSP_EEP_ReadByte( suba++, pbuf++ ) ) {
            break;
        }
    }

    return bytes;
}
/*******************************************************************************
* 名    称： BSP_I2CSetPort
* 功    能： 端口选择
* 入口参数： 无
* 出口参数： 无
* 作　 　者： 无名沈
* 创建日期： 2015-08-18
* 修    改：
* 修改日期：
* 备    注： 外部调用此函数初始化EEPROM
*******************************************************************************/
void BSP_I2CSetPort(INT08U port)
{
    if ( port == 2 ) {        
        AT24CXX_SCL_PIN         = GPIO_Pin_10;
        AT24CXX_SDA_PIN         = GPIO_Pin_11;
    }else {        
        AT24CXX_SCL_PIN         = GPIO_Pin_6;
        AT24CXX_SDA_PIN         = GPIO_Pin_7;
    }  
}

/*******************************************************************************
* 名    称： BSP_EEP_Init
* 功    能： EEPROM初始化
* 入口参数： 无
* 出口参数： 无
* 作　 　者： 无名沈
* 创建日期： 2015-08-18
* 修    改：
* 修改日期：
* 备    注： 外部调用此函数初始化EEPROM
*******************************************************************************/
BOOL BSP_EEP_Init(void)
{
    GPIO_InitTypeDef    gpio_init;
    uint8_t              dat;

    /***********************************************
    * 描述： I2C1
    */
#if GPIO_I2C1_PORT_EN == DEF_ENABLED
    RCC_APB2PeriphClockCmd(I2C1_SCL_RCC , ENABLE);
    gpio_init.GPIO_Pin   = I2C1_SCL_PIN;
    gpio_init.GPIO_Speed = GPIO_Speed_2MHz;
    gpio_init.GPIO_Mode  = GPIO_Mode_Out_OD;
    
    GPIO_Init(I2C1_SCL_PORT, &gpio_init);
    RCC_APB2PeriphClockCmd(I2C1_SDA_RCC , ENABLE);
    gpio_init.GPIO_Pin   = I2C1_SDA_PIN;
    GPIO_Init(I2C1_SDA_PORT, &gpio_init);
#endif
    /***********************************************
    * 描述： I2C2
    */
#if GPIO_I2C2_PORT_EN == DEF_ENABLED
    RCC_APB2PeriphClockCmd(I2C2_SCL_RCC , ENABLE);
    gpio_init.GPIO_Pin   = I2C2_SCL_PIN;
    gpio_init.GPIO_Speed = GPIO_Speed_2MHz;
    gpio_init.GPIO_Mode  = GPIO_Mode_Out_OD;
    
    GPIO_Init(I2C2_SCL_PORT, &gpio_init);
    RCC_APB2PeriphClockCmd(I2C2_SDA_RCC , ENABLE);
    gpio_init.GPIO_Pin   = I2C2_SDA_PIN;
    GPIO_Init(I2C2_SDA_PORT, &gpio_init);
#endif
    
#if GPIO_I2C1_PORT_EN == DEF_ENABLED
    BSP_I2CSetPort(1);                              // 切换到I2C1
    AT24CXX_Init();
#endif
#if GPIO_I2C2_PORT_EN == DEF_ENABLED
    BSP_I2CSetPort(2);                              // 切换到I2C1
    AT24CXX_Init();
#endif
    BOOL ack    = BSP_EEP_ReadByte( 0, &dat );
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
    return ack;
}

/*******************************************************************************
* Function Name  :
* Description    :
* Input          :
* Output         :
* Return         : None
********************************************************************************/
#define     TEST_BUF_LEN    1024
INT32U      idx;
CHAR        atBuf[TEST_BUF_LEN];
CHAR        chkBuf[TEST_BUF_LEN];
void BSP_EEP_Main(void)
{

    uint16_t      baseAddr        = 100;
    idx         = 0;

    BSP_EEP_Init();                                 // 初始化存储器

    memset(atBuf,0x00,TEST_BUF_LEN);
    memset(chkBuf,0x00,TEST_BUF_LEN);

    for(idx = 0; idx < TEST_BUF_LEN; idx++ ) {
      if ( idx % 2 )
        atBuf[idx]  = 0xAA;
      else
        atBuf[idx]  = idx / 2;
    }

    while (1) {
        memset(chkBuf,0x00,TEST_BUF_LEN);
        BSP_EEP_ReadBytes(baseAddr,(uint8_t*)&chkBuf[0],TEST_BUF_LEN);

        for (  idx = 0; idx < TEST_BUF_LEN; idx++ ){
          if ( atBuf[idx] != chkBuf[idx] ) {
            BSP_EEP_WriteBytes  (baseAddr,(uint8_t*)&atBuf[0],TEST_BUF_LEN);
            break;
          }
        }
        if ( idx == TEST_BUF_LEN )
          break;
        Delay_Nms(1000);
        BSP_LED_Toggle(2);
    }
}
/*******************************************************************************
 *              end of file                                                    *
 *******************************************************************************/
#endif
