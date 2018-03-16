/*******************************************************************************
 *   Filename:      bsp_shtxx.c
 *   Revised:       $Date: 2015-01-26
 *   Revision:      $
 *   Writer:        Wuming Shen.
 *
 *   Description:   SHTxx读写，温度相对湿度读取
 *
 *   Notes:
 *
 *
 *   All copyrights reserved to Wuming Shen.
 *
 *******************************************************************************/

/*******************************************************************************
 * INCLUDES
 */
#include "bsp_shtxx.h"
#include "math.h"
#include "global.h"

/*******************************************************************************
 * CONSTANTS
 */
/***********************************************
* 描述： SHTXX寄存器
*/
#define SHTCMD_WRITE_STATUS     0x06            // "写状态寄存器"命令
#define SHTCMD_READ_STATUES     0x07            // "读状态寄存器"命令
#define SHTCMD_MEASURE_TEMP     0x03            // "测量温度"命令
#define SHTCMD_MEASURE_HUMI     0x05            // "测量相对湿度"命令
#define SHTCMD_RESET            0x1e            // "复位"命令
/***********************************************
* 描述： 时间（us）
*/
#define DLY_CLK_SDA             1               // >=1.0us
#define DLY_CLK_LOW             1               // >=1.3us
#define DLY_CLK_HIGH            1               // >=0.6us
#define DLY_SU_STA              1               // >=0.6us
#define DLY_HD_STA              1               // >=0.6us
#define DLY_SU_STO              1               // >=0.6us
#define DLY_SDA_BUF             1               // >=1.3us
/***********************************************
* 描述： 时间（us）
*/
#define DLY_WR_STO              60              // >61us
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

#define I2C_MAX_BYTES            16
/*******************************************************************************
 * MACROS
 */
/***********************************************
* 描述： AT24CXX使用的IO，配置为开漏输出
*/
#define I2C_SCL_RCC             RCC_APB2Periph_GPIOB
#define I2C_SCL_GPIO            GPIOB
#define I2C_SCL_PIN             GPIO_Pin_8

#define I2C_SDA_RCC             RCC_APB2Periph_GPIOB
#define I2C_SDA_GPIO            GPIOB
#define I2C_SDA_PIN             GPIO_Pin_9

#define SET_SCL_H()             (I2C_SCL_GPIO->BSRR = I2C_SCL_PIN)
#define SET_SCL_L()             (I2C_SCL_GPIO->BRR  = I2C_SCL_PIN)

#define SET_SDA_H()             (I2C_SDA_GPIO->BSRR = I2C_SDA_PIN)
#define SET_SDA_L()             (I2C_SDA_GPIO->BRR  = I2C_SDA_PIN)

#define GET_SCL()               (I2C_SCL_GPIO->IDR  & I2C_SCL_PIN)
#define GET_SDA()               (I2C_SDA_GPIO->IDR  & I2C_SDA_PIN)

#define I2C_SET_SCL(sta)        { if(sta) {SET_SCL_H(); SHT_Delay_Nus(DLY_CLK_HIGH);} else { SET_SCL_L(); SHT_Delay_Nus(DLY_CLK_SDA); } }
#define I2C_SET_SDA(sta)        { if(sta) {SET_SDA_H(); SHT_Delay_Nus(DLY_CLK_LOW); } else { SET_SDA_L(); SHT_Delay_Nus(DLY_CLK_LOW); } }
#define I2C_GET_SCL(sta)        { if( GET_SCL() ) sta |= 1; else sta &= ~1; }
#define I2C_GET_SDA(sta)        { if( GET_SDA() ) sta |= 1; else sta &= ~1; }

//#define SET_SDA_INPUT()
//#define SET_SDA_OUTPUT()
//#define SET_SCL_INPUT()
//#define SET_SCL_OUTPUT()

void SET_SDA_INPUT(void)
{
//	GPIO_InitTypeDef GPIO_InitStructure;
//
//    GPIO_InitStructure.GPIO_Pin   = I2C_SDA_PIN;
//    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
//    GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_IPU;
//    GPIO_Init(I2C_SDA_GPIO, &GPIO_InitStructure);
}

void SET_SDA_OUTPUT(void)
{
//	GPIO_InitTypeDef GPIO_InitStructure;
//
//    GPIO_InitStructure.GPIO_Pin   = I2C_SDA_PIN;
//    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
//    GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_Out_PP;
//    GPIO_Init(I2C_SDA_GPIO, &GPIO_InitStructure);
}

