//                           _ooOoo_
//                          o8888888o
//                          88" . "88
//                          (| -_- |)
//                          O\  =  /O
//                       ____/`---'\____
//                     .'  \\|     |//  `.
//                    /  \\|||  :  |||//  \
//                   /  _||||| -:- |||||-  \
//                   |   | \\\  -  /// |   |
//                   | \_|  ''\---/''  |   |
//                   \  .-\__  `-`  ___/-. /
//                 ___`. .'  /--.--\  `. . __
//              ."" '<  `.___\_<|>_/___.'  >'"".
//             | | :  `- \`.;`\ _ /`;.`/ - ` : | |
//             \  \ `-.   \_ __\ /__ _/   .-` /  /
//        ======`-.____`-.___\_____/___.-`____.-'======
//                           `=---='
//
//
//        ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
//                   佛祖保佑       永不死机
//                   心外无法       法外无心

/*******************************************************************************
 *   Filename:      bsp_pm25.C
 *   Revised:       $Date: 2015-07-11
 *   Revision:      $V1.0
 *   Writer:        wumingshen.
 *
 *   Description:   PM2.5灰尘测量驱动模块
 *   Notes:         适用于夏普一代灰尘传感器，通过采集AD值来获取PM2.5的值
 *
 *
 *   All copyrights reserved to wumingshen.
 *
 *******************************************************************************/
#include "global.h"
#include "bsp_pm25.h"
#include "bsp_adc.h"

/***********************************************
* 描述： PM2.5划分等级的值
*/
#define  PM2D5_LEV_0            (35 +0)// 50              //小于EXCELLENCE_VAL值，等级为优
#define  PM2D5_LEV_1            (75 +0)// 100             //小于FINE_VAL值，大于EXCELLENCE_VAL值，等级为良
#define  PM2D5_LEV_2            (115+0)// 150             //小于FINE_VAL值，大于EXCELLENCE_VAL值，等级为良
#define  PM2D5_LEV_3            (150+0)// 200             //小于FINE_VAL值，大于EXCELLENCE_VAL值，等级为良

#define PM2D5_ADC_BUF_LEN       100    
#define PM2D5_UART_BUF_LEN      14
     
u8      PM2D5_UartBuf           [PM2D5_UART_BUF_LEN];
u16     PM2D5_AdcBuf            [PM2D5_ADC_BUF_LEN];

/*******************************************************************************
 * 名    称： Bsp_Pm25Init
 * 功    能： PM2.5传感器硬件初始化
 * 入口参数： 无
 * 出口参数： 无
 * 作　 　者： 无名沈
 * 创建日期： 2015-07-20
 * 修    改：
 * 修改日期：
 * 备    注：
 *******************************************************************************/
