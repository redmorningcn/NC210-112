/*******************************************************************************
 *   Filename:       osal_cpu.h
 *   Revised:        $Date: 2013-04-17 $
 *   Revision:       $
 *   Writer:		 Wuming Shen.
 *
 *   Description:
 *
 *   Notes:
 *
 *
 *   All copyrights reserved to Wuming Shen.  现代虚拟仪器仪表研究所
 *
 *******************************************************************************/
#ifndef OSAL_CPU_H
#define	OSAL_CPU_H

#ifdef	__cplusplus
extern "C" {
#endif
/*******************************************************************************
 * INCLUDES
 */
#include <global.h>

/*******************************************************************************
 * CONSTANTS
 */
#define ZSUCCESS                  0
#define INVALID_TASK              1
#define MSG_BUFFER_NOT_AVAIL      2
#define INVALID_MSG_POINTER       3
#define INVALID_LEN               4
#define INVALID_SENDING_TASK      5
#define INVALID_DESTINATION_TASK  6
#define INVALID_EVENT_ID          7
#define INVALID_TIMEOUT_VALUE     8
#define INVALID_INTERRUPT_ID      9
#define INVALID_ISR_PTR           10
#define INVALID_TASK_TABLE        11
#define NO_TIMER_AVAIL            12
#define INVALID_MSG_LEN           13
#define NOT_READY                 14
#define NV_ITEM_UNINIT            15
#define NV_OPER_FAILED            16
#define INVALID_MEM_SIZE          17
#define NV_BAD_ITEM_LEN           18

/*******************************************************************************
 * TYPEDEFS
 */
typedef unsigned char       byte;
typedef unsigned char       halDataAlign_t;
typedef unsigned int        osalEvt;            // 定义事件数据类型
typedef unsigned int        osalSts;            // 定义状态数据类型
typedef unsigned int        osalFlg;            // 定义标志数据类型
typedef unsigned int        osalTid;            // 定义任务ID数据类型
typedef unsigned int        osalTime;           // 定义时间数据类型
typedef unsigned char       halIntState_t;

/*******************************************************************************
 * MACROS
 */
#ifndef FALSE
   #define FALSE 0
#endif

#ifndef ARRAY_NULL
   #define ARRAY_NULL '\0'
#endif

#ifndef TRUE
   #define TRUE 1
#endif

#ifndef OPEN
   #define OPEN 1
#endif

#ifndef CLOSE
   #define CLOSE 0
#endif

#ifndef NULL
   #define NULL    ((void*) 0 )
#endif

#ifndef HIGH
   #define HIGH 1
#endif

#ifndef LOW
   #define LOW 0
#endif


#define HAL_ENABLE_INTERRUPTS()         CPU_IntEn()         //__set_PRIMASK(0)
#define HAL_DISABLE_INTERRUPTS()        CPU_IntDis()        //__set_PRIMASK(1)
#define HAL_INTERRUPTS_ARE_ENABLED()    CPU_SR_Save()       //__get_PRIMASK()

   
#define SEI()                           HAL_ENABLE_INTERRUPTS()
#define CLI()                           HAL_DISABLE_INTERRUPTS()

#define OSAL_ENTER_CRITICAL()           CPU_CRITICAL_ENTER();   //{ cpu_sr = __get_PRIMASK(); CLI();}
#define OSAL_EXIT_CRITICAL()            CPU_CRITICAL_EXIT();    //{ cpu_sr ? CLI() : HAL_ENABLE_INTERRUPTS();}
//#define HAL_CRITICAL_STATEMENT(x)       { halIntState_t s; OSAL_ENTER_CRITICAL(s); x; OSAL_EXIT_CRITICAL(s); }


#define OS_ENTER_CRITICAL()             OSAL_ENTER_CRITICAL()
#define OS_EXIT_CRITICAL()              OSAL_EXIT_CRITICAL()

#define ENABLE_GLOBAL_INT()             SEI()
#define DISABLE_GLOBAL_INT()            CLI()

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

/*******************************************************************************
 * EXTERN FUNCTIONS
 */

/*******************************************************************************
 * 				     end of file                               *
 *******************************************************************************/
#ifdef	__cplusplus
}
#endif

#endif	/* OSAL_CPU_H */

