/*******************************************************************************
 *   Filename:       app_calc.h
 *   Revised:        $Date: 2015-12-09$
 *   Revision:       $
 *	 Writer:		 Wuming Shen.
 *
 *   Description:
 *   Notes:
 *					QQ:276193028
 *     				E-mail:shenchangwei945@163.com
 *
 *   All copyrights reserved to Wuming Shen.
 *
 *******************************************************************************/
#ifndef	APP_CALC_H
#define	APP_CALC_H

#ifdef __cplusplus
extern "C" {
#endif

#ifdef   APP_GLOBALS
#define  APP_EXT
#else
#define  APP_EXT  extern
#endif
    
/*******************************************************************************
 * INCLUDES
 */
#include <includes.h>

/*******************************************************************************
 * CONSTANTS
 */     
/***********************************************
* 描述： 单位
*/
#define Pa                  *1
#define mA                  *1
#define V                   *1
     
/***********************************************
* 描述： 采样部分定义
*/
#define ADC_RES             65536.0
#define REF_VOL             3300.0
#define RES_SAMPLE          150.00
     
#define Rs                  RES_SAMPLE                  // 采样电阻
#define Ioff               (4.0)                        // 零点电流
#define Ifs                (20.0)                       // 满量程电流
#define Voff               (Ioff * Rs)                  // 零点电压
#define Vfs                (Ifs * Rs)                   // 满量各电压
#define Vref               (REF_VOL)                    // 参考电压
#define ADCres              ADC_RES                     // AD分辨率
#define ADCoff             (Voff / Vref * ADCres)       // 零点AD值
#define ADCs               (Is * Rs / Vref * ADCres)    // 采集AD值
#define Poff               (0.0Pa)                      // 起点压强
#define Pfs                (10000.0Pa)                  // 满量程压强
#define P                  (ADCs/ADCres*Vref/Vfs*Pfs)   // 压强
    
/***********************************************
* 描述：传感器分段数
*/
#define DIVS                5
#define STEP               ((Ifs-Ioff)/DIVS)
     
/***********************************************
* 描述：传感器分段查表数
*/
#define ADC_TO_PRESSURE_TAB_LEN     (1+DIVS+2)
     
/*******************************************************************************
 * MACROS
 */
    
/*******************************************************************************
 * TYPEDEFS
 */
typedef struct {
    float   Prassure;
    float   Adc;
} StrAdcToPressure;
    
extern StrAdcToPressure    ADC_TO_PRESSURE_TAB[];

typedef struct {
    float   I;
    float   Adc;
} StrAdcToI;
    
extern StrAdcToPressure    ADC_TO_PRESSURE_TAB[];
extern StrAdcToI           ADC_TO_I_TAB[];

/*******************************************************************************
 * LOCAL VARIABLES
 */

/*******************************************************************************
 * GLOBAL VARIABLES
 */
 
/*******************************************************************************
 * LOCAL FUNCTIONS
 */

/*******************************************************************************
 * GLOBAL FUNCTIONS
 *
 * 返回类型         函数名称                入口参数
 */
 float              App_AdcToPressure       (u16 adc, float *I, u16 nbr, u8 ch);
 float              App_HightToVolume       (float h);
 
/*******************************************************************************
 * EXTERN VARIABLES
 */

/*******************************************************************************
 * EXTERN FUNCTIONS
 */

/*******************************************************************************
 *              end of file                                                    *
 *******************************************************************************/
#ifdef __cplusplus
}
#endif
#endif