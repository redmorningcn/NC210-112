/*******************************************************************************
 *   Filename:       app_calc.c
 *   Revised:        All copyrights reserved to Wuming Shen.
 *   Date:           2015-12-09
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
const  CPU_CHAR  *app_calc__c = "$Id: $";
#endif

/*******************************************************************************
 * CONSTANTS
 */

/*******************************************************************************
 * MACROS
 */

/*******************************************************************************
 * TYPEDEFS
 */
     
/*******************************************************************************
 * LOCAL VARIABLES
 */
#define ADC_CODE_0         (ADCres * (Ioff - 1 * STEP) * Rs / Vref )
#define ADC_CODE_1         (ADCres * (Ioff + 0 * STEP) * Rs / Vref )
#define ADC_CODE_2         (ADCres * (Ioff + 1 * STEP) * Rs / Vref )
#define ADC_CODE_3         (ADCres * (Ioff + 2 * STEP) * Rs / Vref )
#define ADC_CODE_4         (ADCres * (Ioff + 3 * STEP) * Rs / Vref )
#define ADC_CODE_5         (ADCres * (Ioff + 4 * STEP) * Rs / Vref )
#define ADC_CODE_6         (ADCres * (Ioff + 5 * STEP) * Rs / Vref )
#define ADC_CODE_7         (ADCres )

/*******************************************************************************
 * GLOBAL VARIABLES
 */
StrAdcToPressure    ADC_TO_PRESSURE_TAB[ADC_TO_PRESSURE_TAB_LEN]   = { 
    {-2000 ,    ADC_CODE_0 },     //{2384,  -2000}
    { 0    ,    ADC_CODE_1 },     //{11916, 0},                                // 0Pa
    { 2000 ,    ADC_CODE_2 },     //{21448, 2000},                             // 2000Pa
    { 4000 ,    ADC_CODE_3 },     //{30981, 4000},                             // 4000Pa
    { 6000 ,    ADC_CODE_4 },     //{40513, 6000},                             // 6000Pa
    { 8000 ,    ADC_CODE_5 },     //{50046, 8000},                             // 8000Pa
    { 10000,    ADC_CODE_6 },     //{59578, 10000}                             // 10000Pa
    { 11000,    ADC_CODE_7 }      //{65535, 11000}                             // 10000Pa
};

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
 * 名    称： App_AdcToI
 * 功    能： 将ADC值转换成压强值
 * 入口参数： 无
 * 出口参数： 无
 * 作　 　者： 无名沈
 * 创建日期： 2015-12-09
 * 修    改：
 * 修改日期：
 *******************************************************************************/
float App_AdcToI(u16 adc, u8 ch)
{
    float         I       = 0.0;
    double   k,b;     
    /***********************************************
    * 描述： 以最大和最小点为斜率计算点
    */
    if ( (Ctrl.Para.dat.Check[ch].K == 0) || (Ctrl.Para.dat.Check[ch].B == 0) ) {
        k   = (double)(Ctrl.Para.dat.Check[ch].I2 - Ctrl.Para.dat.Check[ch].I1)
            / (double)(Ctrl.Para.dat.Check[ch].Adc2 - Ctrl.Para.dat.Check[ch].Adc1);
        b   = (double)(Ctrl.Para.dat.Check[ch].I1
            - k * (double)Ctrl.Para.dat.Check[ch].Adc1);
        Ctrl.Para.dat.Check[ch].K   = k;
        Ctrl.Para.dat.Check[ch].B   = b;
    }
    /***********************************************
    * 描述： 获取斜率，计算当前压强
    */
    I       = (double)Ctrl.Para.dat.Check[ch].K * (double)adc 
            + (double)Ctrl.Para.dat.Check[ch].B;    // 放大1倍
        
    return I;
}

/*******************************************************************************
 * 名    称： App_AdcToPressure
 * 功    能： 将ADC值转换成压强值
 * 入口参数： 无
 * 出口参数： 无
 * 作　 　者： 无名沈
 * 创建日期： 2015-12-09
 * 修    改：
 * 修改日期：
 *******************************************************************************/