void BSP_PM2D5Init(void)
{
    /***********************************************
    * 描述： 初始化PM2.5传感器内部LED 端口配置
    */
    GPIO_InitTypeDef GPIO_InitStructure;

    RCC_APB2PeriphClockCmd(PM25LED_RCC, ENABLE);
    GPIO_InitStructure.GPIO_Pin   = PM25LED_PIN ;
    GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_Out_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(PM25LED_PORT, &GPIO_InitStructure);

    PM25LEDOFF();

    /***********************************************
    * 描述： 初始化采集PM2.5传感器的ADC通道
    */
    Bsp_ADC_Init();	          /* 初始化 ADC1 模块 */

    /***********************************************
    * 描述： 初始化2代串口
    */
    BSP_UartCfg( 1, 2400 );
#if defined(PM2D5_UART_CHK)
    BSP_UartCfg( 0, 9600 );                       // 启用串时LED会异常
#endif
    
    /***********************************************
    * 描述： 一代斜率初始化
    */
    char i  = 0;
    if (( Ctrl.Para.dat.Pm2d5Chk.Adc.x1[i] >= Ctrl.Para.dat.Pm2d5Chk.Adc.x2[i] ) ||
        ( Ctrl.Para.dat.Pm2d5Chk.Value.y1[i] >= Ctrl.Para.dat.Pm2d5Chk.Value.y2[i] ) ) {
        Ctrl.Para.dat.Pm2d5Chk.Adc.x1[i]    = (INT16S)pm_x1;
        Ctrl.Para.dat.Pm2d5Chk.Value.y1[i]  = (INT16S)pm_y1;
        Ctrl.Para.dat.Pm2d5Chk.Adc.x2[i]    = (INT16S)pm_x2;
        Ctrl.Para.dat.Pm2d5Chk.Value.y2[i]  = (INT16S)pm_y2;
        
        BSP_FlashWriteBytes((u32) 10*2, (u8 *)&Ctrl.Para.dat.Pm2d5Chk.Offset, (u16) 10*2);
        
        osal_start_timerEx( OS_TASK_ID_SEN,
                            OS_EVT_SEN_PM2D5_CHK,
                            OS_TICKS_PER_SEC * 120);
        BSP_OS_TimeDly(1000);
        BeepExt(3, 1500, 200);
    }    
    Ctrl.PM2D5.k[i]     = ((float)(Ctrl.Para.dat.Pm2d5Chk.Value.y2[i] - Ctrl.Para.dat.Pm2d5Chk.Value.y1[i])
                        / (float)(Ctrl.Para.dat.Pm2d5Chk.Adc.x2[i] - Ctrl.Para.dat.Pm2d5Chk.Adc.x1[i]));
    Ctrl.PM2D5.b[i]     = (Ctrl.Para.dat.Pm2d5Chk.Value.y1[i] - Ctrl.PM2D5.k[i] *Ctrl.Para.dat.Pm2d5Chk.Adc.x1[i]);  

    /***********************************************
    * 描述： 二代斜率初始化
    */
    i   = 1;
    if (( Ctrl.Para.dat.Pm2d5Chk.Adc.x1[i] >= Ctrl.Para.dat.Pm2d5Chk.Adc.x2[i] ) ||
        ( Ctrl.Para.dat.Pm2d5Chk.Value.y1[i] >= Ctrl.Para.dat.Pm2d5Chk.Value.y2[i] ) ) {
        Ctrl.Para.dat.Pm2d5Chk.Adc.x1[i]    = (INT16S)pm_X1;
        Ctrl.Para.dat.Pm2d5Chk.Value.y1[i]  = pm_Y1;
        Ctrl.Para.dat.Pm2d5Chk.Adc.x2[i]    = (INT16S)pm_X2;
        Ctrl.Para.dat.Pm2d5Chk.Value.y2[i]  = pm_Y2;
       
        BSP_FlashWriteBytes((u32) 10*2, (u8 *)&Ctrl.Para.dat.Pm2d5Chk.Offset, (u16) 10*2);
        //
        //osal_start_timerEx( OS_TASK_ID_SENSOR,
        //                   OS_EVT_SEN_PM2D5_CHK,
        //                    OS_TICKS_PER_SEC * 30);
    }  
    Ctrl.PM2D5.k[i]     = ((float)(Ctrl.Para.dat.Pm2d5Chk.Value.y2[i] - Ctrl.Para.dat.Pm2d5Chk.Value.y1[i])
                        / (float)(Ctrl.Para.dat.Pm2d5Chk.Adc.x2[i] - Ctrl.Para.dat.Pm2d5Chk.Adc.x1[i]));
    Ctrl.PM2D5.b[i]     = (Ctrl.Para.dat.Pm2d5Chk.Value.y1[i] - Ctrl.PM2D5.k[i] *Ctrl.Para.dat.Pm2d5Chk.Adc.x1[i] + 39.5);  
}

/*******************************************************************************
 * 名    称： Bsp_GetPm25AdVal
 * 功    能： 获取PM2.5传感器的AD值
 * 入口参数： 无
 * 出口参数： 无
 * 作　 　者： 无名沈
 * 创建日期： 2015-07-20
 * 修    改：
 * 修改日期：
 * 备    注：
 *******************************************************************************/
