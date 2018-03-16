/*******************************************************************************
*   Filename:       bsp_key.c
*   Revised:        All copyrights reserved to Roger.
*   Date:           2015-08-11
*   Revision:       v1.0
*   Writer:	     Roger-WY.
*
*   Description:    按键驱动模块
*                   可以识别长按，短按，双击，旋转编码器的操作。
*
*
*   Notes:
*
*   All copyrights reserved to Roger-WY
*******************************************************************************/

/*******************************************************************************
* INCLUDES
*/
#include <includes.h>
#include <global.h>
#include <bsp_key.h>

#define BSP_KEY_MODULE_EN 1
#if BSP_KEY_MODULE_EN > 0
/*******************************************************************************
* CONSTANTS
*/
#define KEY_SPEED_FAST			5
#define KEY_SPEED_MIDDLE		200
#define KEY_SPEED_SLOW			600


StrKeyEvt Key = {
    NULL,                           // *pkey
    KEY_MENU_HOME,                  // menu
    1,                              // lock
    KEY_EVT_NULL,                   // evt
    0,                              // step
    0,                              // mode
    KEY_VAL_NULL,                   // val
    KEY_SPEED_SLOW,                 // cnt
    0,
    0,
    {0}
};

//============================================================================//



/*******************************************************************************
* 名    称： KEY_Init
* 功    能： 按键引脚初始化
* 入口参数： 无
* 出口参数： 无
* 作　　者： Roger-WY.
* 创建日期： 2015-06-25
* 修    改：
* 修改日期：
* 备    注：
*******************************************************************************/
void KEY_Init( void )
{
    GPIO_InitTypeDef GPIO_InitStructure;

    /* Enable GPIOB clocks */
    RCC_APB2PeriphClockCmd(KEY_PORT_RCC,ENABLE);

    /* Set PC.0 ~ PC.6 GPIO_Mode_IPU*/
    GPIO_InitStructure.GPIO_Pin     = KEY_GPIO_PWR
                                    | KEY_GPIO_NRELOAD
                                    | KEY_GPIO_NRESET;

    GPIO_InitStructure.GPIO_Speed   = GPIO_Speed_2MHz;
    GPIO_InitStructure.GPIO_Mode    = GPIO_Mode_IPU;
    GPIO_Init(KEY_PORT, &GPIO_InitStructure);

    /* Set PA.11 ~ PA.12 GPIO_Mode_IPU*/
//    GPIO_InitStructure.GPIO_Pin = KEY_GPIO_BMA    | KEY_GPIO_BMB;
//    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
//    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
//    GPIO_Init(GPIOA, &GPIO_InitStructure);
}


/*******************************************************************************
* 名    称： KEY_Init
* 功    能： 按键引脚初始化
* 入口参数： key ；按键的所有对应事件
* 出口参数： 0、无按键；1、有按钮事件；2、无
* 作　　者： Roger-WY.
* 创建日期： 2015-06-25
* 修    改：
* 修改日期：
* 备    注：
*******************************************************************************/
uint8_t KEY_GetKey( StrKeyEvt *key )
{
    static  INT16S  speed   = KEY_SPEED_SLOW;
    INT16U          keys;

    /***********************************************
    * 描述：检测旋转编码器（优先级最高）
    */
//    if( KEY_GetEncodingKey(key) )
//        return TRUE;
    /***********************************************
    * 描述：按钮按住
    */
    keys = KEY_KEYS;                                // 初始化键值

    /***********************************************
    * 描述：有键按下或按住
    */
    if ( ( keys & GPIO_ReadInputData(KEY_PORT) ) != keys ) {
//#if ( OSAL_EN == DEF_ENABLED )
//        static INT08U   flag    = 0;
//        flag    = ~flag;
//        if ( flag ) {
//            osal_start_timerRl( OS_TASK_ID_KEY,
//                                (1<<0),
//                                5);
//        } else {
//#else
        BSP_OS_TimeDly(5);                               // 延时一下
//#endif
        if ( ( keys & GPIO_ReadInputData(KEY_PORT) ) != keys ) {        // 依然有键按下
            keys    = GPIO_ReadInputData(KEY_PORT);                     // 读取键值
            keys   &= KEY_KEYS;
            key->val= keys;
            /***********************************************
            * 描述：按下
            */
            if ( ( key->evt == KEY_EVT_NULL ) ||
                ( key->evt == KEY_EVT_UP ) ) {
                    key->val	= keys;
                    key->evt	= KEY_EVT_DOWN;
                    key->cnt	= KEY_SPEED_SLOW;
                    speed       = KEY_SPEED_SLOW;
                    //UartPutString( (CHAR*)"\r\n按下" );
                    return	KEY_STATUS_EVENT;
                }

            /***********************************************
            * 描述：按住
            */
            if ( --key->cnt == 0 ) {
                speed   -= 10;
                if ( speed <= KEY_SPEED_FAST ) {
                    speed   = KEY_SPEED_FAST;
                }

                key->cnt = speed;
                //UartPutString( COMM_PORT_MSG,"\r\n按住" );
                key->evt	= KEY_EVT_PRESSED;
                return KEY_STATUS_EVENT;
            }
            return KEY_STATUS_PRESSED;
        }
//#if ( OSAL_EN == DEF_ENABLED )
//        }
//#else
//#endif
    } else {
//#if ( OSAL_EN == DEF_ENABLED )
//        osal_start_timerRl( OS_TASK_ID_KEY,
//                            (1<<0),
//                            5);
//#else
        BSP_OS_TimeDly(5);                               // 延时一下
//#endif
    }

    /***********************************************
    * 描述：松开
    */
    if ( key->evt == KEY_EVT_DOWN || key->evt == KEY_EVT_PRESSED ) {
        key->evt	= KEY_EVT_UP;
        key->cnt	= KEY_SPEED_SLOW;
        //UartPutString( (CHAR*)"\r\n松开" );
        return KEY_STATUS_EVENT;
        /***********************************************
        * 描述：没有按键
        */
    } else {
        key->val	= KEY_VAL_NULL;
        key->evt	= KEY_EVT_NULL;
        key->cnt	= KEY_SPEED_SLOW;
        return KEY_STATUS_NULL;
    }
}