float App_IToPressure(float I, u8 ch)
{
    float  p       = 0;
    
    INT08U  nbr     = (INT08U)Ctrl.Sen.Para.dat.PINbr;
    INT08U  iMax    = nbr - 1;
    /***********************************************
    * 描述： 做分段处理
    */
    if ( ( nbr > 2 ) && ( nbr < 7  ) ) {
        static char i   = 1;
        /***********************************************
        * 描述： 当前值 <= 最小值
        */
        if ( I <= Ctrl.Sen.Para.dat.PI[ch][0][1] ) {
            i   = 1;
        /***********************************************
        * 描述： 当前值 >= 最大值
        */
        } else if ( I >= Ctrl.Sen.Para.dat.PI[ch][iMax][1] ) {
            i   = iMax;
        /***********************************************
        * 描述： 最小值 < 当前值 < 最大值
        */
        } else { 
            /***********************************************
            * 描述： 向上寻找的值 >= 当前值 >= 上次测量值
            */
            if ( I >= Ctrl.Sen.Para.dat.PI[ch][i][1] ) {
                while( I >= Ctrl.Sen.Para.dat.PI[ch][++i][1] );
            /***********************************************
            * 描述： 向下寻找的值 <= 当前值 <= 上次测量值
            */
            } else if ( I <= Ctrl.Sen.Para.dat.PI[ch][i][1] ) {
                while( I <= Ctrl.Sen.Para.dat.PI[ch][--i][1] );
            }
        }
        /***********************************************
        * 描述： 防止数据溢出
        */        
        if ( i < 1 ) {
            i   = 1;
        } else if ( i > iMax ) {
            i   = iMax;
        }
        
        /***********************************************
        * 描述： 
        */  
        double   k   = (double)(Ctrl.Sen.Para.dat.PI[ch][i][0] - Ctrl.Sen.Para.dat.PI[ch][i-1][0])
                      / (double)(Ctrl.Sen.Para.dat.PI[ch][i][1] - Ctrl.Sen.Para.dat.PI[ch][i-1][1]);
        double   b   = (double)(Ctrl.Sen.Para.dat.PI[ch][i-1][0] )
                      - k * (double)Ctrl.Sen.Para.dat.PI[ch][i-1][1];
        /***********************************************
        * 描述： 获取斜率，计算当前压强
        */
        p       = (float)(k * I + b);
    } else {
        /***********************************************
        * 描述： 以最大和最小点为斜率计算点
        */
        if ( ( Ctrl.Para.dat.SenChk[ch].K == 0 ) || ( Ctrl.Para.dat.SenChk[ch].B == 0 ) ) {
            double   k   = (double)(Ctrl.Para.dat.SenChk[ch].P2 - Ctrl.Para.dat.SenChk[ch].P1)
                          / (double)(Ctrl.Para.dat.SenChk[ch].I2 - Ctrl.Para.dat.SenChk[ch].I1);
            double   b   = (double)(Ctrl.Para.dat.SenChk[ch].P1
                          - k * (double)Ctrl.Para.dat.SenChk[ch].I1);
            Ctrl.Para.dat.SenChk[ch].K   = (float)k;
            Ctrl.Para.dat.SenChk[ch].B   = (float)b;
        }
        /***********************************************
        * 描述： 获取斜率，计算当前压强
        */
        p       = (float)(((double)Ctrl.Para.dat.SenChk[ch].K * (double)I 
                + (double)Ctrl.Para.dat.SenChk[ch].B));    // 放大1倍    
    }
    
    return p;  
}

/*******************************************************************************
 * 名    称： App_AdcToPressure
 * 功    能： 将ADC值转换成压强值
 * 入口参数： 无
 * 出口参数： 无
 * 作　 　者： 无名沈
 * 创建日期： 2015-12-09
 * 修    改：
 * 修改日期：
 *******************************************************************************/
float App_AdcToPressure(u16 adc, float *I, u16 nbr, u8 ch)
{
    /***********************************************
    * 描述： 校准采集板，验证用，如果作用不大，则去掉
    */
    if ( Ctrl.Para.dat.Sel.udat.CalcSel == 0 ) {
                    *I        = App_AdcToI(adc,ch);
        float      Pressure = App_IToPressure(*I,ch);
        return Pressure;
    } else {
                    *I        = App_AdcToI(adc,ch);
        /***********************************************
        * 描述： 
        */
        float       p       = 0;
        double   k,b;
        /***********************************************
        * 描述： 以最大和最小点为斜率计算点
        */        
        //k   = (double)(Ctrl.Para.dat.Check[ch].P2 - Ctrl.Para.dat.Check[ch].P1)
        //    / (double)(Ctrl.Para.dat.Check[ch].Adc2 - Ctrl.Para.dat.Check[ch].Adc1);
        //b   = (double)(Ctrl.Para.dat.Check[ch].P1 )
        //    - (double)Ctrl.Para.dat.Check[ch].Adc1 * k;
        
        k   = (double)(10000 - 0)
            / (double)(59578 - 11916);
        b   = (double)(0 )
            - k * (double)11916;
        /***********************************************
        * 描述： 获取斜率，计算当前压强
        */
        p       = (float)((k * (double)adc + b));    // 放大1倍
            
        return p;  
    }
}

