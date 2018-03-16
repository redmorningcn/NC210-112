/*******************************************************************************
 *   Filename:       osal.h
 *   Revised:        $Date: 2013-04-17 $
 *   Revision:       $
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
#ifndef OSAL_H
#define	OSAL_H

#ifdef	__cplusplus
extern "C" {
#endif
/*******************************************************************************
 * INCLUDES
 */
#include <global.h>
#include <stm32f10x_conf.h>
#include <osal_cpu.h>

/*******************************************************************************
 * CONSTANTS
 */
#ifndef UCOS_EN
#define  UCOS_EN        DEF_ENABLED
#endif
#ifndef OSAL_EN
#define  OSAL_EN        DEF_ENABLED
#endif
/***********************************************
* 描述：
*/
#define OSAL_MAX_TASKS              10          // 定义最大任务数
#define OSAL_MAX_TIMERS             20          // 定义最大定时器个数

#define OS_TASK_PRIO_HIGHEST        ( OSAL_MAX_TASKS -1 )   // 定义最高任务优先级
#define OS_TASK_PRIO_LOWEST         0           // 定义最低任务优先级

/***********************************************
* 描述：osal_memory
*/
#if OSAL_EN == DEF_ENABLED
#define OSAL_TIMER_EN               1
#define OSAL_EVENT_EN               1
#define OSAL_MEM_EN                 0
#else
#define OSAL_TIMER_EN               0
#define OSAL_EVENT_EN               0
#define OSAL_MEM_EN                 0
#endif

/***********************************************
* 描述：osal_timer
*/
#define OSAL_TICKS_PER_SEC          OS_TICKS_PER_SEC   // 定义一秒种时钟拍数
#define OSAL_TICK_TIME_MS           1

/*******************************************************************************
 * TYPEDEFS
 */
typedef struct {
    void        *next;
    INT16U      len;
    osalTid     dest_id;
} osal_msg_hdr_t;

typedef struct {
    osalEvt     event;
    osalSts     status;
} osal_event_hdr_t;

typedef void  *osal_msg_q_t;

/*******************************************************************************
 * MACROS
 */

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
 */

/*******************************************************************************
 * EXTERN VARIABLES
 */
extern INT32U   osal_system_clock;

/*******************************************************************************
 * EXTERN FUNCTIONS
 */

byte        osal_set_event              ( osalTid task_id, osalFlg event_flag );
byte        osal_init_system            ( void );
/*******************************************************************************
 * 				     end of file                                               *
 *******************************************************************************/
#ifdef	__cplusplus
}
#endif

#endif	/* OSAL_H */