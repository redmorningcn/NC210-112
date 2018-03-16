/*******************************************************************************
 *   Filename:       app_task_sen.c
 *   Revised:        All copyrights reserved to wumingshen.
 *   Revision:       v1.0
 *   Writer:	     wumingshen.
 *
 *   Description:    双击选中 sen 按 Ctrl + H, 钩选 Match the case, Replace with
 *                   输入您要的名字，点击 Replace All
 *                   双击选中 Sen 按 Ctrl + H, 钩选 Match the case, Replace with
 *                   输入您要的名字，点击 Replace All
 *                   双击选中 SEN 按 Ctrl + H, 钩选 Match the case, Replace with
 *                   输入您要的名字，点击 Replace All
 *                   在 app_cfg.h 中指定本任务的 优先级  （ APP_TASK_SEN_PRIO ）
 *                                            和 任务堆栈（ APP_TASK_SEN_STK_SIZE ）大小
 *                   在 app.h 中声明本任务的     创建函数（ void  App_TaskSenCreate(void) ）
 *                                            和 看门狗标志位 （ WDTFLAG_Sen ）
 *
 *   Notes:
 *     				E-mail: shenchangwei945@163.com
 *
 *******************************************************************************/

/*******************************************************************************
 * INCLUDES
 */
#define  SNL_APP_SOURCE
#include <includes.h>

#ifdef VSC_INCLUDE_SOURCE_FILE_NAMES
const  CPU_CHAR  *app_task_sen__c = "$Id: $";
#endif

#define APP_TASK_SEN_EN     DEF_ENABLED
#if APP_TASK_SEN_EN == DEF_ENABLED
/*******************************************************************************
 * CONSTANTS
 */
#define CYCLE_TIME_TICKS            (OS_CFG_TICK_RATE_HZ * 2u)
#define CYCLE_SAMPLE_MSEC_TICKS     (OS_CFG_TICK_RATE_HZ / ADC_SAMPLE_CNTS_PER_SEC/2)
#define CYCLE_SAMPLE_SEC_TICKS      (OS_CFG_TICK_RATE_HZ * 5u)
#define CYCLE_SAMPLE_MIN_TICKS      (OS_CFG_TICK_RATE_HZ * 60u)
#define CYCLE_SAMPLE_TICKS          (OS_CFG_TICK_RATE_HZ * 2u)
#define CYCLE_UPDATE_DENSITY        (OS_CFG_TICK_RATE_HZ * 3u*60u)
#define CYCLE_UPDATE_TICKS          (OS_CFG_TICK_RATE_HZ * 30u)                 // 密度检测周期

#define STOP_UPDATE_TICKS_CNT       (5 * 60 / (CYCLE_UPDATE_TICKS / OS_CFG_TICK_RATE_HZ))   // 停车检测时间间隔

#define CYCLE_UPDATE_TICKS_CNT      (30 * 60 * CYCLE_UPDATE_TICKS / CYCLE_UPDATE_TICKS)     // 静止检测时间间隔

#define CYCLE_SEN_ERR_CHK           (OS_CFG_TICK_RATE_HZ * 2u)                  // 传感器故障检测传感器
     
/***********************************************
* 描述： 高度差范围限定

主要城市重力加速度（m/s2）
广州        9.788     米/秒^2。
武汉        9.794     米/秒^2。
上海        9.794     米/秒^2。
东京        9.798     米/秒^2。
北京        9.801     米/秒^2。
纽约        9.803     米/秒^2。
莫斯科      9.816     米/秒^2。
北极地区    9.832     米/秒^2。[6] 

各纬度海平面的重力加速度（m/s2）
纬度        重力加速度
0           9.78030
10          9.78186
20          9.78634
30          9.79321
40          9.80166
50          9.81066
60          9.81914
70          9.82606
80          9.83058
90          9.83218

不同高度的重力加速度 (m/s^2）
海拔 (km)   纬度（度）　	
            0       10      20      30      40      50      60      70      80      90　       
0           9.780   9.782   9.786   9.793   9.802   9.811   9.819   9.826   9.831   9.832　
4           9.768   9.770   9.774   9.781   9.789   9.798   9.807   9.814   9.818   9.820　
8           9.756   9.757   9.762   9.768   9.777   9.786   9.794   9.801   9.806   9.807　
12          9.743   9.745   9.749   9.756   9.765   9.774   9.782   9.789   9.794   9.795　
16          9.731   9.732   9.737   9.744   9.752   9.761   9.770   9.777   9.781   9.783　
20          9.719   9.720   9.725   9.732   9.740   9.749   9.757   9.764   9.769   9.770
　
注：如果上升高度不大,则每升1km，g 减少0.03%。
重力加速度g不同单位制之间的换算关系为：重力加速度g = 9.81m/s^2;= 981cm/s^2; = 32.18ft/s^2;

全国各地区重力加速度表
        序号                                      地区	   力加速度       地区修正值
                                    g(m/s2)	                  g/1kg	       g/3kg	  g/6kg	     g/15kg	      g/30kg
*/
#define GRAVITY_01	                9.7986  	// 包头	    -0.3981      -1.1943 	-2.3886    -11.9430 	-11.9430 
#define GRAVITY_02	                9.8015  	// 北京	    -0.7045      -2.1135 	-4.2270    -10.5675 	-21.1350 
#define GRAVITY_03	                9.8048  	// 长春	    -1.0413      -3.1239 	-6.2478    -15.6195 	-31.2390 
#define GRAVITY_04	                9.7915  	// 长沙	     0.3267       0.9801 	 1.9602 	 9.8010 	  9.8010 
#define GRAVITY_05	                9.7913  	// 成都	     0.3267       0.9801 	 1.9602 	 4.9005 	  9.8010 
#define GRAVITY_06	                9.7914  	// 重庆	     0.3267       0.9801 	 1.9602 	 4.9005 	  9.8010 
#define GRAVITY_07	                9.8011  	// 大连	    -0.6636      -1.9908 	-3.9816 	-9.9540 	-19.9080 
#define GRAVITY_08	                9.7833  	// 广州	     0.6432    	  1.9296 	 3.8592 	 9.6480 	 19.2960 
#define GRAVITY_09	                9.7968  	// 贵阳	     0.7963       2.3889 	 4.7778 	23.8890 	 23.8890 
#define GRAVITY_10	                9.8066  	// 哈尔滨	-1.2251      -3.6753 	-7.3506    -18.3765 	-36.7530 
#define GRAVITY_11	                9.7936  	// 杭州   	 0.1020       0.3060 	 0.6120 	 1.5300 	  3.0600 
#define GRAVITY_12	                9.7863  	// 海口   	 0.8474       2.5422 	 5.0844 	25.4220 	 25.4220 
#define GRAVITY_13	                9.7947  	// 合肥   	 0.0204       0.0612 	 0.1224 	 0.3060 	  0.6120 
#define GRAVITY_14	                9.8048  	// 吉林   	-1.0413      -3.1239 	-6.2478    -15.6195 	-31.2390 
#define GRAVITY_15	                9.7988  	// 济南   	-0.3981      -1.1943 	-2.3886 	-5.9715 	-11.9430 
#define GRAVITY_16	                9.7830  	// 昆明   	 1.1230       3.3690 	 6.7380 	16.8450 	 33.6900 
#define GRAVITY_17	                9.7799  	// 拉萨   	 0.5513       1.6539 	 3.3078 	16.5390 	 16.5390 
#define GRAVITY_18	                9.7920  	// 南昌   	 0.2654       0.7962 	 1.5924 	 7.9620 	  7.9620 
#define GRAVITY_19	                9.7949  	// 南京   	-0.0306      -0.0918 	-0.1836 	-0.4590 	  0.9180 
#define GRAVITY_20	                9.7877  	// 南宁    	 0.7044       2.1132 	 4.2264 	10.5660 	 21.1320 
#define GRAVITY_21	                9.7985  	// 青岛   	-0.3981      -1.1943 	-2.3886 	-5.9715 	-11.9430 
#define GRAVITY_22	                9.7964  	// 上海   	 0.0000       0.0000 	 0.0000 	 0.0000 	  0.0000 
#define GRAVITY_23	                9.8035  	// 沈阳   	-0.9086      -2.7258 	-5.4516    -13.6290 	-27.2580 
#define GRAVITY_24	                9.7997  	// 石家庄	-0.5513      -1.6539 	-3.3078 	-8.2695 	-16.5390 
#define GRAVITY_25	                9.7970  	// 太原   	-0.2450      -0.7350 	-1.4700 	-3.6750 	 -7.3500 
#define GRAVITY_26	                9.8011  	// 天津   	-0.6636      -1.9908 	-3.9816 	-9.9540 	-19.9080 
#define GRAVITY_27	                9.7936  	// 武汉   	 0.1020       0.3060 	 0.6120 	 1.5300 	  3.0600 
#define GRAVITY_28	                9.8015  	// 乌鲁木齐 -0.7248      -2.1744 	-4.3488    -21.7440 	-21.7440 
#define GRAVITY_29	                9.7944  	// 西安   	 0.0204       0.0612 	 0.1224 	 0.3060 	  0.6120 
#define GRAVITY_30	                9.7911  	// 西宁   	 0.3267       0.9801 	 1.9602 	 9.8010 	  9.8010 
#define GRAVITY_31	                9.8000  	// 张家口	-0.5513      -1.6539 	-3.3078 	-8.2695 	-16.5390 
#define GRAVITY_32	                9.7966  	// 郑州   	-0.2041      -0.6123 	-1.2246 	-3.0615 	 -6.1230 