void SET_SCL_OUTPUT(void)
{
//	GPIO_InitTypeDef GPIO_InitStructure;
//
//    GPIO_InitStructure.GPIO_Pin   = I2C_SCL_PIN;
//    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
//    GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_Out_PP;
//    GPIO_Init(I2C_SCL_GPIO, &GPIO_InitStructure);
}

/*******************************************************************************
 * TYPEDEFS
 */

/*******************************************************************************
 * LOCAL VARIABLES
 */
/***********************************************
* 描述： 设备类型 (SHT1x/SHT2x)
*/
#define DEV_SHT1X           0
#define DEV_SHT2X           1

static  uint8            DevType    = DEV_SHT1X;
/*******************************************************************************
 * GLOBAL VARIABLES
 */
SHT_DATA_TYPE               SHT_Data;           //转换数据保存在该变量中

/*******************************************************************************
 * LOCAL FUNCTIONS
 */
void            SHTBoot                 (void);
void            SHTXX_Init              (void);
void            SHTXX_PIN_Init          (void);
void            SHT_Delay_Nus           (uint32  dly);
int8_t          sht_write_byte          (uint8_t value);
int8_t          sht_read_byte           (uint8_t ack);
void            sht_transstart          (void);
void            sht_connectionreset     (void);
int8_t          sht_softreset           (void);
int8_t          sht_read_statusreg      (uint8_t *p_value, uint8_t *p_checksum);
int8_t          sht_write_statusreg     (uint8_t *p_value);
int8_t          sht_measure             (u16 *p_value, int8_t *p_checksum, uint8_t mode, uint8_t step);
void            calc_shtxx              (float *p_humidity ,float *p_temperature);
float           calc_dewpoint           (float h,float t);
uint16          SHT_MeasureOnce         (uint8_t mode , uint8_t step);

/*******************************************************************************
 * GLOBAL FUNCTIONS
 */
static void     I2C_Start               (void);
static void     I2C_Stop                (void);

static uint8    I2C_SendByte            (uint8  byte);                           //数据从高位到低位
static uint8    I2C_RecvByte            (uint8  ack, uint8 *dat);              //数据从高位到低位

/*******************************************************************************
 * EXTERN VARIABLES
 */

/*******************************************************************************
 * EXTERN FUNCTIONS
 */
/*******************************************************************************
* 名    称： Delay_Nus()
* 功    能： 延时大概1us
* 入口参数： dly		延时参数，值越大，延时越久
* 出口参数： 无
* 作　 　者： 无名沈
* 创建日期： 2009-01-03
* 修    改：
* 修改日期：
*******************************************************************************/
OPTIMIZE_NONE void  SHT_Delay_Nus( uint32  dly )
{
    INT08U  i;

    while(dly--) {
        for(i=0; i<2; i++);             // clk = 72000000 1027.7778611111111111111111111111
    }
}

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
* Function Name  : I2C_Start
* Description    : 启动I2C总线，即发送I2C初始条件
*                ：开始时序
*                : SCL ˉˉˉˉˉˉ\____
*                : SDA ˉˉˉˉ\______
*                :       |   |
*                :       START
* Input          : None
* Output         : None
* Return         : None
********************************************************************************/
static void I2C_Start(void)
{
    SET_SDA_OUTPUT();				            /*SDA设为输出                       */
    SET_SDA_H();				                /*发送起始条件的数据信号            */
    SHT_Delay_Nus(DLY_CLK_SDA);
    SET_SCL_H();
    SHT_Delay_Nus(DLY_SU_STA);		            /*起始条件建立时间大于4.7us,延时    */
    SET_SDA_L();					            /*发送起始信号                      */
    SHT_Delay_Nus(DLY_HD_STA);			        /*起始条件保持时间大于4μs           */
    SET_SCL_L();					            /*允许数据线改变，准备发送或接收数据*/
    SHT_Delay_Nus(DLY_CLK_LOW);                 /*CLK与SDA之间延时                  */
}