void BSP_PM2D5G1GetAdcVal(u16 *pdata)
{
    /***********************************************
    * 描述： 开PM2.5传感器内部LED
    */
    PM25LEDON();
    /***********************************************
    * 描述： 延时280uS
    */
    //static u16 pm2d5_dly  = 280;
    Delay_Nus(275);

    /***********************************************
    * 描述： 开启AD转换或者此时PM2.5的AD值
    */
    ADC1Convert_Begin();            /* 开始AD转换 */
    Get_AD_AverageValue();
    *pdata =  ADC_AverageValue[0];
    //*pdata =  ADC_Value[0][0];
    ADC1Convert_Stop();             /* 关闭AD转换 */

    /***********************************************
    * 描述：关PM2.5传感器内部LED
    */
    PM25LEDOFF();
}

/*******************************************************************************
 * 名    称： BSP_PM2D5GetAdcValue
 * 功    能： 10ms定时回调函数
 * 入口参数： 
 * 出口参数： 
 * 作　 　者： Roger-wumingshen
 * 创建日期： 2015-10-19
 * 修    改：
 * 修改日期：
 * 备    注：
 *******************************************************************************/
void BSP_PM2D5GetAdcValueCB(void)
{
    static u8 ctr = 0;
    /***********************************************
    * 描述：注意：回调函数中执行的时间不宜过长
    */
    BSP_PM2D5G1GetAdcVal(&PM2D5_AdcBuf[ctr]);

    if ( ++ctr >= PM2D5_ADC_BUF_LEN ) {
        ctr    = 0;
    }
}

/*******************************************************************************
 * 名    称： BSP_PM2D5G1GetValue
 * 功    能： 获取PM2.5 1代值
 * 入口参数： pm2d5  PM2.5的值
 * 出口参数： 污染等级
 * 作　 　者： 无名沈
 * 创建日期： 2015-07-20
 * 修    改：
 * 修改日期：
 * 备    注：
 *******************************************************************************/
s16 BSP_PM2D5G1GetValue(void)
{
//#define FILTER_BUF_LEN  16
    u16             After[PM2D5_ADC_BUF_LEN];
    static u16      PM25ADDataBuff[FILTER_BUF_LEN]   = {0};          // 
    static  u8      FilterCtr   = 0;
    u32             sum         = 0;
    static float    x           = 0.0;
    static float    last        = 0.0;    
    
    float           k           = Ctrl.PM2D5.k[0];//(1.5*(y2-y1)/(x2-x1));     // 1.8125//400/(839.111 - 170.667) = 400/648.444=0.5984
    float           b           = Ctrl.PM2D5.b[0];//(y1 - k *x1 + 39.5);       // -0.5984*170.667=-102.127
    
    /***********************************************
    * 描述： 一次滤波：排序
    */
    App_BubbleAscendingArray(PM2D5_AdcBuf, PM2D5_ADC_BUF_LEN, After);
    u16 *p  = After;
    
    for(u8 i = 20; i < 90;i++)
        sum += p[i];
    
    x = (float)(sum/70.0);
    /***********************************************
    * 描述： 二次滤波：平均
    */
    //if ( 0 == Ctrl.PM2D5.Type ) {                   // 如果有2代传感器，自动会加
    //    Ctrl.PM2D5.RxCtr++;
    //}
    
    PM25ADDataBuff  [FilterCtr]  = (s16)x;
    sum = 0;
    if ( ++FilterCtr >= FILTER_BUF_LEN )
        FilterCtr   = 0;
    /***********************************************
    * 描述： 第一次上电或数值变化过大时，快速到达
    */
    if ( ( ( last == 0.0 )      && ( x != 0 ) ) ) {// || 
         //( abs( x - last ) > 150 ) && abs( x - last ) < 400 ) ) {
        last    = (uint16)x;
        for ( u8 i = 0; i < FILTER_BUF_LEN; i++ )
        PM25ADDataBuff[i]   = (s16)x;
    }
    
    App_BubbleAscendingArray(PM25ADDataBuff,FILTER_BUF_LEN,After);
        
    for(u8 i = 3; i < FILTER_BUF_LEN - 3; i++)
        sum += p[i];
    
    x = (float)(sum/( FILTER_BUF_LEN - 6 ));
    
    //for (u8 i = 0; i < FILTER_BUF_LEN; i++)
    //    sum   +=  PM25ADDataBuff[i];
    //x = (float)(sum/8.0); 

    /***********************************************
    * 描述： 三次滤波：平滑
    */
    if ( last != 0.0 ) {
        last    = last * 0.7 + x * 0.3;
    } else {
        if ( x != 0 )
            last        = x;
    }
    x                   = last;    
    Ctrl.PM2D5.G1AdVal  = (s16)x;
    /***********************************************
    * 描述： 计算当前值
    */
    s16 y   = (s16)(k*x + b);
    
    return  (s16)y;//(u16)( (y > 0) ? y : 0 );
}