#define GRAVITY_CHANGSHA            GRAVITY_04  // 长沙
#define GRAVITY_WUHAN               GRAVITY_27  // 武汉
#define GRAVITY_CHENGDU             GRAVITY_05  // 成都
#define GRAVITY_JINAN               GRAVITY_15  // 济南
#define GRAVITY_BEIJING             GRAVITY_02  // 北京
#define GRAVITY_SHENYANG            GRAVITY_23  // 沈阳

#define GRAVITY_MIN                 9.768
#define GRAVITY_MAX                 9.832
//#define GRAVITY_DEF                 GRAVITY_CHANGSHA  // 长沙
//#define GRAVITY_DEF                 GRAVITY_WUHAN     // 武汉
//#define GRAVITY_DEF                 GRAVITY_CHENGDU   // 成都
#define GRAVITY_DEF                 GRAVITY_JINAN     // 济南
//#define GRAVITY_DEF                 GRAVITY_BEIJING   // 北京
//#define GRAVITY_DEF                 GRAVITY_SHENYANG  // 沈阳

/***********************************************
* 描述： 柴油密度范围限定
*
柴油密度 编辑
通常国标柴油的密度范围为 0.83～0.855，不同型号的密度不同。如：
  0#柴油密度 0.835
+10#柴油密度 0.85
+20#柴油密度 0.87
-10#柴油密度 0.84
-20#柴油密度 0.83
-30#柴油密度 0.82
-35#柴油密度 0.82

通常柴油密度以 0.84 计算，
这样一吨柴油大约折合1190升。
中文名 柴油密度 密度范围 0.82～0.86
作    用 测量 
应    用 化工 裂化  催化裂化，加氢裂化，减粘裂化  
碳原子数 10-22
*/
#define OIL_DENSITY_MIN             0.8200
#define OIL_DENSITY_MAX             0.8600
#define OIL_DENSITY_DEF             0.8500

/***********************************************
* 描述： 水密度范围限定
*/
#define WARTER_DENSITY_MIN          0.9500
#define WARTER_DENSITY_MAX          1.1000
#define WARTER_DENSITY_DEF          1.0000

/***********************************************
* 描述： 高度差范围限定
*/
#define DIFF_HIGHT_MIN              750
#define DIFF_HIGHT_MAX              850
#define DIFF_HIGHT_DEF              800

/*******************************************************************************
 * MACROS
 */

/*******************************************************************************
 * TYPEDEFS
 */

/*******************************************************************************
 * LOCAL VARIABLES
 */

#if ( OSAL_EN == DEF_ENABLED )
#else
/***********************************************
* 描述： 任务控制块（TCB）
*/
static  OS_TCB   AppTaskSenTCB;

/***********************************************
* 描述： 任务堆栈（STACKS）
*/
static  CPU_STK  AppTaskSenStk[ APP_TASK_SEN_STK_SIZE ];

#endif
/*******************************************************************************
 * GLOBAL VARIABLES
 */

/*******************************************************************************
 * LOCAL FUNCTIONS
 */
#if ( OSAL_EN == DEF_ENABLED )
#else
static  void    AppTaskSen           (void *p_arg);
#endif

float           App_fParaFilter     (float para, float def, float min, float max);
long            App_lParaFilter     (long para, long def, long min, long max);

/*******************************************************************************
 * GLOBAL FUNCTIONS
 */

/*******************************************************************************
 * EXTERN VARIABLES
 */

/*******************************************************************************/

/*******************************************************************************
 * 名    称： App_TaskSenCreate
 * 功    能： **任务创建
 * 入口参数： 无
 * 出口参数： 无
 * 作　 　者： wumingshen.
 * 创建日期： 2015-02-05
 * 修    改：
 * 修改日期：
 * 备    注： 任务创建函数需要在app.h文件中声明
 *******************************************************************************/
void  App_TaskSenCreate(void)
{

#if ( OSAL_EN == DEF_ENABLED )
#else
    OS_ERR  err;

    /***********************************************
    * 描述： 任务创建
    */
    OSTaskCreate((OS_TCB     *)&AppTaskSenTCB,                  // 任务控制块  （当前文件中定义）
                 (CPU_CHAR   *)"App Task Sen",                  // 任务名称
                 (OS_TASK_PTR ) AppTaskSen,                     // 任务函数指针（当前文件中定义）
                 (void       *) 0,                              // 任务函数参数
                 (OS_PRIO     ) APP_TASK_SEN_PRIO,              // 任务优先级，不同任务优先级可以相同，0 < 优先级 < OS_CFG_PRIO_MAX - 2（app_cfg.h中定义）
                 (CPU_STK    *)&AppTaskSenStk[0],               // 任务栈顶
                 (CPU_STK_SIZE) APP_TASK_SEN_STK_SIZE / 10,     // 任务栈溢出报警值
                 (CPU_STK_SIZE) APP_TASK_SEN_STK_SIZE,          // 任务栈大小（CPU数据宽度 * 8 * size = 4 * 8 * size(字节)）（app_cfg.h中定义）
                 (OS_MSG_QTY  ) 5u,                             // 可以发送给任务的最大消息队列数量
                 (OS_TICK     ) 0u,                             // 相同优先级任务的轮循时间（ms），0为默认
                 (void       *) 0,                              // 是一个指向它被用作一个TCB扩展用户提供的存储器位置
                 (OS_OPT      )(OS_OPT_TASK_STK_CHK |           // 允许堆栈检查该任务
                                OS_OPT_TASK_STK_CLR),           // 创建任务时堆栈清零
                 (OS_ERR     *)&err);                           // 指向错误代码的指针，用于创建结果处理
#endif
}

/*******************************************************************************
 * 名    称： AppTaskSen
 * 功    能： 控制任务
 * 入口参数： p_arg - 由任务创建函数传入
 * 出口参数： 无
 * 作　 　者： wumingshen.
 * 创建日期： 2015-02-05
 * 修    改：
 * 修改日期：
 *******************************************************************************/