/*******************************************************************************
* Function Name  : I2C_Stop
* Description    : 结束I2C总线，即发送I2C结束条件
*                ：停止时序
*                : SCL _____/ˉˉˉˉˉˉˉ
*                : SDA _________/ˉˉˉˉˉ
*                :            |   |
*                :            STOP
* Input          : None
* Output         : None
* Return         : None
********************************************************************************/
static void I2C_Stop(void)
{
	SET_SDA_OUTPUT();				            /*SDA设为输出*/
    SET_SCL_L(); 					            /*钳住I2C总线，准备发送或接收数据 */
	SHT_Delay_Nus(DLY_CLK_SDA);
    SET_SDA_L(); 					            /*发送结束条件的数据信号*/
    SHT_Delay_Nus(DLY_CLK_SDA);
    SET_SCL_H(); 					            /*发送结束条件的时钟信号*/
	SHT_Delay_Nus(DLY_SU_STO);			            /*结束条件建立时间大于4μs*/
    SET_SDA_H(); 					            /*发送I2C总线结束信号*/
    SHT_Delay_Nus(DLY_SDA_BUF); 		            /*>4us*/
}

/*******************************************************************************
* Function Name  : I2C_SendByte(uint8 byte)
* Description    : 字节数据传送函数，将数据 c 发送出去，可以是地址，也可以是数据，
                   发完后等待应答，并对此状态位进行操作
*                ：数据时序
*                : SCL ______/ˉˉˉˉˉˉ\___/ˉˉˉˉˉˉˉˉ\______
*                : SDA _____/ˉˉˉˉˉˉˉˉˉXˉˉˉˉˉˉˉˉˉˉˉ\
*                :     ˉˉˉˉˉˉˉˉˉˉˉˉˉˉˉˉˉˉˉˉˉˉˉˉˉˉˉˉˉˉˉˉˉ
*                ：      |数据保持  |   |数据保持|
*                ：                数据改变
* Input          : uint8 型数据
* Output         : None
* Return         : None
********************************************************************************/
static uint8 I2C_SendByte(uint8 byte)
{
    int16  ctr     = 0;                            // 初始化数据位计数器
    uint8  ack     = 0;

    /***********************************************
    * 描述：数据从高位到低位
    */
    for ( ctr = 0 ; ctr < 8 ; ctr ++ ) {            // 检测是否已经发送完毕
        I2C_SET_SCL(LOW);                           // SCL = 0 允许数据线改变，准备发送数据
        I2C_SET_SDA((byte & 0x80) == 0x80);         // SDA = 0 发送数据：0;SDA = 1 发送数据：1
        I2C_SET_SCL(HIGH);                          // SCL = 1 锁定数据，从机可以读取
        byte <<= 1;                                 // 当前值左移一位
    };
    /***********************************************
    * 描述：检查应答
    */
    I2C_SET_SCL(LOW);                               // SCL = 0 允许数据线改变，准备接收应答位
    SET_SDA_INPUT();                                // SDA设为输入
    I2C_SET_SDA(HIGH);                              // SDA = 1 主机释放总线

    I2C_SET_SCL(HIGH);                              // SCL = 1 锁定数据
    I2C_GET_SDA(ack);                               // 读取从机应答ACK
    SET_SDA_OUTPUT();                               // SDA设为输出
    I2C_SET_SCL(LOW);                               // SCL = 0 允许数据线改变，准备接收应答位

    if (ack)                                    	// 检测应答信号
        return (FALSE);                             // NACK返回
    else
        return (TRUE);                              // ACK返回
}

/*******************************************************************************
* Function Name  : I2C_RecvByte(void)
* Description    : 接收从器件传来的数据，并判断总线错误（不发应答信号），收完后
                   需要调用应答函数。
*                ：数据时序
*                : SDA _____/ˉˉˉˉˉˉˉˉˉXˉˉˉˉˉˉˉˉˉˉˉ\
*                :     ˉˉˉˉˉˉˉˉˉˉˉˉˉˉˉˉˉˉˉˉˉˉˉˉˉˉˉˉˉˉˉˉˉ
*                : SCL ______/ˉˉˉˉˉˉ\___/ˉˉˉˉˉˉˉˉ\______
*                ：      |数据保持  |   |数据保持|
*                ：                数据改变
* Input          : None
* Output         : uint8 型数据
* Return         : None
********************************************************************************/
static uint8 I2C_RecvByte(uint8 ack, uint8 *dat)  	//数据从高位到低位
{
    uint8  ctr     = 0;
    int16  data    = 0;

    SET_SDA_INPUT();                                // SDA设为输入
    I2C_SET_SCL(LOW);                               // SCL = 0 允许数据线改变，准备读取从机数据
    /***********************************************
    * 描述：接收数据
    */
    for ( ctr = 0 ; ctr < 8 ; ctr ++ ) {            // 数据左移一位，由于第一个数据是0所以左移还是0
        data <<= 1;                                 // 准备下一位数据
        I2C_SET_SCL(HIGH);                          // SCL = 1 锁定数据，可以读取从机数据
        I2C_GET_SDA(data);                          // 如果为1则置1，否则不改变（默认为0）
        I2C_SET_SCL(LOW);                           // SCL = 0 允许数据线改变，准备读取从机数据
    };

    SET_SDA_OUTPUT();                               // SDA设为输出
    /***********************************************
    * 描述：检查应答
    */
    I2C_SET_SDA(!ack);                              // ACK,发送0

    I2C_SET_SCL(LOW);
    I2C_SET_SCL(HIGH);
    I2C_SET_SDA(LOW);
    I2C_SET_SDA(HIGH);

    *dat    = data;

    return (TRUE);                            	    // Return 8-bit data byte
}

