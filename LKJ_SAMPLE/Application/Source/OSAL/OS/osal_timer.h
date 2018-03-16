/*******************************************************************************
 *   Filename:       osal_timer.h
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
#ifndef OSAL_TIMER_H
#define	OSAL_TIMER_H

#ifdef	__cplusplus
extern "C" {
#endif
/*******************************************************************************
 * INCLUDES
 */
#include <osal.h>

/*******************************************************************************
 * CONSTANTS
 */
#define OSAL_TIMERS_MAX_TIMEOUT 0xFFFF

/*******************************************************************************
 * TYPEDEFS
 */
typedef struct {
    void       *next;
    osalFlg     event_flag;
    osalTime    timeout;
    osalTid     task_id;
    osalTime    reload;
} osalTimerRec_t;

/*******************************************************************************
 * MACROS
 */
#define osal_update_timers  osalTimerUpdate

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
extern osalTimerRec_t  *timerHead;
extern osalTime         tmr_count;                  // Amount of time per tick - in micro-sec
extern INT8U            tmr_decr_time;              // Decr_Time for system timer
extern INT8U            timerActive;                // Flag if hw timer active
    
/*******************************************************************************
 * EXTERN FUNCTIONS
 */
void            osalTimerInit           ( void );
INT8U           osal_start_timerEx      ( osalTid task_id, osalEvt event_id, osalTime timeout_value );
INT8U           osal_start_timerRl      ( osalTid task_id, osalEvt event_id, osalTime timeout_value );
INT8U           osal_stop_timerEx       ( osalTid task_id, osalEvt event_id );
void            osal_update_timers      ( void );
void            osalTimerUpdate         ( void );

/*******************************************************************************
 * 				     end of file                               *
 *******************************************************************************/
#ifdef	__cplusplus
}
#endif
#endif	/* OSAL_TIMER_H */