/*******************************************************************************
 * 名    称： BSP_PM2D5G2GetValue
 * 功    能： 获取PM2.5 2W代（串口型）的值
 * 入口参数： 无
 * 出口参数： 无
 * 作　 　者： 无名沈.
 * 创建日期： 2015-03-28
 * 修    改：
 * 修改日期：
 *******************************************************************************/
s16 BSP_PM2D5G2GetValue(void)
{
    static float    last    = 0.0;
    u32             sum     = 0;
    static float    x       = 0;
    
    u8  CheckVal            = 0;
    static u16 PM25ADDataBuff[8]   = {0};           // 与Ctrl.PM2D5中RxCtr位数对应
    u8  *pbuf   = PM2D5_UartBuf;
    
    /***********************************************
    * 描述： 检测是否有2代传感器
    */
    if ( ++Ctrl.PM2D5.RxFaildCtr > 5 ) {            // 2代传感器超时，改成1代
        Ctrl.PM2D5.RxFaildCtr   = 6;
        Ctrl.PM2D5.Type = 0;
        x   = 0;
        return 0;
    } else { 
        for(u8 i = 0;i < PM2D5_UART_BUF_LEN - 7; i++,pbuf++) {
            /***********************************************
            * 描述： 找PM2.5的数据帧开头和结尾
            */
            if((pbuf[0] == 0xAA) && (pbuf[6] == 0xFF)) {
                CPU_SR  cpu_sr;
                CPU_CRITICAL_ENTER();
                CheckVal    = pbuf[1] + pbuf[2] + pbuf[3] + pbuf[4];
                if(CheckVal == pbuf[5]) {
                    x   = pbuf[1]*256 + pbuf[2];
                    PM25ADDataBuff[Ctrl.PM2D5.RxCtr++] = (u16)x;
                    CPU_CRITICAL_EXIT();
                    Ctrl.PM2D5.Type = 1;
                    break;
                }
                CPU_CRITICAL_EXIT();
            }
        }
        /***********************************************
        * 描述： 第一次上电或数值变化过大时，快速到达
        */
        if ( ( ( last == 0.0 )      && ( x != 0 ) ) || 
             ( ( x - last ) > 200   || ( last - x ) > 200 ) ) {
            last    = (uint16)x;
            for ( uint8 i = 0; i < 8; i++ )
                PM25ADDataBuff[i]   = (u16)x;
        }
        /***********************************************
        * 描述： 2代传感器未超时
        */
        for (u8 i = 0; i < 8; i++)
            sum   +=  PM25ADDataBuff[i];
        
        x = (u16)(sum/8);
    }
    
    /***********************************************
    * 描述： 
    */
    float           K       = Ctrl.PM2D5.k[1];//((Y2-Y1)/(X2-X1));     // 1.8125//400/(839.111 - 170.667) = 400/648.444=0.5984
    float           B       = Ctrl.PM2D5.b[1];//(Y1 - K *X1 + 0);       // -0.5984*170.667=-102.127
  
    /***********************************************
    * 描述： 滤波
    */
    if ( last != 0.0 ) {
        last    = last * 0.7 + x * 0.3;
    } else {
        if ( x != 0 )
            last        = x;
    }
    x                   = last;

    Ctrl.PM2D5.G2AdVal  = (s16)x;
    /***********************************************
    * 描述： 计算实际值
    */
    s16 y   = (s16)(K*x + B);
    /***********************************************
    * 描述： 返回
    */
    return  (s16)y;//(u16)( (y > 0) ? y : 0 );
}