/*******************************************************************************
* Function Name  : BSP_I2C_SetCmd
* Description    : 设置当前地址
* Input          : suba - 地址； opt - 0:写； 1:读
* Output         :
* Return         : None
********************************************************************************/
static uint8 BSP_I2C_SetCmd(uint8 cmd, uint8 opt)
{
    uint8      ack;
    /***********************************************
    * 描述： 如果为读，则发送读地址
    */
    if ( opt ) {
        ack = I2C_SendByte(0x80+1);
    } else {
        ack = I2C_SendByte(0x80);                           // 发送设备地址
    }

    if(!ack)    goto exit;                                  // 转至结束

    ack = I2C_SendByte(cmd);                                // 发送内存低地址
exit:
    return ack;
}

/*******************************************************************************
* Function Name  : uint8 BSP_I2C_WriteByte(uint16 suba,uint8 c)
* Description    : 从启动总线到发送地址、数据，结束总线的全过程，如果返回1，表示
                   操作成功，否则操作有误。
* Input          : 从器件地址 sla，子地址 suba, 发送字节 c
* Output         : 0（操作有误），1（操作成功）
* Return         : None
********************************************************************************/
uint8 BSP_I2C_WriteByte(uint8 suba,uint8 byte )
{
    uint8   ack;

    /***********************************************
    * 描述： OS接口
    */
    SHT_Delay_Nus(DLY_WR_STO);                              // 上一个停止和下一个起始之间要最多延时10ms

    I2C_Start();                                            // 发送起始条件

    ack = BSP_I2C_SetCmd(byte,0);                           // 设置读地址
    if(!ack)    goto exit;                                  // 转至结束

    //ack = I2C_SendByte(byte);                             // 发送要写入的内容
    //if(!ack)    goto exit;                                // 转至结束

exit:
    I2C_Stop();                                             // 发送停止条件

    return(ack);
}
/*******************************************************************************
* Function Name  : uint8 BSP_EEP_WriteBytes
* Description    : 从启动总线到发送地址、数据，结束总线的全过程，如果返回1，表示
                   操作成功，否则操作有误。
* Input          : 从器件地址 sla，子地址 suba, 发送字节 c
* Output         : 0（操作有误），1（操作成功）
* Return         : None
********************************************************************************/
uint16 BSP_I2C_WriteBytes( uint8 suba, uint8 *pbuf, uint16 len )
{
    uint8      bytes   = 0;
    uint8      ack;
    /***********************************************
    * 描述： 检测地址是否会溢出，如果溢出则直接返回
    */
    if ( (suba + len) > (256) )
        return 0;

    /***********************************************
    * 描述： OS接口
    */
    //if ( BSP_I2C_GetReady() == FALSE )
    //  return 0;

    SHT_Delay_Nus(DLY_WR_STO);                              // 上一个停止和下一个起始之间要最多延时10ms

    I2C_Start();

    ack = BSP_I2C_SetCmd(suba,0);                           // 设置读地址
    if(!ack)    goto exit;                                  // 转至结束

    for ( ; bytes < len ; bytes++ ) {
        ack = I2C_SendByte( pbuf[ bytes ] );
        if(!ack)    goto exit;                              // 转至结束
    }
exit:
    I2C_Stop();
    if ( len == 0 )
        return ack;
    /***********************************************
    * 描述： OS接口
    */

    return bytes;
}

