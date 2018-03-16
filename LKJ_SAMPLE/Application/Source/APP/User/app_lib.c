/*******************************************************************************
 *   Filename:       app_lib.c
 *   Revised:        All copyrights reserved to Wuming Shen.
 *   Date:           2014-07-05
 *   Revision:       v1.0
 *   Writer:	     Wuming Shen.
 *
 *   Description:    双击选中 wdt 按 Ctrl + H, 钩选 Match the case, Replace with
 *                   输入您要的名字，点击 Replace All
 *                   双击选中 Wdt 按 Ctrl + H, 钩选 Match the case, Replace with
 *                   输入您要的名字，点击 Replace All
 *                   双击选中 WDT 按 Ctrl + H, 钩选 Match the case, Replace with
 *                   输入您要的名字，点击 Replace All
 *                   在 app_cfg.h 中指定任务的优先级 和 任务堆栈大小
 *
 *   Notes:
 *					QQ:276193028
 *     				E-mail:shenchangwei945@163.com
 *
 *******************************************************************************/

/*******************************************************************************
 * INCLUDES
 */
#define  SNL_APP_SOURCE
#include <app.h>
#include <app_lib.h>
#include <app_ctrl.h>

#ifdef VSC_INCLUDE_SOURCE_FILE_NAMES
const  CPU_CHAR  *app_lib__c = "$Id: $";
#endif

#define APP_TEMP_EN     DEF_ENABLED
#if APP_TEMP_EN == DEF_ENABLED
/*******************************************************************************
 * CONSTANTS
 */
#define CYCLE_TIME_TICKS            (OS_TICKS_PER_SEC * 1)

/*******************************************************************************
 * MACROS
 */

/*******************************************************************************
 * TYPEDEFS
 */
     
/*******************************************************************************
 * LOCAL VARIABLES
 */
/***********************************************
* 描述： NTC线性表
*/

/*******************************************************************************
 * GLOBAL VARIABLES
 */
extern StrCtrlDCB              Ctrl;
UnionCtrlPara   EepWrBuf,EepRdBuf;

/*******************************************************************************
 * LOCAL FUNCTIONS
 */

/*******************************************************************************
 * GLOBAL FUNCTIONS
 */

/*******************************************************************************
 * EXTERN VARIABLES
 */

 /*******************************************************************************
 * EXTERN FUNCTIONS
 */

/*******************************************************************************/

/*******************************************************************************
 * 名    称： App_SetParaToEep
 * 功    能：
 * 入口参数：
 * 出口参数： 无
 * 作　　者： 无名沈
 * 创建日期： 2014-03-28
 * 修    改：
 * 修改日期：
 *******************************************************************************/
BOOL App_SetParaToEep(INT16U addr, INT08U *pbuf, INT16U len)
{
    INT08U    retrys    = 5;
    INT08U    ret;
    INT08U    *pc;
    INT08U    *pb;
    /***********************************************
    * 描述： 将默认参数写入EEPROM中
    */
    do {
        pb  = (INT08U   *)pbuf;
        pc  = (INT08U   *)&EepRdBuf;
        
        if ( len == BSP_EEP_WriteBytes( addr, pb, len ) ) {
            if ( len == BSP_EEP_ReadBytes( addr, pc, len ) ) {
                for ( INT08U i = 0; i < len; i++ ) {
                    ret     = 1;
                    if ( *pb++ != *pc++ ) {
                        ret = 0;
                        break;
                    }
                }
                if ( ret )
                    break;
            }
        }
    } while ( --retrys );
    
    if ( !ret )
        return FALSE;
    
    return TRUE;
}

/*******************************************************************************
 * 名    称： App_SetParaToEeprom()
 * 功    能： 应用参数存储到EERPOM
 * 入口参数：
 * 出口参数： 无
 * 作　　者：
 * 创建日期：
 * 修    改： 无名沈
 * 修改日期： 2014-06-25
 *******************************************************************************/
