#ifndef	BSP_PID_H
#define	BSP_PID_H
/*******************************************************************************
 *   Filename:       bsp_pid.h
 *   Revised:        $Date: 2013-11-08	20:15 (Fri) $
 *   Revision:       $
 *	 Writer:		 Wuming Shen.
 *
 *   Description:
 *
 *   Notes:
 *					QQ:276193028
 *     				E-mail:shenchangwei945@163.com
 *
 *   All copyrights reserved to Wuming Shen.
 *
 *******************************************************************************/

/*******************************************************************************
 * INCLUDES
 */
 #include  <cpu.h>

/*******************************************************************************
 * CONSTANTS
 */
#define PID_POLARITY_BIPOLAR            0           // 单极性
#define PID_POLARITY_POSITIVE           0           // 正极性
#define PID_POLARITY_NEGATIVE           1           // 负极性
#define PID_POLARITY_UNIPOLAR           2           // 双极性

#define PID_TYPE_DIGITAL                0           // 数字
#define PID_TYPE_ANALOG                 1           // 模拟
/*******************************************************************************
 * MACROS
 */

/*******************************************************************************
 * TYPEDEFS
 */
 /***********************************************
 * 描述：PID数据类型定义
 */
typedef struct _PID {
    // 控制项
	CPU_INT16S 		    Pv; 							// 过程量
	CPU_INT16S 		    Sv; 							// 设定值

	float 		        Kp; 							// 比例系数
	float 		        Ki; 							// 积分系数
	float 		        Kd; 							// 微分系数
	float 		        Integral; 						// 积分值 －－ 偏差累计值

	CPU_INT16S 		    Err; 							// 偏差值
	CPU_INT16S 		    LastErr; 						// 上次偏差值

	CPU_INT08U 		    DeadBand;    					// 死区
    CPU_INT08U          PidEnBand;                      // 开始PID调节下限值

    // 配置项
    CPU_INT16S          MinPv;                          // 过程量最小值
    CPU_INT16S          MaxPv;                          // 过程量最大值
    CPU_INT16S          HiLimit;                        // 设定值最小值
    CPU_INT16S          LoLimit;                        // 设定值最大值
    CPU_INT16S          MinOut;                         // 输出最小值（占空比或输出幅度）
    CPU_INT16S          MaxOut;                         // 输出最大值（调节周期或输出幅度）
    CPU_INT16U          Ti;                             // 积分时间
    CPU_INT16U          Td;                             // 微分时间
    CPU_INT16U          T;                              // 采样周期

    CPU_BOOLEAN         InType              : 1;         // 数字量或模拟量输入
    CPU_BOOLEAN         InPolarity          : 2;         // 模拟量输入极性
    CPU_BOOLEAN         OutType             : 1;         // 数字量或模拟量输出
    CPU_BOOLEAN         OutPolarity         : 2;         // 模拟量输出极性
    CPU_BOOLEAN         AlarmLFlag          : 1;         // PV低报警
    CPU_BOOLEAN         AlarmHFlag          : 1;         // PV高报警


    CPU_INT16S          Ctr;                             // 打开计数器
    CPU_INT16S          OnDly;                           // 打开计数器
    CPU_INT16S          OffDly;                          // 关闭计数器

    CPU_INT08S          POut;                            // 输出比例（-100 ~ 100）

} StrPid;

extern StrPid   *pid;
/*******************************************************************************
 * GLOBAL VARIABLES
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
void                PID_Init                (StrPid         *HeatPid,
                                             int            pv,
                                             int            sv);
void                PID_Tune                (StrPid         *pid,
                                             float          p_gain,
                                             float          i_gain,
                                             float          d_gain,
                                             float          integral,
                                             CPU_INT16S     dead_band,
                                             CPU_INT16S     MaxIn,
                                             CPU_INT16S     MaxOut,
                                             CPU_INT16S     PidBand);
void                PID_SetInteral          (StrPid         *pid,
                                             float          new_integral);
void                PID_Bumpless            (StrPid         *pid);
float               PID_Calc                (StrPid         *pid);
void                BSP_PID_Main            (void);
/*******************************************************************************
 * 				end of file
 *******************************************************************************/
#endif	/* GLOBLES_H */