/*******************************************************************************
* Function Name  : BSP_EEP_ReadByte
* Description    : 读N字节
* Input          :
* Output         :
* Return         : None
********************************************************************************/
uint8 BSP_I2C_ReadByte( uint8 suba, uint8 *byte)
{
    uint8       ack;
    /***********************************************
    * 描述： OS接口
    */
    SHT_Delay_Nus(DLY_WR_STO);                              // 上一个停止和下一个起始之间要最多延时10ms

    I2C_Start();                                            // 发送起始条件

    ack = BSP_I2C_SetCmd(suba,1);                           // 设置读地址
    if(!ack)    goto exit;                                  // 转至结束

    ack = I2C_RecvByte(I2C_NACK,byte);                      // 接收内容并发关非应答
exit:
    I2C_Stop();                                             // 发送停止条件

    return ack;
}

/*******************************************************************************
* Function Name  :BSP_EEP_ReadBytes
* Description    :读N字节
* Input          :
* Output         :
* Return         : None
********************************************************************************/
uint16 BSP_I2C_ReadBytes(uint8 suba,uint8 *buf, uint16 len)
{
    uint16      bytes   = 0;
    uint8      ack;
    /***********************************************
    * 描述： 检测是否超出存储容量
    */
    if ( (suba + len) > (I2C_MAX_BYTES) )
        return 0;
    /***********************************************
    * 描述： OS接口
    */

    /***********************************************
    * 描述： 检测是否可操作
    */
    //if ( BSP_I2C_GetReady() == FALSE )
    //    return 0;

    SHT_Delay_Nus(DLY_WR_STO);                          // 上一个停止和下一个起始之间要最多延时10ms
    /***********************************************
    * 描述： 检测是否到最后一字节
    */
    I2C_Start();                                        // 发送起始条件
    ack = BSP_I2C_SetCmd(suba,1);                       // 设置读地址

    if(!ack) {
        I2C_Stop();                                     // 发送停止条件
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
            //I2C_Stop();                               // 发送停止条件
            break;
        }
    }
exit:
    I2C_Stop();                                         // 发送停止条件
    /***********************************************
    * 描述： OS接口
    */

    return bytes;
}

/*******************************************************************************
 * 函数名称：sht_write_byte
 *
 * 功能描述：向SHT11写1个字节并检测SHT11是否应答
 *
 * 参    数：value  要写入的1字节数据
 *
 * 返 回 值：error=1表明SHT11未应答
 ******************************************************************************/
int8_t sht_write_byte(uint8_t byte)
{
    int16  ctr     = 0;                             // 初始化数据位计数器
    uint8  ack     = 0;

    SET_SDA_OUTPUT();                               // DATA设置为输出

    /***********************************************
    * 描述：从高到低逐位发送
    */
    for ( ctr = 0 ; ctr < 8 ; ctr ++ ) {            // 检测是否已经发送完毕
        I2C_SET_SDA((byte & 0x80) == 0x80);         // SDA = 0 发送数据：0;SDA = 1 发送数据：1
        I2C_SET_SCL(HIGH);                          // SCL = 0 允许数据线改变，准备发送数据
        //SHT_Delay_Nus(5);
        I2C_SET_SCL(LOW);                           // SCL = 1 锁定数据，从机可以读取
        //SHT_Delay_Nus(5);
        byte <<= 1;                                 // 当前值左移一位
    };

    I2C_SET_SDA(HIGH);                              // 释放DATA线
    I2C_SET_SCL(HIGH);                              // 第9个SCK
    //SHT_Delay_Nus(5);
    SET_SDA_INPUT();
    I2C_GET_SDA(ack);
    //SHT_Delay_Nus(5);
    I2C_SET_SCL(LOW);

    return ack;                                     // error=1表明SHT11未应答
}

/*******************************************************************************
 * 函数名称：sht_read_byte
 *
 * 功能描述：从SHT11读1个字节并当输入参数ack=1时给出应答
 *
 * 参    数：ack  应答标志
 *
 * 返 回 值：error=1表明SHT11未应答
 ******************************************************************************/