BOOL App_SetParaToEeprom(void)
{
#if (STORAGE_TYPE == INTNER_FLASH )
    Ctrl.Para.dat.RestoreSetting    = 0xA5;
    //BSP_FlashWrite((INT32U  ) PARAM_START_ADDR + 0,
    //          (INT16U *)Ctrl.Para.buf2,128);
    BSP_FlashWriteBytes((u32) 0*2, (u8 *)Ctrl.Para.buf1, (u16) 128*2);
#elif (STORAGE_TYPE == INTNER_EEPROM )
#elif (STORAGE_TYPE == EXTERN_EEPROM )
    INT08U    ret;
    INT08U   *pb;
    
    pb    = (INT08U       *)&EepWrBuf;
    
    /***********************************************
    * 描述： 将默认参数写入EEPROM中
    */
    pb    = (INT08U   *)Ctrl.Para.buf1;
    ret   = App_SetParaToEep( 0, pb, 255 );
    
    if ( !ret )
        return FALSE;
    
    /***********************************************
    * 描述： 初始化数据标志写入EEPROM中
    */
    if(!BSP_EEP_WriteByteChk(255, 0xA5)) {
        return FALSE;
    }
    
#endif
    return TRUE;
}
extern  const StrParaRange MbsParaRange[];
/*******************************************************************************
 * 名    称： App_GetParaFromEeprom()
 * 功    能： 应用参数读取到RAM
 * 入口参数：
 * 出口参数： 无
 * 作　　者：
 * 创建日期：
 * 修    改： 无名沈
 * 修改日期： 2014-06-25
 *******************************************************************************/
BOOL App_GetParaFromEeprom(void)
{    
#if (STORAGE_TYPE == INTNER_FLASH )    
    INT08U   *pb = (INT08U   *)&EepWrBuf;
    //BSP_FlashRead((INT32U  ) PARAM_START_ADDR + 0,
    //          (INT16U *)pu,127);
    BSP_FlashReadBytes((u32) 0*2, (u8 *)pb, (u16) 127*2);
#elif (STORAGE_TYPE == INTNER_EEPROM )
#elif (STORAGE_TYPE == EXTERN_EEPROM )
    INT08U   *pb = (INT08U   *)&EepWrBuf;
    /***********************************************
    * 描述： 已经配置过了，非首次初始化，则读取参数到RAM
    */
    /***********************************************
    * 描述： 读取MODBUS参数
    */
    
    INT08U  retrys  = 10;
    do {
        if ( 255 == BSP_EEP_ReadBytes(0,(INT8U *)pb, 255) )
            break;
        
#ifdef APP_LIBRARY
        Delay_Nms( 10 );
#else
        BSP_OS_TimeDly(10);
#endif
    } while( --retrys );
    
    if ( !retrys )
        return FALSE;
#endif
    /***********************************************
    * 描述： 缓冲区参数定入MODBUS参数
    */
    StrParaRange *prag    = Ctrl.Prge;
    
#if ( PARA_TYPE == PARA_TYPE_INT16U )
    INT16U  *pu = (INT16U*)pb;
    for ( INT8U i = 0; i < 127; i++, pu++,prag++ ) {
        if ( ( (INT16U)*pu >= prag->Min ) &&
            ( (INT16U)*pu <= prag->Max ) ) {
            Ctrl.Para.buf2[i] = *pu;
        }
    }
#elif ( PARA_TYPE == PARA_TYPE_INT16S )
    INT16S  *ps = (INT16S*)pb;
    for ( INT8U i = 0; i < 127; i++, ps++,prag++ ) {
        if ( ( (INT16S)*ps >= prag->Min ) &&
            ( (INT16S)*ps <= prag->Max ) ) {
            Ctrl.Para.buf2[i] = *ps;
        }
    }
#elif ( PARA_TYPE == PARA_TYPE_INT08U )
    for ( INT8U i = 0; i < 254; i++, pb++,prag++ ) {
        if ( ( (INT08U)*pb >= prag->Min ) &&
            ( (INT08U)*pb <= prag->Max ) ) {
            Ctrl.Para.buf1[i] = *pb;
        }
    }
#else
    for ( INT8U i = 0; i < 254; i++, pb++,prag++ ) {
        if ( ( (INT08S)*pb >= prag->Min ) &&
            ( (INT08S)*pb <= prag->Max ) ) {
            Ctrl.Para.buf1[i] = *pb;
        }
    }
#endif
    return TRUE;
}

/*******************************************************************************
 * 名    称：
 * 功    能：
 * 入口参数：
 * 出口参数： 无
 * 作　　者： 无名沈
 * 创建日期： 2014-03-28
 * 修    改：
 * 修改日期：
 *******************************************************************************/