/*******************************************************************************
 * 名    称： BSP_PM2D5GetValue
 * 功    能： 
 * 入口参数： 
 * 出口参数： 
 * 作　 　者： wumingshen
 * 创建日期： 2015-10-19
 * 修    改：
 * 修改日期：
 * 备    注：
 *******************************************************************************/
s16 BSP_PM2D5GetValue(u8 type)
{
    /***********************************************
    * 描述： 一代传感器读取
    */
    if ( type == 0 ) 
        return BSP_PM2D5G1GetValue();
    /***********************************************
    * 描述： 二代传感器读取
    */
    else
        return BSP_PM2D5G2GetValue();
}

/*******************************************************************************
 * 名    称： BSP_PM2D5GetLevel
 * 功    能： 获取PM2.5传感器的真实值
 * 入口参数： pm2d5  PM2.5的值
 * 出口参数： 污染等级
 * 作　 　者： 无名沈
 * 创建日期： 2015-07-20
 * 修    改：
 * 修改日期：
 * 备    注：
 *******************************************************************************/
uint8_t BSP_PM2D5GetLevel(u16 pm2d5)
{
    /***********************************************
    * 描述： 
    */
    typedef struct _PM2D5_LEVEL {
        u32     retlevel    : 2;
        u32     lastlevel   : 2;
        u32     lastvalue   :12;
        u32     chgctr      : 6;
        u32     rsv         :10;
    } StrPM2D5Level;
    
    static StrPM2D5Level    pm25Level;
    u16                     pm2d5cmpL;
    u16                     pm2d5cmpH;
    
    /***********************************************
    * 描述： 
    */
#define DL                  5
#define DH                  2
#define CHG_CTR             30
    
    /***********************************************
    * 描述： PM2.5等级： 优
    */
    if(pm2d5 <=  PM2D5_LEV_0) {
        pm25Level.retlevel  = 0;
        pm2d5cmpL           = PM2D5_LEV_0;
        pm2d5cmpH           = PM2D5_LEV_0;
    /***********************************************
    * 描述： PM2.5等级： 良、
    */
    } else if( pm2d5 <= PM2D5_LEV_1 ) {
        pm25Level.retlevel  = 1;
        pm2d5cmpL           = PM2D5_LEV_0;
        pm2d5cmpH           = PM2D5_LEV_1;
    /***********************************************
    * 描述： PM2.5等级： 中、
    */
    } else if( pm2d5 <= PM2D5_LEV_2 ) {
        pm25Level.retlevel  = 1;
        pm2d5cmpL           = PM2D5_LEV_1;
        pm2d5cmpH           = PM2D5_LEV_2;
    /***********************************************
    * 描述： PM2.5等级： 差、
    */
    } else {
        pm25Level.retlevel  = 2;//3;
        pm2d5cmpL           = PM2D5_LEV_2;
        pm2d5cmpH           = PM2D5_LEV_3;
    }
    
    /***********************************************
    * 描述： 如果等级上升
    */
    if ( pm25Level.retlevel > pm25Level.lastlevel ) {
        if ( pm2d5 < pm2d5cmpL + DL ) {                  // 当前值小于下限+阀值
            pm25Level.chgctr    = 0;
            pm25Level.retlevel  = pm25Level.lastlevel;
        } else {
            if ( (pm25Level.chgctr < CHG_CTR) && 
                 (pm25Level.chgctr++ < CHG_CTR ) ) {   // 持续检测5次
                pm25Level.retlevel  = pm25Level.lastlevel;
            }
        }
    /***********************************************
    * 描述： 如果等级下降
    */
    } else if ( pm25Level.retlevel < pm25Level.lastlevel ) {
        if ( pm2d5 > pm2d5cmpH - DH ) {                  // 当前值大于上限-阀值
            pm25Level.chgctr    = 0;
            pm25Level.retlevel  = pm25Level.lastlevel;
        } else {
            if ( (pm25Level.chgctr < CHG_CTR) &&
                 (pm25Level.chgctr++ < CHG_CTR ) ) {    // 持续检测5次
                pm25Level.retlevel  = pm25Level.lastlevel;
            }
        }
    /***********************************************
    * 描述： 如果等级不变
    */
    } else {
        pm25Level.chgctr    = 0;
    }
    
    /***********************************************
    * 描述： 
    */
    pm25Level.lastlevel   = pm25Level.retlevel;
    pm25Level.lastvalue   = pm2d5;
    
    /***********************************************
    * 描述： 
    */
    return(pm25Level.retlevel);
}

