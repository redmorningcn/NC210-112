/*******************************************************************************
 *   Filename:       app_comm_protocol.h
 *   Revised:        $Date: 2015-12-07$
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
#ifndef	APP_COMM_PROTOCOL_H
#define	APP_COMM_PROTOCOL_H

#ifdef __cplusplus
extern "C" {
#endif

/*******************************************************************************
 * INCLUDES
 */
#include <includes.h>

/*******************************************************************************
 * CONSTANTS
 */

/*******************************************************************************
 * MACROS
 */
#ifdef   APP_GLOBALS
#define  APP_EXT
#else
#define  APP_EXT  extern
#endif
    
/*******************************************************************************
 * TYPEDEFS
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
 *
 * 返回类型         函数名称                入口参数
 */
unsigned char      CSNR_GetData        ( unsigned char *RecBuf,
                                          unsigned char   RecLen,
                                          unsigned char  *DataBuf,
                                          unsigned short *InfoLen);

void	            DataSend_CSNR       ( unsigned char   SourceAddr,
                                          unsigned char   DistAddr,
                                          unsigned char  *DataBuf,
                                          unsigned short  DataLen);

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