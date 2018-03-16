/*******************************************************************************
 *   Filename:      bsp_shtxx.h
 *   Revised:       $Date: 2015-08-28
 *   Revision:      $
 *   Writer:        Wuming Shen.
 *
 *   Description:
 *
 *   Notes:
 *
 *
 *   All copyrights reserved to Wuming Shen.
 *
 *******************************************************************************/

#ifndef __BSP_SHTXX_H__
#define __BSP_SHTXX_H__
/*******************************************************************************
 * INCLUDES
 */
#include "stm32f10x.h"
#include "global.h"
#include "math.h"

/*******************************************************************************
 * CONSTANTS
 */
enum {TEMP,HUMI};

/*******************************************************************************
 * MACROS
 */

/*******************************************************************************
 * TYPEDEFS
 */

typedef struct
{
    float   Temp;
    float   Humi;
    float   DewPoint;
}SHT_DATA_TYPE;

/*******************************************************************************
 * GLOBAL VARIABLES
 */

/*******************************************************************************
 * GLOBAL FUNCTIONS
 */
extern SHT_DATA_TYPE SHT_Data;      //转换数据保存在该变量中

uint16  BSP_ShtMeasureOnce          (unsigned char mode , unsigned char step);
void    BSP_ShtxxInit               (void);

#endif
/*******************************************************************************
 *                        end of file                                          *
 *******************************************************************************/