#if ( OSAL_EN == DEF_ENABLED )
osalEvt  TaskSenEvtProcess(INT8U task_id, osalEvt task_event)
#else
static  void  AppTaskSen (void *p_arg)
#endif
{
    OS_ERR      err;
    
    /***********************************************
    * 描述： 本任务看门狗标志置位
    */
    OS_FlagPost ((OS_FLAG_GRP *)&WdtFlagGRP,
                 (OS_FLAGS     ) WDT_FLAG_SEN,
                 (OS_OPT       ) OS_OPT_POST_FLAG_SET,
                 (CPU_TS       ) 0,
                 (OS_ERR      *) &err);    
  
    /***************************************************************************
    *                               数据采集
    ***************************************************************************/
    
    /***************************************************************************
    * 描述： 一级采样滤波：100MS/2采样一次
    */
    if( task_event & OS_EVT_SEN_MSEC ) {
        static u8   step    = 0;
        static u8   ctrL    = 0;
        static u8   ctrH    = 0;
        
        static u16  ctr1   = 0;
        static u16  ctr2   = 0;
        
        /***********************************************
        * 描述： 采集高点值
        */
        if ( step )  {
            __StrSen    *pSen   = &Ctrl.Sen.H;              // 指向高点传感器
            double  AdcValue    = (double)BSP_ADCGetValue( 2 );
                        
            if ( pSen->AdcValue != 0 ) {
                pSen->AdcValue           = (u16)((double)pSen->AdcValue * 0.500 + (double)AdcValue * 0.500);
            } else {
                pSen->AdcValue           = (u16)AdcValue;
            }
            
            pSen->AdcFilter[ctrH] = pSen->AdcValue; 
            if ( ++ctrH >= ADC_SAMPLE_CNTS_PER_SEC ) {
                ctrH    = 0;
                /***********************************************
                * 描述： 启动秒计算事件
                */
                osal_set_event( OS_TASK_ID_SEN, OS_EVT_SEN_SEC);
            }
            
            Ctrl.Sen.Filter[0][ctr2]   = pSen->AdcValue;
            
            if ( ++ctr1 >= ADC_SAMPLE_CNT ) {
                ctr1   = 0;
                if ( ++ctr2 >= ADC_SAMPLE_CNTS_DEEPTH ) {
                    ctr2   = 0;
                }
            }
            /***********************************************
            * 描述： 采集低点值
            */
        } else {
            __StrSen    *pSen   = &Ctrl.Sen.L;              // 指向低点传感器
            double  AdcValue     = (double)BSP_ADCGetValue( 3 );
            pSen->AdcValue      = (u16)((double)pSen->AdcValue * 0.500 + (double)AdcValue * 0.5000);
            pSen->AdcFilter[ctrL] = pSen->AdcValue; 
            if ( ++ctrL >= ADC_SAMPLE_CNTS_PER_SEC ) {
                ctrL    = 0;  
            }
            
            Ctrl.Sen.Filter[1][ctr2]   = pSen->AdcValue;
        }
        
        step    = ~step;                                    // 切换采集通道
        /***********************************************
        * 描述： 
        */
        
        return ( task_event ^ OS_EVT_SEN_MSEC );
    }
    
    /***************************************************************************
    * 描述： 二级滤波：1秒钟滤波一次
    */
    if( task_event & OS_EVT_SEN_SEC ) {
        static u8   ctr = 0;
        u16         *After[ADC_SAMPLE_CNTS_PER_SEC];
        __StrSen    *pSen   = &Ctrl.Sen.H;              // 指向高点传感器
        
        for ( u8 sen = 0; sen < 2; sen++, pSen++ ) {
            /***********************************************
            * 描述： 高低位传感器计算
            */ 
            double  AdcValue    = 
                (double)App_GetFilterValue( (u16 *)&pSen->AdcFilter[0], 
                                           (u16 *)&After[0], 
                                           (u16  ) ADC_SAMPLE_CNTS_PER_SEC, 
                                           (u16  ) ADC_SAMPLE_CNTS_PER_SEC - 4);
            if ( pSen->AdcValueSec != 0 )
                pSen->AdcValueSec           = (u16)((double)pSen->AdcValueSec * 0.500 + (double)AdcValue * 0.500);
            else
                pSen->AdcValueSec           = (u16)AdcValue;
            pSen->AdcFilterSec[ctr]  = pSen->AdcValueSec;
        }
        
        if ( ++ctr  >= ADC_SAMPLE_SECS ) {
            ctr = 0;
            /***********************************************
            * 描述： 启动多秒计算事件
            */
            osal_set_event( OS_TASK_ID_SEN, OS_EVT_SEN_MIN);                
        }  
        
        return ( task_event ^ OS_EVT_SEN_SEC );
    }
    
    /***************************************************************************
    * 描述： 三四级滤波：1分钟滤波一次
    */
    if( task_event & OS_EVT_SEN_MIN ) {
        u16         *After[ADC_SAMPLE_SECS];
        __StrSen    *pSen   = &Ctrl.Sen.H;              // 指向高点传感器
        
#define AVER_TIME   30
#if AVER_TIME > ADC_SAMPLE_CNTS_MAX
#undef AVER_TIME
#define AVER_TIME   ADC_SAMPLE_CNTS_MAX
#endif
        
        /***********************************************
        * 描述： 高低位传感器计算
        */ 
        for( char sen = 0; sen < 2; sen++, pSen++ ) {
            u32 sum     = 0;
            u16 cnt     = 0;
            double  AdcValue    = 
                (double)App_GetFilterValue( (u16 *)&pSen->AdcFilterSec[0], 
                                            (u16 *)&After[0], 
                                            (u16  ) ADC_SAMPLE_SECS, 
                                            (u16  ) ADC_SAMPLE_SECS - 0);
            if ( pSen->AdcValueMin != 0 )
                pSen->AdcValueMin           = (u16)((double)pSen->AdcValueMin * 0.500 + (double)AdcValue * 0.500);
            else
                pSen->AdcValueMin           = (u16)AdcValue;
            // 将当前值写入缓冲区
            pSen->AdcFilterMin[pSen->Head]  = (u16)AdcValue;
            /***********************************************
            * 描述： 头指针在前面（缓冲区未满）
            */
            if ( pSen->Head > pSen->Tail ) {
                for ( int i  = pSen->Tail; 
                     ( i <= pSen->Head ) && ( cnt <= AVER_TIME ); 
                     i++, cnt++ ) {
                    sum    += pSen->AdcFilterMin[i];
                }
                pSen->AdcValueAll = sum / cnt; 
                /***********************************************
                * 描述： 头指针在后面（缓冲区已满）
                */
            } else if ( pSen->Head < pSen->Tail ) {
                for ( int i = 0; 
                    i < AVER_TIME; 
                    i++, cnt++ ) {
                    sum    += pSen->AdcFilterMin[i];
                }
                pSen->AdcValueAll = sum / cnt; 
                /***********************************************
                * 描述： 头尾指针相等，没数据
                */
            } else {
                pSen->AdcValueAll = 0;
            }
            
            if ( ++pSen->Head >= ADC_SAMPLE_CNTS_MAX ) {
                pSen->Head = 0; 
                if ( pSen->Tail >= pSen->Head )
                    pSen->Tail++; 
                
                if ( pSen->Tail == 0 )
                    pSen->Tail++;
            } else {
                if ( pSen->Tail >= pSen->Head )
                    pSen->Tail++; 
            }
            
            if ( pSen->Tail >= ADC_SAMPLE_CNTS_MAX )
                pSen->Tail = 0;
        }            
        
        return ( task_event ^ OS_EVT_SEN_MIN );
    }
    
    /***************************************************************************
    * 描述： 深度滤波：1秒钟滤波一次
    */
    if( task_event & OS_EVT_SEN_DEAL ) {
        __StrSen    *pSen   = &Ctrl.Sen.H;              // 指向高点传感器
        
        for ( u8 i  = 0; i < 2; i++, pSen++ ) {
            double  AdcValue  = 
           (double) App_GetFilterValue( (u16 *)&Ctrl.Sen.Filter[i][0], 
                                        NULL, 
                                       (u16  ) ADC_SAMPLE_CNTS_DEEPTH, 
                                       (u16  ) ADC_SAMPLE_CNTS_DEEPTH - 0);            
                pSen->AdcValueSample     = (u16  )AdcValue;
        }
        return ( task_event ^ OS_EVT_SEN_DEAL );
    }
    
    /***************************************************************************
    *                               数据计算
    ***************************************************************************/
    
    /***************************************************************************
    * 描述： 数据计算
    */
    if( task_event & OS_EVT_SEN_TICKS ) {
        //osal_set_event( OS_TASK_ID_SEN, OS_EVT_SEN_MIN);
        /***********************************************
        * 描述： 数值计算
        *        传感器参数
        *        采样电阻： 150R/1%/25ppm
        *        4~20MA     FS
        *        < 4MA      故障：断路
        *        > 20MA     故障：短路等
        *        4MA        = 0KP
        *        20MA       = 10KP
        *        标定压力： 0，   2，  4，  6，  8，  10
        *        标定ADC：  
        * 
        *        Rs     = 150/1%/25ppm                  // 采样电阻
        *        Ioff   = 4mA;                          // 零点电流
        *        Ifs    = 20mA;                         // 满量程电流
        *        Voff   = Ioff * Rs;                    // 零点电压
        *        Vfs    = Ifs * Rs;                     // 满量各电压
        *        Vref   = 3.3V;                         // 参考电压
        *        ADCres = 65536;                        // AD分辨率
        *        ADCoff = Voff / Vref * ADCres;         // 零点AD值
        *        ADCs   = Is * Rs / Vref * ADCres;      // 采集AD值
        *        Poff   = 0Pa;                          // 起点压强
        *        Pfs    = 10000Pa;                      // 满量程压强
        *        P      = ADCs/ADCres*Vref/Vfs*Pfs;     // 压强
        *        h      = P/ρg;                        // 高度       
        *      Δh      = hL - hH;                      // 高度差            
        *        y      = kx + b;                       // y:高度；xADC值；k:斜率；b:偏移  
        *        x1     = (ADCoff) 
        *        y1     = (Poff)      
        *        x2     = (ADCfs)  
        *        y2     = (Pfs)    
        *        k      = (y2 - y1)/(x2 - x1))
        *        b      = (y1 - k * x1)
        *        y      = kx + b; 
        */ 
        __StrSen    *pSen   = &Ctrl.Sen.H;              // 指向高点传感器
        
        /***********************************************
        * 描述： 1）电流、压强电、电压计算
        */
        for ( u8 sen = 0; sen < 2; sen++, pSen++ ) {
            /***********************************************
            * 描述： 电流、压强计算 4~20mA、0~10000pa
            */
            pSen->Pressure              = App_AdcToPressure( pSen->AdcValue, 
                                                            (float *)&(pSen->Current), 
                                                            ADC_TO_PRESSURE_TAB_LEN, sen );
            pSen->PressureSec           = App_AdcToPressure( pSen->AdcValueSec, 
                                                            (float *)&(pSen->CurrentSec), 
                                                            ADC_TO_PRESSURE_TAB_LEN, sen );
            pSen->PressureMin           = App_AdcToPressure( pSen->AdcValueMin, 
                                                            (float *)&(pSen->CurrentMin), 
                                                            ADC_TO_PRESSURE_TAB_LEN, sen );
            pSen->PressureAll           = App_AdcToPressure( pSen->AdcValueAll, 
                                                            (float *)&(pSen->CurrentAll), 
                                                            ADC_TO_PRESSURE_TAB_LEN, sen );
            pSen->PressureSample        = App_AdcToPressure( pSen->AdcValueSample, 
                                                            (float *)&(pSen->CurrentSample), 
                                                            ADC_TO_PRESSURE_TAB_LEN, sen );
            
            /***********************************************
            * 描述： 电压计算
            */
            pSen->Voltage               = (INT16S)(REF_VOL * (double)pSen->AdcValue        / ADC_RES);
            pSen->VoltageSec            = (INT16S)(REF_VOL * (double)pSen->AdcValueSec     / ADC_RES);
            pSen->VoltageMin            = (INT16S)(REF_VOL * (double)pSen->AdcValueMin     / ADC_RES);
            pSen->VoltageAll            = (INT16S)(REF_VOL * (double)pSen->AdcValueAll     / ADC_RES);
            pSen->VoltageSample         = (INT16S)(REF_VOL * (double)pSen->AdcValueSample  / ADC_RES);
        }
        
        /***********************************************
        * 描述： 压差计算
        */
        Ctrl.Sen.C.DiffPressure         = Ctrl.Sen.L.Pressure         - Ctrl.Sen.H.Pressure;
        Ctrl.Sen.C.DiffPressureSec      = Ctrl.Sen.L.PressureSec      - Ctrl.Sen.H.PressureSec;
        Ctrl.Sen.C.DiffPressureMin      = Ctrl.Sen.L.PressureMin      - Ctrl.Sen.H.PressureMin;
        Ctrl.Sen.C.DiffPressureAll      = Ctrl.Sen.L.PressureAll      - Ctrl.Sen.H.PressureAll;
        Ctrl.Sen.C.DiffPressureSample   = Ctrl.Sen.L.PressureSample   - Ctrl.Sen.H.PressureSample;
        
        /***********************************************
        * 描述： 密度选择
        */
        static float LastDensity    = 0.0;
        if ( LastDensity == 0.0 )
            LastDensity = Ctrl.Para.dat.LastDensity;
        
        switch ( Ctrl.Para.dat.Sel.udat.DensitySel ) {
        case 0:                                         // 选择设置密度
            {
            float   temp    = Ctrl.Para.dat.LastDensity - LastDensity;
            if ( temp != 0 ) {
                temp *= 0.8;
                if ( temp > 0.002 )
                    temp    = 0.002;
                else if ( temp < -0.002 )
                    temp    = -0.002;
                
                Ctrl.Sen.C.Density          = Ctrl.Para.dat.SetDensity + temp;
            } else {
                Ctrl.Sen.C.Density          = Ctrl.Para.dat.SetDensity;
            }
            }
            break;
        case 1:                                         // 选择测量密度
            Ctrl.Sen.C.Density          = Ctrl.Para.dat.LastDensity;
            break;
        default:
            Ctrl.Sen.C.Density          = Ctrl.Para.dat.SetDensity;
            break;
        }
        
        /***********************************************
        * 描述： 重力加速选择
        */
        switch ( Ctrl.Para.dat.Sel.udat.GravitySel ) {
        case 0:                                         // 选择设置重力加速度
            Ctrl.Sen.C.Gravity          = Ctrl.Para.dat.SetGravity;
            break;
        case 1:                                         // 选择没理重力加速度
            Ctrl.Sen.C.Gravity          = Ctrl.Para.dat.LastGravity;
            break;
        default:
            Ctrl.Sen.C.Gravity          = Ctrl.Para.dat.SetGravity;
            break;
        }
        
        /***********************************************
        * 描述： 2）密度计算 0.xxxx克/立方厘米（10000倍）
        *        h1      = P1/(d*g);
        *        h2      = P2/(d*g);
        *        h1 - h2 = (P1 - P2)/(d*g);
        *        dh      = Ctrl.Sen.C.CalcDiffHight;
        *        P1      = Ctrl.Sen.L.Pressure;
        *        P2      = Ctrl.Sen.H.Pressure;
        */
        Ctrl.Sen.C.CalcDensity              = (float)(10.000 * (double)(Ctrl.Sen.C.DiffPressure)      / (double)((double)Ctrl.Sen.Para.dat.ChkedDiffHight * Ctrl.Sen.C.Gravity));
        Ctrl.Sen.C.CalcDensitySec           = (float)(10.000 * (double)(Ctrl.Sen.C.DiffPressureSec)   / (double)((double)Ctrl.Sen.Para.dat.ChkedDiffHight * Ctrl.Sen.C.Gravity));
        Ctrl.Sen.C.CalcDensityMin           = (float)(10.000 * (double)(Ctrl.Sen.C.DiffPressureMin)   / (double)((double)Ctrl.Sen.Para.dat.ChkedDiffHight * Ctrl.Sen.C.Gravity));
        Ctrl.Sen.C.CalcDensityAll           = (float)(10.000 * (double)(Ctrl.Sen.C.DiffPressureAll)   / (double)((double)Ctrl.Sen.Para.dat.ChkedDiffHight * Ctrl.Sen.C.Gravity));            
        Ctrl.Sen.C.CalcDensitySample        = (float)(10.000 * (double)(Ctrl.Sen.C.DiffPressureSample)/ (double)((double)Ctrl.Sen.Para.dat.ChkedDiffHight * Ctrl.Sen.C.Gravity));            
        
        /***********************************************
        * 描述： 3）高度计算 xxx.x毫米（10倍）
        *        h = P/ρg; 
        */
        pSen                            = &Ctrl.Sen.H;              // 指向高点传感器
        
        for ( u8 sen = 0; sen < 2; sen++, pSen++ ) {
            pSen->Hight                 = (INT16S)((double)pSen->Pressure       * 10.0 / ( (double)Ctrl.Sen.C.Density * Ctrl.Sen.C.Gravity ) + Ctrl.Sen.C.OffsetHight);
            pSen->HightSec              = (INT16S)((double)pSen->PressureSec    * 10.0 / ( (double)Ctrl.Sen.C.Density * Ctrl.Sen.C.Gravity ) + Ctrl.Sen.C.OffsetHight);
            pSen->HightMin              = (INT16S)((double)pSen->PressureMin    * 10.0 / ( (double)Ctrl.Sen.C.Density * Ctrl.Sen.C.Gravity ) + Ctrl.Sen.C.OffsetHight);
            pSen->HightAll              = (INT16S)((double)pSen->PressureAll    * 10.0 / ( (double)Ctrl.Sen.C.Density * Ctrl.Sen.C.Gravity ) + Ctrl.Sen.C.OffsetHight);
            pSen->HightSample           = (INT16S)((double)pSen->PressureSample * 10.0 / ( (double)Ctrl.Sen.C.Density * Ctrl.Sen.C.Gravity ) + Ctrl.Sen.C.OffsetHight);
        }
        
        Ctrl.Sen.C.CalcHight            = (INT16S)((double)Ctrl.Sen.L.Pressure        * (double)Ctrl.Sen.Para.dat.ChkedDiffHight / (double)( Ctrl.Sen.C.DiffPressure       ) + Ctrl.Sen.C.OffsetHight);
        Ctrl.Sen.C.CalcHightSec         = (INT16S)((double)Ctrl.Sen.L.PressureSec     * (double)Ctrl.Sen.Para.dat.ChkedDiffHight / (double)( Ctrl.Sen.C.DiffPressureSec    ) + Ctrl.Sen.C.OffsetHight);
        Ctrl.Sen.C.CalcHightMin         = (INT16S)((double)Ctrl.Sen.L.PressureMin     * (double)Ctrl.Sen.Para.dat.ChkedDiffHight / (double)( Ctrl.Sen.C.DiffPressureMin    ) + Ctrl.Sen.C.OffsetHight);
        Ctrl.Sen.C.CalcHightAll         = (INT16S)((double)Ctrl.Sen.L.PressureAll     * (double)Ctrl.Sen.Para.dat.ChkedDiffHight / (double)( Ctrl.Sen.C.DiffPressureAll    ) + Ctrl.Sen.C.OffsetHight);
        Ctrl.Sen.C.CalcHightSample      = (INT16S)((double)Ctrl.Sen.L.PressureSample  * (double)Ctrl.Sen.Para.dat.ChkedDiffHight / (double)( Ctrl.Sen.C.DiffPressureSample ) + Ctrl.Sen.C.OffsetHight);
       
        Ctrl.Sen.C.AverHight            = (INT16S)(((float)Ctrl.Sen.L.Hight       + (float)Ctrl.Sen.H.Hight       + Ctrl.Sen.Para.dat.ChkedDiffHight) / 2);
        Ctrl.Sen.C.AverHightSec         = (INT16S)(((float)Ctrl.Sen.L.HightSec    + (float)Ctrl.Sen.H.HightSec    + Ctrl.Sen.Para.dat.ChkedDiffHight) / 2);
        Ctrl.Sen.C.AverHightMin         = (INT16S)(((float)Ctrl.Sen.L.HightMin    + (float)Ctrl.Sen.H.HightMin    + Ctrl.Sen.Para.dat.ChkedDiffHight) / 2);
        Ctrl.Sen.C.AverHightAll         = (INT16S)(((float)Ctrl.Sen.L.HightAll    + (float)Ctrl.Sen.H.HightAll    + Ctrl.Sen.Para.dat.ChkedDiffHight) / 2);
        Ctrl.Sen.C.AverHightSample      = (INT16S)(((float)Ctrl.Sen.L.HightSample + (float)Ctrl.Sen.H.HightSample + Ctrl.Sen.Para.dat.ChkedDiffHight) / 2);
        
           
        INT16S  *ph;
        
        if ( Ctrl.Para.dat.Sel.udat.FilterSel > 4 )
            Ctrl.Para.dat.Sel.udat.FilterSel = 0;
        
        INT08U  SenSel  = Ctrl.Para.dat.Sel.udat.SensorSel;
        /***********************************************
        * 描述： 传感器选择:0低、1高、2平均、3智能
        */
        again:
        switch ( SenSel ) {            
        /***********************************************
        * 描述： 传感器选择：0低
        */
        case 0:
            ph                          = (INT16S *)(&Ctrl.Sen.L.HightSample - Ctrl.Para.dat.Sel.udat.FilterSel);
            Ctrl.Sen.C.Hight            = *ph;
            break;
        /***********************************************
        * 描述： 传感器选择：1高
        */
        case 1:
            ph                          = (INT16S *)(&Ctrl.Sen.H.HightSample - Ctrl.Para.dat.Sel.udat.FilterSel);
            Ctrl.Sen.C.Hight            = (INT16S)(*ph + Ctrl.Sen.Para.dat.ChkedDiffHight);
            break;
        /***********************************************
        * 描述： 传感器选择：2平均
        */
        case 2:
            ph                          = (INT16S *)(&Ctrl.Sen.C.AverHightSample - Ctrl.Para.dat.Sel.udat.FilterSel);
            Ctrl.Sen.C.Hight            = *ph;
            break;
        /***********************************************
        * 描述： 传感器选择：3智能
        */
        case 3:
            /***********************************************
            * 描述： 高位传感器有故障
            */
            if (Ctrl.Para.dat.SenSts.Udat.Err.HSenBrokenErr || 
                Ctrl.Para.dat.SenSts.Udat.Err.HSenExitDetection ||
                Ctrl.Para.dat.SenSts.Udat.Err.HSenShortErr ) {
                /***********************************************
                * 描述： 低位传感器有故障
                */
                if (Ctrl.Para.dat.SenSts.Udat.Err.LSenBrokenErr || 
                    Ctrl.Para.dat.SenSts.Udat.Err.LSenExitDetection ||
                    Ctrl.Para.dat.SenSts.Udat.Err.LSenShortErr ) {
                   //ph          = NULL;
                /***********************************************
                * 描述： 低位传感器无故障，选择低位传感器
                */
                } else {
                    SenSel      = 0;
                    goto again;
                }
            /***********************************************
            * 描述： 低位传感器有故障
            */
            } else if (Ctrl.Para.dat.SenSts.Udat.Err.LSenBrokenErr || 
                Ctrl.Para.dat.SenSts.Udat.Err.LSenExitDetection ||
                Ctrl.Para.dat.SenSts.Udat.Err.LSenShortErr ) {
                if (Ctrl.Para.dat.SenSts.Udat.Err.HSenBrokenErr || 
                    Ctrl.Para.dat.SenSts.Udat.Err.HSenExitDetection ||
                    Ctrl.Para.dat.SenSts.Udat.Err.HSenShortErr ) {
                   //ph          = NULL;
                /***********************************************
                * 描述： 高位传感器无故障，选择高位传感器
                */
                } else {
                    SenSel      = 1;
                    goto again;
                }
            /***********************************************
            * 描述： 两传感器均无故障
            */
            } else {
                /***********************************************
                * 描述： 判断数据是否正常
                */
                //if (  ) {
                
                /***********************************************
                * 描述： 数据正常，选择平均
                */
                //} else {
                if ( abs( (int)(Ctrl.Sen.C.CalcDiffHight - Ctrl.Sen.Para.dat.ChkedDiffHight)) > 100 ) {
                    SenSel  = 0;
                } else {
                    SenSel  = 2;
                }
                    goto again;
                //}
            }
            
            Ctrl.Sen.C.Hight            = 0;
            break;
        /***********************************************
        * 描述： 传感器选择:0或其他值选择低
        */
        default: 
            Ctrl.Para.dat.Sel.udat.SensorSel    = 0;
            break;
        }
        
        /***********************************************
        * 描述： 4）高度差计算 0~1000x0.1mm(两传感器高度差
        *        Δd = dL - dH
        */
        Ctrl.Sen.C.CalcDiffHight        = (INT16S)(100 * Ctrl.Sen.C.DiffPressure       / Ctrl.Sen.Para.dat.TestDensity   / Ctrl.Sen.Para.dat.TestGravity);//Ctrl.Sen.L.Hight      - Ctrl.Sen.H.Hight;
        Ctrl.Sen.C.CalcDiffHightSec     = (INT16S)(100 * Ctrl.Sen.C.DiffPressureSec    / Ctrl.Sen.Para.dat.TestDensity   / Ctrl.Sen.Para.dat.TestGravity);//Ctrl.Sen.L.HightSec   - Ctrl.Sen.H.HightSec;
        Ctrl.Sen.C.CalcDiffHightMin     = (INT16S)(100 * Ctrl.Sen.C.DiffPressureMin    / Ctrl.Sen.Para.dat.TestDensity   / Ctrl.Sen.Para.dat.TestGravity);//Ctrl.Sen.L.HightMin   - Ctrl.Sen.H.HightMin;
        Ctrl.Sen.C.CalcDiffHightAll     = (INT16S)(100 * Ctrl.Sen.C.DiffPressureAll    / Ctrl.Sen.Para.dat.TestDensity   / Ctrl.Sen.Para.dat.TestGravity);//Ctrl.Sen.L.HightAll   - Ctrl.Sen.H.HightAll;
        Ctrl.Sen.C.CalcDiffHightSample  = (INT16S)(100 * Ctrl.Sen.C.DiffPressureSample / Ctrl.Sen.Para.dat.TestDensity   / Ctrl.Sen.Para.dat.TestGravity);//Ctrl.Sen.L.HightSample- Ctrl.Sen.H.HightSample;
        
        /***********************************************
        * 描述： 5）体积计算 
        *        V = h*s
        */
        Ctrl.Sen.C.Volume   = App_HightToVolume(Ctrl.Sen.C.Hight) ;
        
        /***********************************************
        * 描述： 6）质量计算 
        *        m = ρV
        */
        Ctrl.Sen.C.Weight   = Ctrl.Sen.C.Density * Ctrl.Sen.C.Volume;
        if ( Ctrl.Sen.C.Weight < 0 )
            Ctrl.Sen.C.Weight   = 0;
        
        /***********************************************
        * 描述：  
        *        Kp = ρgh/ΔP
        */
        Ctrl.Sen.C.Kp       = Ctrl.Sen.Para.dat.TestDensity
                            * Ctrl.Sen.Para.dat.TestGravity
                            * Ctrl.Sen.Para.dat.ChkedDiffHight
                            / Ctrl.Sen.C.DiffPressureSample
                            / 10.0;
        
        /***********************************************
        * 描述： 速度不为零时，停止更新密度定时器
        */
        return ( task_event ^ OS_EVT_SEN_TICKS );
    }
    
    /***************************************************************************
    *                               密度测量
    ***************************************************************************/
    
    /***************************************************************************
    * 描述： 更新密度值
    */
    if( task_event & OS_EVT_SEN_UPDATE_DENSITY ) {
        /***********************************************
        * 描述： 密度获取状态机
        *        0 上电或重启
        *        1 停车
        *        2 停车->启动
        *        3 运行
        *        4 运行->停车
        *        5 停车
        */
        switch ( Ctrl.Sen.DensityStep ) {
        /***********************************************
        * 描述： 上电或重启
        */
        case 0:
            if ( Ctrl.Comm.Rd.Data.Speed == 0 ) {           // 上电后速度为零
                Ctrl.Sen.DensitySts        |= 1 << 0;       // 更新一次密度值
                Ctrl.Sen.DensityStep        = 1;            // 转到30分钟检测
            } else {                                        // 上电后速度不为零
                if ( Ctrl.Para.dat.LastDensity == OIL_DENSITY_DEF )
                    Ctrl.Sen.DensitySts    |= 1 << 0;       // 更新一次密度值
                else
                    Ctrl.Sen.DensitySts     = 0;            // 则不更新密度
                Ctrl.Sen.DensityStep        = 2;            // 转到停车5分钟检测
            }
            // 启动5秒重载定时器
            osal_start_timerRl( OS_TASK_ID_SEN, OS_EVT_SEN_UPDATE_DENSITY , CYCLE_UPDATE_TICKS);
            break;
        /***********************************************
        * 描述： 停车后30分钟检测
        */
        case 1:
            if ( Ctrl.Comm.Rd.Data.Speed == 0 ) {           // 停车后如果速度为零，即没有开车
                if ( ++Ctrl.Sen.DensityCtr > CYCLE_UPDATE_TICKS_CNT ) { // 则30分钟后
                    Ctrl.Sen.DensityCtr = 0;                // 清除30分钟计数器，重新计数
                    Ctrl.Sen.DensitySts|= 1 << 0;           // 更新一次密度值
                }
            } else {                                        // 如果停车后机车启动，即有速度
                Ctrl.Sen.DensityCtr     = 0;                // 清除5分钟计数器
                Ctrl.Sen.DensityStep    = 2;                // 转到5分钟停车检测
            }
            break;
        /***********************************************
        * 描述： 停车后5分钟检测
        */
        case 2:
            if ( Ctrl.Comm.Rd.Data.Speed == 0 ) {           // 如果停车
                if ( ++Ctrl.Sen.DensityCtr > STOP_UPDATE_TICKS_CNT ) {         // 计时5分钟到
                    Ctrl.Sen.DensityCtr     = 0;            // 清除计数器
                    Ctrl.Sen.DensitySts    |= 1 << 0;       // 更新一次密度值
                    Ctrl.Sen.DensityStep    = 1;            // 转到停车30分钟检测
                }
            } else {                                        // 如果机车运行中
                Ctrl.Sen.DensityCtr = 0;                    // 清除计数器
            }
            break;
        /***********************************************
        * 描述： 
        */
        default:
            break;
        }
        
        /***********************************************
        * 描述： 液位检测,根据不同的液位更新密度
        */
        static char     flag    = 0; 
        float  Pressure         = Ctrl.Sen.L.PressureSample;
#define DIFF_P  250
        if ( Pressure < 1000 - DIFF_P  ) {
            if ( flag != 1 ) {
                flag    = 1;
                Ctrl.Sen.DensitySts |= 1 << 1;
            }
        } else if ( ( Pressure  > 1000 + DIFF_P ) && ( Pressure < 2000 - DIFF_P ) ) {
            if ( flag != 2 ) {
                flag    = 2;
                Ctrl.Sen.DensitySts |= 1 << 1;
            }
        } else if ( ( Pressure  > 2000 + DIFF_P ) && ( Pressure < 3000 - DIFF_P ) ) {
            if ( flag != 3 ) {
                flag    = 3;
                Ctrl.Sen.DensitySts |= 1 << 1;
            }
        } else if ( ( Pressure  > 3000 + DIFF_P ) && ( Pressure < 4000 - DIFF_P ) ) {
            if ( flag != 4 ) {
                flag    = 4;
                Ctrl.Sen.DensitySts |= 1 << 1;
            }
        } else if ( ( Pressure  > 4000 + DIFF_P ) && ( Pressure < 5000 - DIFF_P ) ) {
            if ( flag != 5 ) {
                flag    = 5;
                Ctrl.Sen.DensitySts |= 1 << 1;
            }
        } else if ( ( Pressure  > 5000 + DIFF_P ) && ( Pressure < 6000 - DIFF_P ) ) {
            if ( flag != 6 ) {
                flag    = 6;
                Ctrl.Sen.DensitySts |= 1 << 1;
            }
        } else if ( ( Pressure  > 6000 + DIFF_P ) && ( Pressure < 7000 - DIFF_P ) ) {
            if ( flag != 7 ) {
                flag    = 7;
                Ctrl.Sen.DensitySts |= 1 << 1;
            }
        } else if ( ( Pressure  > 7000 + DIFF_P ) && ( Pressure < 8000 - DIFF_P ) ) {
            if ( flag != 8 ) {
                flag    = 8;
                Ctrl.Sen.DensitySts |= 1 << 1;
            }
        } else if ( ( Pressure  > 8000 + DIFF_P ) && ( Pressure < 9000 - DIFF_P ) ) {
            if ( flag != 9 ) {
                flag    = 9;
                Ctrl.Sen.DensitySts |= 1 << 1;
            }
        } else if ( ( Pressure  > 9000 + DIFF_P ) && ( Pressure < 10000 - DIFF_P ) ) {
            if ( flag != 10 ) {
                flag    = 10;
                Ctrl.Sen.DensitySts |= 1 << 1;
            }
        } else if ( Pressure  > 10050  ) {
            if ( flag != 11 ) {
                flag    = 11;
                Ctrl.Sen.DensitySts |= 1 << 1;
            }
        } 
        
        static float   LastPressure     = 0;
        static float   LastDensity      = 0.0;
        
        if ( LastDensity == 0.0 )
            LastDensity = Ctrl.Sen.C.CalcDensity;
        
        if ( LastPressure == 0 )  {
            LastPressure    = Ctrl.Sen.L.PressureSec;
            goto next;
        }
        /***********************************************
        * 描述： 更新一次密度值，并保存以备下次重启或上电用
        */
        if ( ( Ctrl.Sen.DensitySts ) &&                                         // 有密度更新事件
             ( fabs( LastPressure - Ctrl.Sen.L.PressureSec ) < 3 ) &&           // 压力的变化速率不能超过10Pas
             ( Ctrl.Sen.H.PressureMin > 500 ) &&                                // 高位传感器的压强 > 100Pa
             ( !Ctrl.Para.dat.SenSts.Udat.Test ) ) {                            // 不在测试模式
            /***********************************************
            * 描述： 计算当前密度
            */ 
            float   CalcDensity   = 0.0; 
            INT08U   nbr           = 0;
            
            if ( ( (Ctrl.Sen.C.CalcDensityMin > OIL_DENSITY_MIN) && (Ctrl.Sen.C.CalcDensityMin  < OIL_DENSITY_MAX) ) || 
                 ( (Ctrl.Sen.C.CalcDensityMin > WARTER_DENSITY_MIN) && (Ctrl.Sen.C.CalcDensityMin  < WARTER_DENSITY_MAX) ) ) {
                CalcDensity +=  Ctrl.Sen.C.CalcDensityMin;
                nbr++;
            }
            
            if ( ( (Ctrl.Sen.C.CalcDensityAll > OIL_DENSITY_MIN) && (Ctrl.Sen.C.CalcDensityAll  < OIL_DENSITY_MAX) ) || 
                 ( (Ctrl.Sen.C.CalcDensityAll > WARTER_DENSITY_MIN) && (Ctrl.Sen.C.CalcDensityAll  < WARTER_DENSITY_MAX) ) ) {
                CalcDensity +=  Ctrl.Sen.C.CalcDensityAll;
                nbr++;
            }
            
            CalcDensity /= nbr;
            
            if ( ( (CalcDensity > OIL_DENSITY_MIN) && (CalcDensity  < OIL_DENSITY_MAX) ) || 
                 ( (CalcDensity > WARTER_DENSITY_MIN) && (CalcDensity  < WARTER_DENSITY_MAX) ) ) {
                /***********************************************
                * 描述： 
                */ 
                nbr     = (INT08U)Ctrl.Sen.Para.dat.DiffPNbr;
                /***********************************************
                * 描述： 校准方式为1和2的时候加段补偿
                */ 
                if ( (( Ctrl.Sen.Para.dat.SenSts.udat.ChkType == 1 ) ||
                      ( Ctrl.Sen.Para.dat.SenSts.udat.ChkType == 2 ) ) && 
                     ( ( nbr > 1 ) && ( nbr <= 10 ) ) ) {
                       
                    INT08U    i       = (INT08U)(Ctrl.Sen.L.PressureMin/Ctrl.Sen.C.CalcDensity/(1000*(10/nbr)) );
                    i                 = ( i > 9 ) ? 9 : i;
                    float     k       = Ctrl.Sen.Para.dat.DiffP[i];
                      
                    /***********************************************
                    * 描述： 检测系数是否合法，不合法则不乘系数
                    */ 
                    if ( ( k > 0.9 ) && ( k < 1.1 ) )
                        CalcDensity       *= k; 
                }
                
                /***********************************************
                * 描述： 检测当前密度的变化范围与上次密度变化太大时，重新测量一次
                */ 
                if ( fabs(LastDensity - CalcDensity) < 0.003 ) {
                    /***********************************************
                    * 描述： 清除更新状态位
                    */
                    Ctrl.Sen.DensitySts = 0;
                    /***********************************************
                    * 描述： 
                    */ 
                    Ctrl.Para.dat.LastDensity  = CalcDensity;
                    /***********************************************
                    * 描述： 保存当前32位密度值
                    */ 
                    BSP_EEP_WriteBytes(2, (INT08U *)&Ctrl.Para.buf1[2],4);
                }
                
                LastDensity = CalcDensity;
            } else {
                /***********************************************
                * 描述： 
                */ 
                Ctrl.Sen.DensityCtr     = 0;                    // 清除5分钟计数器
                Ctrl.Sen.DensityStep    = 2;                    // 转到5分钟停车检测
            }
        }
    next:
        /***********************************************
        * 描述： 更新上次压力值
        */ 
        LastPressure    = Ctrl.Sen.L.PressureSec;
        
        return ( task_event ^ OS_EVT_SEN_UPDATE_DENSITY );
    }
    
    /***************************************************************************
    *                               故障检测
    ***************************************************************************/
    
    /***************************************************************************
    * 描述： 传感器故障检测：2秒钟滤波一次
    */
    if( task_event & OS_EVT_SEN_ERR_CHK ) {
        __StrSen    *pSen   = &Ctrl.Sen.H;              // 指向高点传感器
         
        if ( pSen->CurrentSec < 0.1 ) {
            Ctrl.Para.dat.SenSts.Udat.Err.HSenBrokenErr         = TRUE;
            Ctrl.Para.dat.SenSts.Udat.Err.HSenExitDetection     = FALSE;
        } else if ( pSen->Hight < 20 ) {
            Ctrl.Para.dat.SenSts.Udat.Err.HSenExitDetection     = TRUE;
        } else if ( pSen->CurrentSec >= 22.0 ) {
            Ctrl.Para.dat.SenSts.Udat.Err.HSenShortErr          = TRUE;
            Ctrl.Para.dat.SenSts.Udat.Err.HSenExitDetection     = FALSE;
        } else { 
            Ctrl.Para.dat.SenSts.Udat.Err.HSenBrokenErr         = FALSE;
            Ctrl.Para.dat.SenSts.Udat.Err.HSenExitDetection     = FALSE;
            Ctrl.Para.dat.SenSts.Udat.Err.HSenShortErr          = FALSE;
        }
        
        pSen   = &Ctrl.Sen.L;                           // 指向低点传感器
         
        if ( pSen->CurrentSec < 0.1 ) {
            Ctrl.Para.dat.SenSts.Udat.Err.LSenBrokenErr         = TRUE;
            Ctrl.Para.dat.SenSts.Udat.Err.LSenExitDetection     = FALSE;
        } else if ( pSen->Hight < 0.0 ) {
            Ctrl.Para.dat.SenSts.Udat.Err.LSenExitDetection     = TRUE;
        } else if ( pSen->CurrentSec >= 22.0 ) {
            Ctrl.Para.dat.SenSts.Udat.Err.LSenShortErr          = TRUE;
            Ctrl.Para.dat.SenSts.Udat.Err.LSenExitDetection     = FALSE;
        } else { 
            Ctrl.Para.dat.SenSts.Udat.Err.LSenBrokenErr         = FALSE;
            Ctrl.Para.dat.SenSts.Udat.Err.LSenExitDetection     = FALSE;
            Ctrl.Para.dat.SenSts.Udat.Err.LSenShortErr          = FALSE;
        }
        
        //osal_start_timerEx( OS_TASK_ID_SEN, OS_EVT_SEN_ERR_CHK ,        CYCLE_SEN_ERR_CHK);  

        return ( task_event ^ OS_EVT_SEN_ERR_CHK );
    }
    return 0;
}