/*******************************************************************************
研究了好阵子的编码开关程序，发现中断法，扫描法都无法很好地去抖。
后来在网上看到下面这种处理方式，经验证是最可靠地，非常好地避免了抖动问题。
不妨设旋转编码器的输出波形如下
______        ______        ______        ______
正转时:  A相   ___|      |______|      |______|      |______|

______        ______        ______        ___
B相   ______|      |______|      |______|      |______|

低位为A相         01  11 10  00 01  11 10  00 01  11 10  00 01  11
______        ______        ______        ___
反转时:  A相   ______|      |______|      |______|      |______|
______        ______        ______        ______
B相   ___|      |______|      |______|      |______|
低位为A相         10 11  01  00 10  11 01  00 10  11 01  00 10  11

A相接在某端口上，对应检测代码为　if((REG32(P4SIN) & (1<<3)))  PhaseShift |= 0x01;
B相接在某端口上，对应检测代码为　if((REG32(P1SIN) & (1<<2)))  PhaseShift |= 0x02;

PhaseShift的<1:0>两位表示当前AB两相的状态，<3:2>两位表示上一次AB两相的状态
正转时，AB相变化顺序为   01->11->10->00->01    低位为A相
因此正转时，PhaseShift的低四位值为 0111 1110 1000 0001
反转时，AB相变化顺序为   10->11->01->00->10    低位为A相
因此反转时，PhaseShift的低四位值为 1011 1101 0100 0010
*******************************************************************************/
/***********************************************
* 描述：相位代码
*/
#define phase_no_move   0       // 0000 未转动
#define phase_dec      -1       // 0001 正转
#define phase_inc       1       // 0010 反转
#define phase_bad       2       // 0011 不可能

/***********************************************
* 描述：假设正转为减，反转为加，则可得如下一表
*/
const INT8S table_Phase[16]=
{
    phase_no_move,    // 0000 未转动
    phase_dec,        // 0001 正转
    phase_inc,        // 0010 反转
    phase_bad,        // 0011 不可能

    phase_inc,        // 0100 反转
    phase_no_move,    // 0101 未转动
    phase_bad,        // 0110 不可能
    phase_dec,        // 0111 正转

    phase_dec,        // 1000 正转
    phase_bad,        // 1001 不可能
    phase_no_move,    // 1010 未转动
    phase_inc,        // 1011 反转

    phase_bad,        // 1100 不可能
    phase_inc,        // 1101 反转
    phase_dec,        // 1110 正转
    phase_no_move,    // 1111 未转动

};

/*******************************************************************************
* 名    称： KEY_GetEncodingKey
* 功    能： 获取旋转编码器的键值
* 入口参数： key ；按键的所有对应事件
* 出口参数： 0、无按键；1、有按钮事件；2、无
* 作　　者： Roger-WY.
* 创建日期： 2015-06-25
* 修    改：
* 修改日期：
* 备    注：
*******************************************************************************/
uint8_t KEY_GetEncodingKey ( StrKeyEvt *key )
{
    CPU_INT32U pin;

    static CPU_INT32U PhaseShift = 0;
    static CPU_INT32S PhaseCount = 0;

    /***********************************************
    * 描述：A相接在某端口上，对应检测代码为
    */
    pin = GPIO_ReadInputDataBit(KEY_PORT_BM, KEY_GPIO_BMA);
    if (pin == 0)
        PhaseShift |= 0x01;

    /***********************************************
    * 描述：B相接在某端口上，对应检测代码为
    */
    pin = GPIO_ReadInputDataBit(KEY_PORT_BM, KEY_GPIO_BMB);
    if (pin == 0)
        PhaseShift |= 0x02;


    /***********************************************
    * 描述：取低4位
    */
    PhaseShift &= 0x0f;

    /***********************************************
    * 描述：根据PhaseShift的低四位值来查此表即可得转动方向
    */
    if(table_Phase[PhaseShift] == phase_inc)
        PhaseCount++;
    if(table_Phase[PhaseShift] == phase_dec)
        PhaseCount--;
    PhaseShift <<= 2;
    /***********************************************
    * 描述：当同一方向上的转动次数为4次时，
    *       才认为是一次有效的转动，达到了去抖的目的
    */
    if(PhaseCount > 3) {
        PhaseCount -= 4;
        key->val    = KEY_INC;
        key->evt    = KEY_EVT_UP;
        return KEY_STATUS_EVENT;                    // phase_inc;
    } else if(PhaseCount < -3) {
        PhaseCount += 4;
        key->val    = KEY_DEC;
        key->evt    = KEY_EVT_UP;
        return KEY_STATUS_EVENT;                    // phase_dec;
    }

    return KEY_STATUS_NULL;                         // (INT8S)(phase_no_move);
}
/*******************************************************************************
* 				end of file
*******************************************************************************/
#endif