/*******************************************************************************
 * 名    称： App_AdcToPressure
 * 功    能： 将ADC值转换成压强值
 * 入口参数： 无
 * 出口参数： 无
 * 作　 　者： 无名沈
 * 创建日期： 2015-12-09
 * 修    改：
 * 修改日期：
 *******************************************************************************/
float App_HightToVolume(float h)
{
    float  v       = 0;
    
    INT08U  nbr     = (INT08U)Ctrl.VTab->Tab.Cnts;
    INT08U  iMax    = nbr - 1;
    /***********************************************
    * 描述： 做分段处理
    */
    if ( ( nbr > 1 ) && ( nbr < 50  ) ) {
        char i   = 0;
        /***********************************************
        * 描述： 当前值 <= 最小值
        */
        if ( h <= Ctrl.VTab->Tab.Point[0].Hight ) {
            i   = 0;
        /***********************************************
        * 描述： 当前值 >= 最大值
        */
        } else if ( h >= Ctrl.VTab->Tab.Point[iMax].Hight ) {
            i   = iMax;
        /***********************************************
        * 描述： 最小值 < 当前值 < 最大值
        */
        } else { 
            /***********************************************
            * 描述： 向上寻找的值 > 当前值 > 上次测量值
            */
            if ( h >= Ctrl.VTab->Tab.Point[i].Hight ) {
                while( h >= Ctrl.VTab->Tab.Point[++i].Hight );
            /***********************************************
            * 描述： 向下寻找的值 < 当前值 < 上次测量值
            */
            } else if ( h <= Ctrl.VTab->Tab.Point[i].Hight ) {
                while( h <= Ctrl.VTab->Tab.Point[--i].Hight );
            }
        }
        /***********************************************
        * 描述： 防止数据溢出
        */        
        if ( i < 1 ) {
            i   = 1;
        } else if ( i > iMax ) {
            i   = iMax;
        }
        
        /***********************************************
        * 描述： 
        */  
        double   k   = (double)(Ctrl.VTab->Tab.Point[i].Volume - Ctrl.VTab->Tab.Point[i-1].Volume)
                      / (double)(Ctrl.VTab->Tab.Point[i].Hight - Ctrl.VTab->Tab.Point[i-1].Hight);
        double   b   = (double)(Ctrl.VTab->Tab.Point[i-1].Volume )
                      - k * (double)Ctrl.VTab->Tab.Point[i-1].Hight;
        /***********************************************
        * 描述： 获取斜率，计算当前压强
        */
        v       = (float)(k * h + b);
        if ( v < 0 )
            v   = 0;
    } else {
        v   = 0;
        /***********************************************
        * 描述： 以最大和最小点为斜率计算点
        *
        if ( Ctrl.Para.dat.SenChk[ch].K == 0 ) {
            double   k   = (double)(Ctrl.Para.dat.SenChk[ch].P2 - Ctrl.Para.dat.SenChk[ch].P1)
                / (double)(Ctrl.Para.dat.SenChk[ch].I2 - Ctrl.Para.dat.SenChk[ch].I1);
            double   b   = (double)(Ctrl.Para.dat.SenChk[ch].P1
                                    - k * (double)Ctrl.Para.dat.SenChk[ch].I1);
            Ctrl.Para.dat.SenChk[ch].K   = k;
            Ctrl.Para.dat.SenChk[ch].B   = b;
        }
        *//***********************************************
        * 描述： 获取斜率，计算当前压强
        */
        //v       = (float)(((double)Ctrl.Para.dat.SenChk[ch].K * (double)I + (double)Ctrl.Para.dat.SenChk[ch].B));    // 放大1倍    
    }
    
    return v;  
}

/*******************************************************************************
 * 				                    end of file                                *
 *******************************************************************************/