int8_t sht_read_byte(uint8_t ack)
{
    uint8  ctr     = 0;
    int16  data    = 0;

    SET_SDA_OUTPUT();                               // DATA设置为输出
    I2C_SET_SDA(HIGH);                              // 释放DATA线
    SET_SDA_INPUT();

    /***********************************************
    * 描述：从高到低逐位读取
    */
    for ( ctr = 0 ; ctr < 8 ; ctr ++ ) {            // 数据左移一位，由于第一个数据是0所以左移还是0
        data <<= 1;                                 // 准备下一位数据
        I2C_SET_SCL(HIGH);                          // SCL = 1 锁定数据，可以读取从机数据
        I2C_GET_SDA(data);                          // 如果为1则置1，否则不改变（默认为0）
        I2C_SET_SCL(LOW);                           // SCL = 0 允许数据线改变，准备读取从机数据
    };

    SET_SDA_OUTPUT();                               // DATA设置为输出

    I2C_SET_SDA(!ack);                              // ACK,发送0
    I2C_SET_SCL(HIGH);                              // 第9个SCK
    //SHT_Delay_Nus(5);
    I2C_SET_SCL(LOW);
    I2C_SET_SDA(HIGH);                              // 释放DATA线

    return data;
}

/*******************************************************************************
 * 函数名称：sht_transstart
 *
 * 功能描述：发送一个"启动传输"序列
 *                 ____         ________
 *           SDA:      |_______|
 *                    ___     ___
 *           SCK : __|   |___|   |______
 *
 * 参    数：无
 *
 * 返 回 值：无
 ******************************************************************************/
void sht_transstart(void)
{
   SET_SDA_OUTPUT();                                        // DATA设置为输出

   I2C_SET_SDA(HIGH);
   I2C_SET_SCL(LOW);                                        // 初始状态
   //SHT_Delay_Nus(10);

   I2C_SET_SCL(HIGH);
   //SHT_Delay_Nus(10);
   I2C_SET_SDA(LOW);
   //SHT_Delay_Nus(10);

   I2C_SET_SCL(LOW);
   //SHT_Delay_Nus(10);
   I2C_SET_SCL(HIGH);
   //SHT_Delay_Nus(10);

   I2C_SET_SDA(HIGH);
   //SHT_Delay_Nus(10);
   I2C_SET_SCL(LOW);
   //SHT_Delay_Nus(10);

   I2C_SET_SDA(LOW);
   //SHT_Delay_Nus(20);
}

/*******************************************************************************
 * 函数名称：sht_connectionreset
 *
 * 功能描述：通信复位
 *                 _____________________________________________________         ________
 *           DATA:                                                      |_______|
 *                    _    _    _    _    _    _    _    _    _        ___     ___
 *           SCK : __| |__| |__| |__| |__| |__| |__| |__| |__| |______|   |___|   |______
 *
 * 参    数：无
 *
 * 返 回 值：无
 ******************************************************************************/
void sht_connectionreset(void)
{
    uint8_t i;

    SET_SDA_OUTPUT();                                       // DATA设置为输出

    I2C_SET_SDA(HIGH);
    I2C_SET_SCL(LOW);                                       // 初始状态

    /* 9个SCK 周期*/
    for(i=0;i<9;i++) {
        I2C_SET_SCL(HIGH);
        //SHT_Delay_Nus(10);
        I2C_SET_SCL(LOW);
        //SHT_Delay_Nus(10);
    }
}

/*******************************************************************************
* 名    称： sht_softreset
* 功    能： 软件复位
* 入口参数：
* 出口参数： 返回值为1表示SHT11未响应
* 作　 　者：
* 创建日期： 2015-08-27
* 修    改：
* 修改日期：
* 备    注：
*******************************************************************************/
int8_t sht_softreset(void)
{
    uint8_t error = 0xfe;
    for ( int8_t i = 0; i < 5; i++ ) {
        if ( 1 == BSP_I2C_WriteByte(0x80, error) ) {
            DevType    = DEV_SHT2X;
            //SHT_Delay_Nus(1000*10);
            return 0;                                       // error=1表示SHT11未响应
        }
        SHT_Delay_Nus(1000*20);
    }
    DevType    = DEV_SHT1X;
    sht_connectionreset();                                  // 通信复位
    error   = 0;
    error  += sht_write_byte(SHTCMD_RESET);                 // 发送"复位"命令给SHT11
    return error;                                           // error=1表示SHT11未响应
}

/*******************************************************************************
 * 函数名称：sht_read_statusreg
 *
 * 功能描述：读取状态寄存器和校验和
 *
 * 参    数：p_value      状态寄存器的值
 *           p_checksum   校验和
 *
 * 返 回 值：返回值为1表示SHT11未响应
 ******************************************************************************/