BOOL App_PraInit(void)
{
    extern const StrCtrlPara    MbsParaAtrIdx;
    
    Ctrl.Prge               = (StrParaRange *)MbsParaRange;
    //Ctrl.Key                = (StrKeyEvt *)&Key;
    Ctrl.Para.dat.idx       = &MbsParaAtrIdx;
    
#if (STORAGE_TYPE == INTNER_FLASH ) 
    BSP_FlashReadBytes((u32) 0*2, (u8 *)EepWrBuf.buf1, (u16) 128*2);
    INT16U dat = EepWrBuf.buf2[127];    
#elif (STORAGE_TYPE == INTNER_EEPROM )
#elif (STORAGE_TYPE == EXTERN_EEPROM )
    INT08U dat = 0;
    BSP_EEP_Init();                                 // 初始化存储器
    /***********************************************
    * 描述：读取初始化地址上的值
    */
    INT08U  retrys  = 10;
    do {
        if ( TRUE == BSP_EEP_ReadByte(255, &dat))
            break;
#ifdef APP_LIBRARY
        Delay_Nms( 10 );
#else
        BSP_OS_TimeDly(10);
#endif
    } while( --retrys );
#endif
    /***********************************************
    * 描述：跟默认值比较
    */
    if(dat == 0xA5) {
        dat = App_GetParaFromEeprom();                  // 已经初始化，读取参数
        Ctrl.Sys.FirstPwrFlag       = FALSE;
        /***********************************************
        * 描述：没有初始化，保存默念值
        */
    } else {        
        dat = App_SetParaToEeprom();                    // 未初始化，保存默认参数
        Ctrl.Sys.FirstPwrFlag       = TRUE;
    }
    
    return (BOOL)dat;
}

/*******************************************************************************
 * 				                    延时函数                                   *
 *******************************************************************************/
/*******************************************************************************
* 名    称： Delay_Nus()
* 功    能： 延时大概1us
* 入口参数： dly		延时参数，值越大，延时越久
* 出口参数： 无
* 作　　者： 无名沈
* 创建日期： 2009-01-03
* 修    改：
* 修改日期：
*******************************************************************************/
OPTIMIZE_NONE void  Delay_Nus( INT32U  dly )
{
    /***********************************************
    * 描述：定时器初始化
    *       72000000次循环  = 6.9999999583333333333333333333333秒
    *       一次循环 = 6.9999999583333333333333333333333 / 72000000
    *       = 97.222221643518518518518518518519 ns
    *       ~= 0.1 us
    *       延时1us  dly  = 9
    *       dly = 0时，循环一次
    *
    while(dly--);
    *//***********************************************
    * 描述：定时器初始化
    *       1000000次循环  = 1027.7778611111111111111111111111ms
    *       平均一次循环 = 1.02777786111111111111111111111us
    *       dly = 1时，= 1.1111111111111111111111111111111us
    *       dly = 2时  = 2.1388888888888888888888888888889us
    *       dly 每增加1 增加-1.0277777777777777777777777777778us
    */
    INT32U  i;

    while(dly--) {
        for(i=0; i<7; i++);             // clk = 72000000 1027.7778611111111111111111111111
    }
}

/*******************************************************************************
* 名    称： Delay_Nms()
* 功    能： 延时大概1ms
* 入口参数： dly		延时参数，值越大，延时越久
* 出口参数： 无
* 作　　者： 无名沈
* 创建日期： 2009-01-03
* 修    改：
* 修改日期：
*******************************************************************************/
OPTIMIZE_NONE void  Delay_Nms( INT32U  dly )
{
#if UCOS_II_EN > 0
    OSTimeDly(dly);
#else
    CPU_INT32U  hclk_freq;
    CPU_INT32U  cnts;
    INT32U      i;

    hclk_freq   = BSP_CPU_ClkFreq();            // hclk_freq = 72000000时dly 1000 接近1秒
    cnts        = hclk_freq / 9000;
    //WdtReset();

    for( ; dly > 0; dly-- ) {
        for(i=0; i<cnts; i++);
    }
#endif
}

/*******************************************************************************
* 名    称：Delay_Ns()
* 功    能：延时大概1s
* 入口参数：dly		延时参数，值越大，延时越久
* 出口参数：无
* 作　　者：无名沈
* 创建日期：2009-01-03
* 修    改：
* 修改日期：
*******************************************************************************/
OPTIMIZE_NONE void  Delay_Ns( INT32U  dly )
{
#if UCOS_II_EN > 0
    OSTimeDlyHMSM(dly/3600,dly/60%60,dly%3600,0);
#else
    for( ; dly > 0; dly--) {
        //WdtReset();
        Delay_Nms( 950 );
    }
#endif
}

/*******************************************************************************
 * 				                    end of file                                *
 *******************************************************************************/
#endif