/*******************************************************************************
 * 名    称： App_fParaFilter
 * 功    能： 浮点参数过滤
 * 入口参数： 无
 * 出口参数： 无
 * 作　　者： wumingshen.
 * 创建日期： 2016-04-20
 * 修    改：
 * 修改日期：
 *******************************************************************************/
float App_fParaFilter(float para, float def, float min, float max)
{
    if ( para < min ) 
        para   = def;
    else if ( para > max )
        para   = def;
    else if ( (INT16U)para == 0XFFFF ) {
        para   = def;
    }
    
    return para;
}

/*******************************************************************************
 * 名    称： App_lParaFilter
 * 功    能： 整型参数过滤
 * 入口参数： 无
 * 出口参数： 无
 * 作　　者： wumingshen.
 * 创建日期： 2016-04-20
 * 修    改：
 * 修改日期：
 *******************************************************************************/
long App_lParaFilter(long para, long def, long min, long max)
{
    if ( para < min ) 
        para   = def;
    else if ( para > max )
        para   = def; 
    
    return para;
}
/*******************************************************************************
 * 名    称： APP_SenInit
 * 功    能： 任务初始化
 * 入口参数： 无
 * 出口参数： 无
 * 作　　者： wumingshen.
 * 创建日期： 2015-03-28
 * 修    改：
 * 修改日期：
 *******************************************************************************/