int8_t sht_read_statusreg(uint8_t *p_value, uint8_t *p_checksum)
{
    uint8_t error=0;

    sht_transstart();                                       // 发送一个"启动传输"序列
    error       = sht_write_byte(SHTCMD_READ_STATUES);      // 发送"读状态寄存器"命令
    *p_value    = sht_read_byte(I2C_ACK);                   // 读状态寄存器
    *p_checksum = sht_read_byte(I2C_NACK);                  // 读校验和

    return error;                                           // error=1表示SHT11未响应
}

/*******************************************************************************
* 名    称： sht_write_statusreg
* 功    能： 写状态寄存器
* 入口参数： p_value      状态寄存器的值
* 出口参数： 返回值为1表示SHT11未响应
* 作　 　者：
* 创建日期： 2015-08-27
* 修    改：
* 修改日期：
* 备    注：
*******************************************************************************/
int8_t sht_write_statusreg(uint8_t *p_value)
{
    uint8_t error=0;

    sht_transstart();                                       // 发送一个"启动传输"序列
    error+=sht_write_byte(SHTCMD_WRITE_STATUS);             // 发送"写状态寄存器"命令
    error+=sht_write_byte(*p_value);                        // 写状态寄存器

    return error;                                           // error=1表示SHT11未响应
}

/*******************************************************************************
* 名    称： sht_measure
* 功    能： 进行一次测量(相对湿度或温度)
* 入口参数： p_value      测量值
*            checksum     校验和
*            mode         TEMP表示进行温度测量
*                         HUMI表示进行相对湿度测量
* 出口参数：
* 作　 　者：
* 创建日期： 2015-08-27
* 修    改：
* 修改日期：
* 备    注：
*******************************************************************************/
int8_t sht_measure(u16 *p_value,  int8_t *p_checksum, uint8_t mode, uint8_t step)
{
    uint8_t error=0,err = 0;

    switch(step)
    {
        case 0:
        sht_transstart();                                   // 发送一个"启动传输"序列
        switch(mode)                                        // 根据输入参数mode进行一次相应的测量
        {
            case TEMP:
            error+=sht_write_byte(SHTCMD_MEASURE_TEMP);
            break;
            case HUMI:
            error+=sht_write_byte(SHTCMD_MEASURE_HUMI);
            break;
            default:
            break;
        } break;
        case 1:
        I2C_GET_SDA(err);
        if(err) {
            error+=1;
            break;
        }                                                   // 测量错误
        /*
        * Note:在此数据读取两个字节，数据存储需考虑大小端格式存放问题
        */
        static u8 buf[2];
        buf[0]         = sht_read_byte(I2C_ACK);            // 读第1个字节 (MSB)
        buf[1]         = sht_read_byte(I2C_ACK);            // 读第2个字节 (LSB)
        *p_checksum     = sht_read_byte(I2C_NACK);          // 读校验和
        *p_value        = BUILD_INT16U(buf[1],buf[0]);
        break;
    }

    return error;
}

/*******************************************************************************
* 名    称： calc_shtxx
* 功    能： 计算相对湿度和温度
* 入口参数： p_humidity      SHT11采集到的相对湿度值(经过本函数的转换，该参数返回实际物理量的值)
*            p_temperature   SHT11采集到的温度值(经过本函数的转换，该参数返回实际物理量的值)
* 出口参数：
* 作　 　者：
* 创建日期： 2015-08-27
* 修    改：
* 修改日期：
* 备    注：
*******************************************************************************/
void calc_shtxx(float *p_humidity ,float *p_temperature)
{
    const float C1  = -4.0;                                 // 12位
    const float C2  = +0.0405;                              // 12 Bit
    const float C3  = -0.0000028;                           // 12 Bit
    const float T1  = +0.01;                                // 14位 5V
    const float T2  = +0.00008;                             // 14位 5V

    float rh        = *p_humidity;                          // 相对湿度采集值 12位
    float t         = *p_temperature;                       // 温度采集值 14位
    float t_C;                                              // 温度物理量值
    float rh_lin;                                           // 相对湿度的非线性补偿
    float rh_true;                                          // 相对湿度物理量值

    /***********************************************
    * 描述：
    */
    if ( DevType == DEV_SHT1X ) {
        t_C             = t * 0.01 - 39.60;                 // 计算温度物理量值
        rh_lin          = C3*rh*rh + C2*rh + C1;            // 计算相对湿度的非线性补偿
    } else {
        t_C             = t * 175.72 / 16384 - 46.85;       // t*0.010725 - 46.85;
        rh_lin          = 125.0 * rh / 4096 - 6;            // 计算相对湿度的非线性补偿
    }

    /***********************************************
    * 描述：
    */
    rh_true         = (t_C-25) * (T1+T2*rh) + rh_lin;       // 计算相对湿度物理量值

    /***********************************************
    * 描述： 若计算出来的相对湿度物理量值超范围则截断
    */
    if(rh_true>100)
        rh_true     = 100;
    if(rh_true<0.1)
        rh_true     = 0.1;
    /***********************************************
    * 描述：
    */
    *p_temperature  = t_C;                                  // 返回温度物理量值
    *p_humidity     = rh_true;                              // 返回相对湿度物理量值
}

