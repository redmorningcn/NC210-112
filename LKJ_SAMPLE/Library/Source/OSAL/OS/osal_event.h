/*******************************************************************************
 *   Filename:       osal_event.h
 *   Revised:        $Date: 2013-04-17 $
 *   Revision:       $
 *   Writer:         Wuming Shen.
 *
 *   Description:
 *
 *   Notes:
 *
 *
 *   All copyrights reserved to Wuming Shen.
 *
 *******************************************************************************/
#ifndef OSAL_EVENT_H
#define	OSAL_EVENT_H

#ifdef	__cplusplus
extern "C" {
#endif
/*******************************************************************************
 * INCLUDES
 */
#include    <osal.h>
#include    <os.h>

/*******************************************************************************
 * CONSTANTS
 */

/*******************************************************************************
 * TYPEDEFS
 */
typedef void        (*pTaskInitFn)( void );

typedef osalEvt     (*pTaskEventHandlerFn)(INT8U task_id,osalEvt task_event);

typedef struct OSALTaskREC {
    struct  OSALTaskREC      *next;
    pTaskInitFn               pfnInit;
    pTaskEventHandlerFn       pfnEventProcessor;
    osalTid                   taskID;
    INT8U                     taskPriority;
    osalEvt                   events;
} OsalTadkREC_t;

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
extern OsalTadkREC_t    *TaskActive;
/***********************************************
* 描述： OS接口
*/
#if (UCOS_EN == DEF_ENABLED ) && ( OS_VERSION > 30000U )
extern OS_SEM           Osal_EvtSem;               // 信号量
#endif

/*******************************************************************************
 * EXTERN FUNCTIONS
 */
void            osal_start_system       (void);
void            osal_add_Task           (pTaskInitFn pfnInit,
                                         pTaskEventHandlerFn pfnEventProcessor,
                                         INT8U taskPriority,INT8U id);
void            osal_Task_init          (void);
void            osal_init_TaskHead      (void);
OsalTadkREC_t   *osalNextActiveTask     (void);
OsalTadkREC_t   *osalFindTask           (osalTid taskID);

/*******************************************************************************
 * 				     end of file                               *
 *******************************************************************************/
#ifdef	__cplusplus
}
#endif

#endif	/* OSAL_EVENT_H */