void BSP_PM2D5Chk(u8 type, s16 value )
{
    char i  = 0;
    
    /***********************************************
    * 描述： 恢复默认
    */
    if ( type == PM2D5_CHK_TYPE_DEF ) {
        Ctrl.Wifi.Wr.Offset = 0;
        if ( Ctrl.PM2D5.Type    == 0 ) {
            i  = 0;
            Ctrl.Para.dat.Pm2d5Chk.Adc.x1[i]    = (INT16S)pm_x1;
            Ctrl.Para.dat.Pm2d5Chk.Value.y1[i]  = (INT16S)pm_y1;
            Ctrl.Para.dat.Pm2d5Chk.Adc.x2[i]    = (INT16S)pm_x2;
            Ctrl.Para.dat.Pm2d5Chk.Value.y2[i]  = (INT16S)pm_y2;
                
            Ctrl.Para.dat.Pm2d5Chk.Offset.b[i]       = Ctrl.Wifi.Wr.Offset;
            Ctrl.PM2D5.k[i]     = ((float)(Ctrl.Para.dat.Pm2d5Chk.Value.y2[i] - Ctrl.Para.dat.Pm2d5Chk.Value.y1[i])
                                / (float)(Ctrl.Para.dat.Pm2d5Chk.Adc.x2[i] - Ctrl.Para.dat.Pm2d5Chk.Adc.x1[i]));
            Ctrl.PM2D5.b[i]     = (Ctrl.Para.dat.Pm2d5Chk.Value.y1[i] - Ctrl.PM2D5.k[i] *Ctrl.Para.dat.Pm2d5Chk.Adc.x1[i]);  
        } else {
            i   = 1;    
            Ctrl.Para.dat.Pm2d5Chk.Adc.x1[i]    = (INT16S)pm_X1;
            Ctrl.Para.dat.Pm2d5Chk.Value.y1[i]  = (INT16S)pm_Y1;
            Ctrl.Para.dat.Pm2d5Chk.Adc.x2[i]    = (INT16S)pm_X2;
            Ctrl.Para.dat.Pm2d5Chk.Value.y2[i]  = (INT16S)pm_Y2;
           
            Ctrl.Para.dat.Pm2d5Chk.Offset.b[i]       = Ctrl.Wifi.Wr.Offset;
            Ctrl.PM2D5.k[i]     = ((float)(Ctrl.Para.dat.Pm2d5Chk.Value.y2[i] - Ctrl.Para.dat.Pm2d5Chk.Value.y1[i])
                                / (float)(Ctrl.Para.dat.Pm2d5Chk.Adc.x2[i] - Ctrl.Para.dat.Pm2d5Chk.Adc.x1[i]));
            Ctrl.PM2D5.b[i]     = (Ctrl.Para.dat.Pm2d5Chk.Value.y1[i] - Ctrl.PM2D5.k[i] *Ctrl.Para.dat.Pm2d5Chk.Adc.x1[i] + 39.5);  
        }
    /***********************************************
    * 描述： 校准低点
    */
    } else if ( type == PM2D5_CHK_TYPE_K_Y1 ) {
        /***********************************************
        * 描述： 修改PM2.5标定低值
        */
        if ( Ctrl.PM2D5.Type    == 0 ) {
            Ctrl.Para.dat.Pm2d5Chk.Value.y1[0]    = value;
            Ctrl.Para.dat.Pm2d5Chk.Adc.x1[0]      = Ctrl.PM2D5.G1AdVal;
        } else {
            Ctrl.Para.dat.Pm2d5Chk.Value.y1[1]    = value;
            Ctrl.Para.dat.Pm2d5Chk.Adc.x1[1]      = Ctrl.PM2D5.G2AdVal;
        } 
            
        i  = Ctrl.PM2D5.Type;
        Ctrl.Wifi.Wr.Offset = 0;
        Ctrl.Para.dat.Pm2d5Chk.Offset.b[i]       = Ctrl.Wifi.Wr.Offset;
        Ctrl.PM2D5.k[i]     = ((float)(Ctrl.Para.dat.Pm2d5Chk.Value.y2[i] - Ctrl.Para.dat.Pm2d5Chk.Value.y1[i])
                            / (float)(Ctrl.Para.dat.Pm2d5Chk.Adc.x2[i] - Ctrl.Para.dat.Pm2d5Chk.Adc.x1[i]));
        Ctrl.PM2D5.b[i]     = (Ctrl.Para.dat.Pm2d5Chk.Value.y1[i] - Ctrl.PM2D5.k[i] *Ctrl.Para.dat.Pm2d5Chk.Adc.x1[i]); 

    /***********************************************
    * 描述： 校准高点
    */
    } else if ( type == PM2D5_CHK_TYPE_K_Y2 ) {
        if ( Ctrl.PM2D5.Type    == 0 ) {
            Ctrl.Para.dat.Pm2d5Chk.Value.y2[0]    = value;
            Ctrl.Para.dat.Pm2d5Chk.Adc.x2[0]      = Ctrl.PM2D5.G1AdVal;
        } else {
            Ctrl.Para.dat.Pm2d5Chk.Value.y2[1]    = value; 
            Ctrl.Para.dat.Pm2d5Chk.Adc.x2[1]      = Ctrl.PM2D5.G2AdVal;
        }
            
        i  = Ctrl.PM2D5.Type;
        Ctrl.Wifi.Wr.Offset = 0;
        Ctrl.Para.dat.Pm2d5Chk.Offset.b[i]       = Ctrl.Wifi.Wr.Offset;
        Ctrl.PM2D5.k[i]     = ((float)(Ctrl.Para.dat.Pm2d5Chk.Value.y2[i] - Ctrl.Para.dat.Pm2d5Chk.Value.y1[i])
                            / (float)(Ctrl.Para.dat.Pm2d5Chk.Adc.x2[i] - Ctrl.Para.dat.Pm2d5Chk.Adc.x1[i]));
        Ctrl.PM2D5.b[i]     = (Ctrl.Para.dat.Pm2d5Chk.Value.y1[i] - Ctrl.PM2D5.k[i] *Ctrl.Para.dat.Pm2d5Chk.Adc.x1[i]);  
    /***********************************************
    * 描述： 校准偏移
    */
    } else if ( type == PM2D5_CHK_TYPE_B_OFFSET ) {
        Ctrl.Wifi.Wr.Offset = value - Ctrl.PM2D5.Val;
        Ctrl.Para.dat.Pm2d5Chk.Offset.b[Ctrl.PM2D5.Type]       = Ctrl.Wifi.Wr.Offset;
    /***********************************************
    * 描述： 校准偏移
    */
    } else if ( type == PM2D5_CHK_TYPE_OFFSET ) {
        Ctrl.Wifi.Wr.Offset = value;
        Ctrl.Para.dat.Pm2d5Chk.Offset.b[Ctrl.PM2D5.Type]       = Ctrl.Wifi.Wr.Offset;
    }  
        
    BSP_FlashWriteBytes((u32) 10*2, (u8 *)&Ctrl.Para.dat.Pm2d5Chk.Offset, (u16) 10*2);
}