/*******************************************************************************
* 名    称： calc_dewpoint
* 功    能： 计算露点
* 入口参数： h      相对湿度物理量值
*            t      温度物理量值
* 出口参数：
* 作　 　者：
* 创建日期： 2015-08-27
* 修    改：
* 修改日期：
* 备    注：
*******************************************************************************/
float calc_dewpoint(float h,float t)
{
    float logEx,dew_point;

    logEx       = 0.66077 + 7.5 * t / (237.3 + t) + (log10(h) - 2);
    dew_point   = (logEx - 0.66077) * 237.3 / (0.66077 + 7.5 - logEx);

    return dew_point;
}

/*******************************************************************************
* 名    称： BSP_ShtMeasureOnce
* 功    能：
* 入口参数：
* 出口参数：
* 作　 　者：
* 创建日期： 2015-08-27
* 修    改：
* 修改日期：
* 备    注：
*******************************************************************************/
uint16 BSP_ShtMeasureOnce(uint8_t mode , uint8_t step)
{
    int8_t                checksum;
    int8_t                error   = 0;
    static s16          humi    = 0;
    static s16          temp    = 0;

    /***********************************************
    * 描述：
    */
    switch(mode) {
    /***********************************************
    * 描述： 采集温度
    */
    case TEMP:
        error += sht_measure((u16*)&temp, &checksum, TEMP, step);
        return error;
    /***********************************************
    * 描述： 采集湿度
    */
    case HUMI:
        error += sht_measure((u16*)&humi, &checksum, HUMI, step);
        return error;
    /***********************************************
    * 描述： 计算值
    */
    default: {
        //sht_read_statusreg(&j,&k);
        /***********************************************
        * 描述： 转换成浮点
        */
        float   ftemp       = (float)temp;
        float   fhumi       = (float)humi;
        /***********************************************
        * 描述： 计算温湿度
        */
        calc_shtxx(&fhumi,&ftemp);
        /***********************************************
        * 描述： 给全局变量
        */
        SHT_Data.Temp       = ftemp;
        SHT_Data.Humi       = fhumi;
        //SHT_Data.DewPoint   = calc_dewpoint(fhumi,ftemp);
        }
        return 0;
    }
}

/*******************************************************************************
* 名    称： SHT_GpioInit
* 功    能：
* 入口参数：
* 出口参数：
* 作　 　者： 无名沈
* 创建日期： 2015-08-27
* 修    改：
* 修改日期：
* 备    注：
*******************************************************************************/
void SHT_GpioInit(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;

    RCC_APB2PeriphClockCmd(I2C_SCL_RCC,ENABLE);
    RCC_APB2PeriphClockCmd(I2C_SDA_RCC,ENABLE);

    GPIO_InitStructure.GPIO_Pin   = I2C_SDA_PIN;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_Out_OD;
    GPIO_Init(I2C_SDA_GPIO, &GPIO_InitStructure);

    GPIO_InitStructure.GPIO_Pin   = I2C_SCL_PIN;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_Out_PP;
    GPIO_Init(I2C_SCL_GPIO, &GPIO_InitStructure);

    SET_SCL_OUTPUT();
    SET_SDA_OUTPUT();

    I2C_SET_SCL(HIGH);
    I2C_SET_SDA(HIGH);
}

/*******************************************************************************
 * 函数名称： BSP_ShtxxInit
 *
 * 功能描述： SHT11初始化操作(包括IO引脚初始化和复位操作)
 *
 * 参    数： NONE
 *
 * 返 回 值： NONE
 ******************************************************************************/
void BSP_ShtxxInit(void)
{
    SHT_GpioInit();                                          //Initialize sht11
    sht_softreset();
}

/*******************************************************************************
*              end of file                                                    *
*******************************************************************************/