void TaskInitSen(void)
{
    /***********************************************
    * 描述： 初始化ADC
    */
    BSP_ADCInit();
    /***********************************************
    * 描述： 上电需要初始化的参数
    */
    // 获取上次密度
    Ctrl.Para.dat.LastDensity       = (float)App_fParaFilter(Ctrl.Para.dat.LastDensity,
                                                             OIL_DENSITY_DEF,
                                                             OIL_DENSITY_MIN,
                                                             OIL_DENSITY_MAX);
    Ctrl.Para.dat.SetDensity        = (float)App_fParaFilter(Ctrl.Para.dat.SetDensity,
                                                             OIL_DENSITY_DEF,
                                                             OIL_DENSITY_MIN,
                                                             OIL_DENSITY_MAX);
    Ctrl.Para.dat.LastGravity       = (float)App_fParaFilter(Ctrl.Para.dat.LastGravity,
                                                             GRAVITY_DEF,
                                                             GRAVITY_MIN,
                                                             GRAVITY_MAX);
    Ctrl.Para.dat.SetGravity        = (float)App_fParaFilter(Ctrl.Para.dat.SetGravity,
                                                             GRAVITY_DEF,
                                                             GRAVITY_MIN,
                                                             GRAVITY_MAX); 
    
    // 获取偏移值
    Ctrl.Sen.C.OffsetHight          = (INT16S)App_fParaFilter(Ctrl.Para.dat.Offset,0,0,1000);
    if ( fabs(Ctrl.Sen.Para.dat.Offset) < 50.0 )
        Ctrl.Sen.C.OffsetHight     += (INT16S)Ctrl.Sen.Para.dat.Offset;
    
    // 获取传感器安装高度差
    Ctrl.Sen.Para.dat.ChkedDiffHight= (float)App_fParaFilter(Ctrl.Sen.Para.dat.ChkedDiffHight,
                                                             DIFF_HIGHT_DEF,
                                                             DIFF_HIGHT_MIN,
                                                             DIFF_HIGHT_MAX);
    
    //Ctrl.Sen.C.ChkedDiffHight       = (INT16S)Ctrl.Sen.Para.dat.ChkedDiffHight;
    //Ctrl.Sen.C.ChkedDensity         = (INT16S)Ctrl.Sen.Para.dat.ChkedDiffHight;
    
    // 测试标志位清零
    Ctrl.Para.dat.SenSts.Udat.Test      = 0;
    
    // 复位标志位清零
    Ctrl.Para.dat.SenSts.Udat.RstFlag   = 0; 
    
    // 显示方式恢复默认
    //Ctrl.Para.dat.Sel.udat.DispSel      = 0; 
    
    // 传感器校准方式选择
    if ( Ctrl.Sen.Para.dat.SenSts.udat.ChkType > 4 )
        Ctrl.Sen.Para.dat.SenSts.udat.ChkType   = 0;
    
    // 传感器选择方式：智能
    Ctrl.Para.dat.Sel.udat.SensorSel            = 3; 
    
    // 油箱容积表选择
    //Ctrl.VTab   = &OilTab.Tab[0];
    Ctrl.VTab   = &Ctrl.Para.dat.Vtab;
    
    /***********************************************
    * 描述： 电路板校准电初始化
    */
    double   k,b;
    INT08U   saveFlag;
    StrIToPSenChk   i2pDef      = {
        0.0,                                        // 压力1  Pa为单位
        10000.0,                                    // 压力2
        4.0000,                                     // 电流1  uA为单位
        20.0000,                                    // 电流2
        625.0,                                      // 斜率
        -2500,                                      // 偏置
    };
    StrAdcToIBoardChk a2iDef    = {
        4.0000,                                     // 电流1  uA为单位
        20.0000,                                    // 电流2
        11915.6,                                    // ADC1
        59578.2,                                    // ADC2
        3.356929752048776e-4,                       // 斜率
        1.678464876024388e-5,                       // 偏置
    };
    
    for ( char ch = 0; ch < 2; ch++ ) {
        saveFlag    = 0;
        
        /***********************************************
        * 描述： 检测校准类型（压强差还是压强电流补偿）
        */ 
        switch ( Ctrl.Sen.Para.dat.SenSts.udat.ChkType ) {
        /***********************************************
        * 描述：  不校准
        */
        case 0:
        /***********************************************
        * 描述：  压强差校准
        */
        case 1:
            Ctrl.Para.dat.SenChk[ch]    = i2pDef;
            break;
        /***********************************************
        * 描述：  压强差+传感器补偿校准
        */
        case 2:
            if ( Ctrl.Sen.Para.dat.PINbr == 2 ) {
                Ctrl.Para.dat.SenChk[ch].P1 = Ctrl.Sen.Para.dat.PI[ch][0][0];
                Ctrl.Para.dat.SenChk[ch].I1 = Ctrl.Sen.Para.dat.PI[ch][0][1];
                Ctrl.Para.dat.SenChk[ch].P2 = Ctrl.Sen.Para.dat.PI[ch][1][0];
                Ctrl.Para.dat.SenChk[ch].I2 = Ctrl.Sen.Para.dat.PI[ch][1][1];
            }
            break;
        /***********************************************
        * 描述：  传感器参数校准
        */
        case 3:
        case 4:
            if ( Ctrl.Sen.Para.dat.PINbr == 2 ) {
                Ctrl.Para.dat.SenChk[ch].P1 = Ctrl.Sen.Para.dat.PI[ch][0][0];
                Ctrl.Para.dat.SenChk[ch].I1 = Ctrl.Sen.Para.dat.PI[ch][0][1];
                Ctrl.Para.dat.SenChk[ch].P2 = Ctrl.Sen.Para.dat.PI[ch][1][0];
                Ctrl.Para.dat.SenChk[ch].I2 = Ctrl.Sen.Para.dat.PI[ch][1][1];
            }
            break;
        default:
            break;
        }
        
        /***********************************************
        * 描述： 计算测量模块的斜率；以最大和最小点为斜率计算点
        */        
        if ( ( Ctrl.Para.dat.SenChk[ch].I2 > Ctrl.Para.dat.SenChk[ch].I1 ) &&
             ( Ctrl.Para.dat.SenChk[ch].P2 > Ctrl.Para.dat.SenChk[ch].P1 ) ) {
            k   = (double)(Ctrl.Para.dat.SenChk[ch].P2 - Ctrl.Para.dat.SenChk[ch].P1)
                / (double)(Ctrl.Para.dat.SenChk[ch].I2 - Ctrl.Para.dat.SenChk[ch].I1);
            b   = (double)(Ctrl.Para.dat.SenChk[ch].P1 )
                - (double)Ctrl.Para.dat.SenChk[ch].I1 * k;
         } else {
            saveFlag    = 1;
            k   = ( 10000.0 - 0.0 ) / ( 20.0000 - 4.0000 ); // (y2-y1)/(x2-x1)
            b   = 0.0 - k * 4.0000;                         // y1-k*x1
         } 
        /***********************************************
        * 描述： 测量模块默认标准值
        */
        if ( saveFlag || (fabs( k - 625 ) > 20 ) ) {
            saveFlag    = 0;
            Ctrl.Para.dat.SenChk[ch]    = i2pDef;
            // 将默认参数写入EEPROM中
            INT08U *pb    = (INT08U   *)&Ctrl.Para.dat.SenChk[ch];
            App_SetParaToEep( 68 + ch * 24, pb, sizeof(StrAdcToIBoardChk) );
        } else {        
            Ctrl.Para.dat.SenChk[ch].K   = k;
            Ctrl.Para.dat.SenChk[ch].B   = b; 
        }
        
        /***********************************************
        * 描述： 计算显示模块斜率；以最大和最小点为斜率计算点
        */
        if ( ( Ctrl.Para.dat.Check[ch].I2 > Ctrl.Para.dat.Check[ch].I1 ) &&
             ( Ctrl.Para.dat.Check[ch].Adc2 > Ctrl.Para.dat.Check[ch].Adc1 ) ) {
            k   = (double)(Ctrl.Para.dat.Check[ch].I2 - Ctrl.Para.dat.Check[ch].I1)
                / (double)(Ctrl.Para.dat.Check[ch].Adc2 - Ctrl.Para.dat.Check[ch].Adc1);
            b   = (double)(Ctrl.Para.dat.Check[ch].I1 )
                - (double)Ctrl.Para.dat.Check[ch].Adc1 * k;
        } else {
            saveFlag    = 1;
            k   = ( 20.0000 - 4.0000 ) / ( 59578.2 - 11915.6 ); // (y2-y1)/(x2-x1)
            b   = 4.0000 - k * 11915.6;                         // y1-k*x1
        } 
        
        /***********************************************
        * 描述： 默认标准值
        */
        if ( saveFlag || ( fabs( k - a2iDef.K > 0.000005 ) ) ) {
            saveFlag    = 0;
            Ctrl.Para.dat.Check[ch]     = a2iDef;
            // 将默认参数写入EEPROM中
            INT08U *pb    = (INT08U   *)&Ctrl.Para.dat.Check[0];
            App_SetParaToEep( 20 + ch * 24, pb, sizeof(StrAdcToIBoardChk) );
        } else {
            Ctrl.Para.dat.Check[ch].K   = k;
            Ctrl.Para.dat.Check[ch].B   = b; 
        }
    }
       
    /***********************************************
    * 描述： 在看门狗标志组注册本任务的看门狗标志
    */
    WdtFlags |= WDT_FLAG_SEN;
    /*************************************************
    * 描述：启动事件查询
    */
#if ( OSAL_EN == DEF_ENABLED )
    osal_start_timerRl( OS_TASK_ID_SEN, OS_EVT_SEN_TICKS,           CYCLE_TIME_TICKS);
    osal_start_timerRl( OS_TASK_ID_SEN, OS_EVT_SEN_MSEC ,           CYCLE_SAMPLE_MSEC_TICKS);
    osal_start_timerRl( OS_TASK_ID_SEN, OS_EVT_SEN_DEAL ,           CYCLE_SAMPLE_TICKS);
    osal_start_timerEx( OS_TASK_ID_SEN, OS_EVT_SEN_UPDATE_DENSITY , CYCLE_UPDATE_DENSITY);  
    osal_start_timerRl( OS_TASK_ID_SEN, OS_EVT_SEN_ERR_CHK ,        CYCLE_SEN_ERR_CHK);  
#else
#endif
}

/*******************************************************************************
 * 				                    end of file                                *
 *******************************************************************************/
#endif